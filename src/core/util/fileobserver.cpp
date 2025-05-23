/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystemobserver.h>

namespace inviwo {
FileObserver::FileObserver(InviwoApplication* app) : FileObserver{app->getFileSystemObserver()} {}

FileObserver::FileObserver(FileSystemObserver* fso) : fileSystemObserver_{fso} {
    if (fileSystemObserver_) {
        fileSystemObserver_->registerFileObserver(this);
    }
}

FileObserver::FileObserver(FileObserver&& other)
    : fileSystemObserver_{other.fileSystemObserver_}
    , observedFiles_(std::move(other.observedFiles_)) {

    if (fileSystemObserver_) {
        fileSystemObserver_->registerFileObserver(this);
    }
}
FileObserver& FileObserver::operator=(FileObserver&& that) {
    if (this != &that) {
        observedFiles_ = std::move(that.observedFiles_);
    }
    return *this;
}

FileObserver::~FileObserver() {
    if (fileSystemObserver_) {
        fileSystemObserver_->unRegisterFileObserver(this);

        for (auto file : observedFiles_) {
            fileSystemObserver_->stopFileObservation(file);
        }
    }
}

bool FileObserver::startFileObservation(const std::filesystem::path& fileName) {
    if (!fileSystemObserver_) return false;
    auto it = observedFiles_.find(fileName);
    if (it == observedFiles_.end()) {
        if (std::filesystem::is_regular_file(fileName) || std::filesystem::is_directory(fileName)) {
            observedFiles_.insert(fileName);
            fileSystemObserver_->startFileObservation(fileName);
            return true;
        }
    }
    return false;
}

bool FileObserver::stopFileObservation(const std::filesystem::path& fileName) {
    if (!fileSystemObserver_) return false;

    auto it = observedFiles_.find(fileName);
    if (it != observedFiles_.end()) {
        observedFiles_.erase(it);
        fileSystemObserver_->stopFileObservation(fileName);
        return true;
    }
    return false;
}

void FileObserver::stopAllObservation() {
    if (!fileSystemObserver_) return;

    for (auto file : observedFiles_) {
        fileSystemObserver_->stopFileObservation(file);
    }
    observedFiles_.clear();
}

bool FileObserver::isObserved(const std::filesystem::path& fileName) const {
    return observedFiles_.find(fileName) != observedFiles_.end();
}

const std::unordered_set<std::filesystem::path, PathHash>& FileObserver::getFiles() const {
    return observedFiles_;
}

}  // namespace inviwo
