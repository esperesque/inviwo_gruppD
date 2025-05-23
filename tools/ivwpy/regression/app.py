# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2025 Inviwo Foundation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ********************************************************************************
import os
import itertools
import glob
import datetime
import json
import shutil
import filecmp
import logging
import difflib

from . import inviwoapp
from . import testdata
from .. util import *
from .. colorprint import *
from .. git import *
from . imagecompare import *
from . generatereport import *
from . database import *
from . reporttest import *
from . logprinter import *


def findModuleTest(path):
    # assume path points to a folder of modules.
    # look in folder path/<module>/tests/regression/*
    tests = []
    for moduleDir in subDirs(path):
        regressionDir = toPath(path, moduleDir, "tests", "regression")
        for testDir in subDirs(regressionDir):
            tests.append(testdata.TestData(
                name=testDir,
                module=moduleDir,
                path=toPath(regressionDir, testDir)
            ))
    return tests


class App:
    class Summary:
        def __init__(self):
            self.successes = 0
            self.failures = 0
            self.skipped = 0
            self.errors = []

    def __init__(self,
                 appPath,
                 moduleTestPaths,
                 outputDir,
                 jsonFile="report",
                 htmlFile="report",
                 sqlFile="report",
                 runSettings=inviwoapp.RunSettings(),
                 testSettings=ReportTestSettings()):

        self.app = inviwoapp.InviwoApp(appPath, runSettings)
        self.output = outputDir
        self.jsonFile = jsonFile
        self.htmlFile = htmlFile
        self.sqlFile = sqlFile
        self.testSettings = testSettings
        self.summary = App.Summary()

        tests = [findModuleTest(p) for p in moduleTestPaths]

        self.tests = list(itertools.chain(*tests))
        self.tests.sort(key=str)
        self.reports = {}
        self.git = Git(pyconfsearchpath=appPath)
        if not self.git.foundGit():
            print_error("Git not found")
            exit(1)

        self.db = Database(self.output + "/" + self.sqlFile + ".sqlite")
        self.loadJson()

    def runTest(self, test, run):
        report = {}
        report['date'] = datetime.datetime.now().isoformat()
        report = test.report(report)

        report = self.app.runTest(test, report, self.output)
        report = self.compareImages(test, report)
        report = self.compareTextFiles(test, report)
        testsuite = ReportTestSuite(self.testSettings)
        report = testsuite.checkReport(report)

        report['status'] = "new"
        report['git'] = self.git.info(report['path'])

        self.updateDatabase(report, run)
        return report

    def filterTests(self, testrange, testfilter, onlyRunFailed=False):
        selected1 = range(len(self.tests))
        selected2 = selected1[testrange]
        selected3 = list(filter(lambda i: testfilter(self.tests[i]), selected2))

        # don't run test from modules that we have not built
        selected4 = list(filter(lambda i: self.app.isModuleActive(self.tests[i].module), selected3))

        # Only Run Failed test
        if onlyRunFailed:
            def testfailed(test):
                last = self.db.getLastTestRun(test.module, test.name)
                if last and len(last.failures) > 0:
                    return True
                return False

            selected5 = list(filter(lambda i: testfailed(self.tests[i]), selected4))
        else:
            selected5 = selected4

        reasons = list(map(lambda x: "", range(len(self.tests))))
        for i in set(selected1).difference(set(selected2)):
            reasons[i] = "Test not in testrange"
        for i in set(selected2).difference(set(selected3)):
            reasons[i] = "Filtered out"
        for i in set(selected3).difference(set(selected4)):
            reasons[i] = "Needed module not available"
        for i in set(selected4).difference(set(selected5)):
            reasons[i] = "Only running failed tests"

        return selected5, reasons

    def runTests(self, testrange=slice(0, None), testfilter=lambda x: True, onlyRunFailed=False):
        selected, reasons = self.filterTests(testrange, testfilter, onlyRunFailed)
        run = self.db.addRun()
        for i, test in enumerate(self.tests):
            with LogPrinter(logging.getLogger()) as log:
                if i in selected:
                    log.pair("Running test {:3d} (Enabled: {:d}, Total: {:d})"
                             .format(i, len(selected), len(self.tests)), test.toString())

                    report = self.runTest(test, run)
                    self.reports[test.toString()] = report
                    for k, v in report.items():
                        log.pair(k, v, width=15)

                    if len(report["failures"]) > 0:
                        self.summary.failures += 1
                        failures = ', '.join(formatKey(f) for f, _ in report["failures"])
                        self.summary.errors.append(test.toString() + " Failures: " + failures)
                        log.success = False
                        log.error(f"{'Result':<15} : Failed {failures}")
                    else:
                        self.summary.successes += 1
                        log.good(f"{'Result':<15} : Success")
                else:
                    self.summary.skipped += 1
                    log.pair("Skipping test {:3d} (Enabled: {:d}, Total: {:d})"
                             .format(i, len(selected), len(self.tests)),
                             test.toString())
                    dbtest = self.db.getOrAddTest(test.module, test.name)
                    self.db.addSkipRun(run=run, test=dbtest, reason=reasons[i])

    def compareTextFiles(self, test, report):
        outputdir = report['outputdir']
        refFiles = set(test.getTextFiles())
        txtFiles = set(os.path.relpath(file, toPath(outputdir, 'imgtest'))
                       for ext in test.getTextExtensions()
                       for file in glob.glob(f"{outputdir}/imgtest/*.{ext}"))

        txtTests = {
            "refs": list(refFiles),
            "txts": list(txtFiles),
            'missing_refs': list(txtFiles - refFiles),
            'missing_txts': list(refFiles - txtFiles),
            'tests': []
        }

        for file in txtFiles & refFiles:
            with open(toPath(outputdir, "imgtest", file), 'r') as txtFile, \
                    open(toPath(test.path, file), 'r') as refFile:
                txtLines = txtFile.readlines()
                refLines = refFile.readlines()

            size = len(list(filter(lambda x: x[:2] == "@@",
                                   list(difflib.unified_diff(refLines, txtLines, n=0)))))
            txtTest = {
                "txt": file,
                "diff": size
            }
            txtTests["tests"].append(txtTest)

        report["txts"] = txtTests
        return report

    def compareImages(self, test, report):
        refimgs = test.getImages()
        refs = set(refimgs)

        outputdir = report['outputdir']
        imgs = glob.glob(outputdir + "/imgtest/*.png")
        imgs = set([os.path.relpath(x, toPath(outputdir, 'imgtest')) for x in imgs])

        imagetests = {
            "refs": list(refimgs),
            "imgs": list(imgs),
            'missing_refs': list(imgs - refs),
            'missing_imgs': list(refs - imgs),
            'tests': []
        }

        olddirs = list(reversed(sorted(glob.glob(outputdir + "/../*"))))
        if len(olddirs) > 1:
            lastoutdir = olddirs[1]
        else:
            lastoutdir = None

        testpath = mkdir(toPath(outputdir, "imgtest"))
        refpath = mkdir(toPath(outputdir, "imgref"))
        diffpath = mkdir(toPath(outputdir, "imgdiff"))
        maskpath = mkdir(toPath(outputdir, "imgmask"))

        invertDifferenceImage = (self.testSettings.imageDiffInvert or
            safeget(test.config, "image_test", "invertDifferenceImage", failure=False))
        logscaleDifferenceImage = (self.testSettings.imageDiffLogscale or
            safeget(test.config, "image_test", "logscaleDifferenceImage", failure=False))

        for img in imgs & refs:
            with ImageCompare(testImage=toPath(testpath, img),
                              refImage=toPath(test.path, img),
                              logscaleDifferenceImage=logscaleDifferenceImage,
                              invertDifferenceImage=invertDifferenceImage) as comp:

                comp.saveReferenceImage(toPath(refpath, img))
                comp.saveDifferenceImage(toPath(diffpath, img))
                comp.saveMaskImage(toPath(maskpath, img))

                imgtest = {
                    'image': img,
                    'difference_percent': comp.getDifferencePercent(),
                    'difference_pixels': comp.getDifferencePixelCount(),
                    'max_differences': comp.getMaxDifferences(),
                    'test_size': comp.getTestSize(),
                    'ref_size': comp.getRefSize(),
                    'test_mode': comp.getTestMode(),
                    'ref_mode': comp.getRefMode()
                }
                imagetests['tests'].append(imgtest)

            # do some hardlinks to save disk space...
            if lastoutdir is not None:
                self.linkImages(toPath(lastoutdir, "imgtest", img), toPath(testpath, img))
                self.linkImages(toPath(lastoutdir, "imgref", img), toPath(refpath, img))
                self.linkImages(toPath(lastoutdir, "imgdiff", img), toPath(diffpath, img))
                self.linkImages(toPath(lastoutdir, "imgmask", img), toPath(maskpath, img))

        report["images"] = imagetests
        return report

    def linkImages(self, oldimg, newimg):
        if os.path.exists(oldimg) and os.path.exists(newimg) and filecmp.cmp(oldimg, newimg):
            os.remove(newimg)
            os.link(oldimg, newimg)

    def printTestList(self, testrange=slice(0, None), testfilter=lambda x: True, printfun=print):
        printfun("List of regression tests")
        printfun("-" * 80)

        selected, reasons = self.filterTests(testrange, testfilter)
        for i, test in enumerate(self.tests):
            active = "Enabled" if i in selected and testfilter(
                test) else "Disabled (Reason: %s)" % reasons[i]
            printfun("{:3d} {:8s} {}".format(i, active, test))

    def saveJson(self):
        with open(self.output + "/" + self.jsonFile + ".json", 'w') as f:
            json.dump(self.reports, f, indent=4, separators=(',', ': '))

    def loadJson(self):
        if os.path.exists(self.output + "/" + self.jsonFile + ".json"):
            with open(self.output + "/" + self.jsonFile + ".json", 'r') as f:
                self.reports = json.load(f)
            for name, report in self.reports.items():
                report['status'] = "old"

    def saveHtml(self, header=None, footer=None):
        html = HtmlReport(basedir=self.output,
                          reports=self.reports,
                          database=self.db,
                          header=header,
                          footer=footer)
        filepath = html.saveHtml(self.htmlFile)
        date = self.db.getLastRunDate().strftime('%Y-%m-%dT%H_%M_%S')
        shutil.copyfile(filepath, toPath(self.output,
                                         self.htmlFile + "-" + date + ".html"))

    def success(self):
        for name, report in self.reports.items():
            if len(report['failures']) != 0:
                if safeget(report, "config", "enabled", failure=True):
                    return False
        return True

    def printSummary(self, out=sys.stdout):
        if(self.success()):
            print_good("Success: {0.successes}, Failures: {0.failures}, Skipped: {0.skipped}".format(
                self.summary), file=out)
        else:
            print_error(f"Success: {self.summary.successes}, Failures: {self.summary.failures},"
                        f" Skipped: {self.summary.skipped}", file=out)
        for i in self.summary.errors:
            print_error(i, file=out)

    def updateDatabase(self, report, run):
        dbtest = self.db.getOrAddTest(report["module"], report["name"])
        dbtime = self.db.getOrAddQuantity("time", "s")
        dbcount = self.db.getOrAddQuantity("count", "")
        dbfrac = self.db.getOrAddQuantity("fraction", "%")

        db_elapsed_time = self.db.getOrAddSeries(dbtest, dbtime, "elapsed_time")
        db_failures = self.db.getOrAddSeries(dbtest, dbcount, "number_of_failures")

        git = report["git"]
        dbcommit = self.db.getOrAddCommit(hash=git["hash"],
                                          date=stringToDate(git['date']),
                                          author=git['author'],
                                          message=git['message'],
                                          server=git['server'])

        dbtestrun = self.db.addTestRun(run=run,
                                       test=dbtest, commit=dbcommit,
                                       config=json.dumps(safeget(report, "config", failure="")))

        self.db.addMeasurement(series=db_elapsed_time,
                               testrun=dbtestrun,
                               value=report["elapsed_time"])
        self.db.addMeasurement(series=db_failures,
                               testrun=dbtestrun,
                               value=len(report["failures"]))

        for key, messages in report['failures']:
            if isinstance(key, list):
                key = "/".join(key)
            for message in messages:
                self.db.addFailure(testrun=dbtestrun, key=key, message=message)

        for img in report["images"]["tests"]:
            db_img_test = self.db.getOrAddSeries(dbtest, dbfrac, "image_test_diff." + img["image"])
            self.db.addMeasurement(series=db_img_test,
                                   testrun=dbtestrun,
                                   value=img["difference_percent"])

        if os.path.exists(report['outputdir'] + "/stats.json"):
            stats = []
            with open(report['outputdir'] + "/stats.json", 'r') as f:
                stats = json.load(f)

            for stat in stats:
                unit = self.db.getOrAddQuantity(stat["quantity"], stat["unit"])
                series = self.db.getOrAddSeries(dbtest, unit, stat["name"])
                self.db.addMeasurement(series=series, testrun=dbtestrun, value=stat["value"])
