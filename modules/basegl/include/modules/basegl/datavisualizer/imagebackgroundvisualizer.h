/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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
#pragma once

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/rendering/datavisualizer.h>  // for DataVisualizer
#include <inviwo/core/util/document.h>             // for Document

#include <string>   // for string
#include <utility>  // for pair
#include <vector>   // for vector

namespace inviwo {
class FileExtension;
class InviwoApplication;
class Outport;
class Processor;
class ProcessorNetwork;

class IVW_MODULE_BASEGL_API ImageBackgroundVisualizer : public DataVisualizer {
public:
    ImageBackgroundVisualizer(InviwoApplication* app);
    virtual ~ImageBackgroundVisualizer() = default;

    virtual std::string getName() const override;
    virtual Document getDescription() const override;
    virtual std::vector<FileExtension> getSupportedFileExtensions() const override;
    virtual bool isOutportSupported(const Outport* port) const override;

    virtual bool hasSourceProcessor() const override;
    virtual bool hasVisualizerNetwork() const override;

    virtual std::pair<Processor*, Outport*> addSourceProcessor(
        const std::filesystem::path& filename, ProcessorNetwork* network,
        const ivec2& origin) const override;
    virtual std::vector<Processor*> addVisualizerNetwork(Outport* outport,
                                                         ProcessorNetwork* network) const override;
    virtual std::vector<Processor*> addSourceAndVisualizerNetwork(
        const std::filesystem::path& filename, ProcessorNetwork* network,
        const ivec2& origin) const override;

private:
    InviwoApplication* app_;
};

}  // namespace inviwo
