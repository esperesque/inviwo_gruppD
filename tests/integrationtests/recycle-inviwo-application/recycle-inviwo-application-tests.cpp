/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/common/coremodulesharedlibrary.h>

namespace inviwo {

void forceBufferRepresentationCreation() {
    LogCentral logCentral;
    LogCentral::init(&logCentral);
    InviwoApplication inviwoApp("Inviwo-InitDeinit-Tests");
    // Initialize all modules
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>> modules;
    modules.emplace_back(createInviwoCore());
    inviwoApp.registerModules(std::move(modules));
    inviwoApp.resizePool(0);
    Buffer<float> buffer(1);
    // Make RepresentationFactoryManager request BufferRepresentation
    buffer.getRepresentation<BufferRAM>();
}

TEST(InitDeinit, representationFactoryManager) {
    // Create InviwoApplication, request representation, and then destroy InviwoApplication
    forceBufferRepresentationCreation();
    // It should be possible to initialize Inviwo a second time and request representations
    forceBufferRepresentationCreation();
}

}  // namespace inviwo
