/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

LayerRAM::LayerRAM(LayerType type) : LayerRepresentation(type) {}

bool LayerRAM::copyRepresentationsTo(LayerRepresentation* targetLayerRam) const {
    // We use a LayerRamResizer to copy/resize one representation into another.
    // The CImg module implements a LayerRamResizer and registers it with the app

    if (auto resizer = InviwoApplication::getPtr()->getLayerRamResizer()) {
        return resizer->resize(*this, *static_cast<LayerRAM*>(targetLayerRam));
    }
    return false;
}

std::type_index LayerRAM::getTypeIndex() const { return std::type_index(typeid(LayerRAM)); }

std::shared_ptr<LayerRAM> createLayerRAM(const size2_t& dimensions, LayerType type,
                                         const DataFormatBase* format,
                                         const SwizzleMask& swizzleMask,
                                         InterpolationType interpolation,
                                         const Wrapping2D& wrapping) {
    return dispatching::singleDispatch<std::shared_ptr<LayerRAM>, dispatching::filter::All>(
        format->getId(), [&]<typename T>() {
            return std::make_shared<LayerRAMPrecision<T>>(dimensions, type, swizzleMask,
                                                          interpolation, wrapping);
        });
}

std::shared_ptr<LayerRAM> createLayerRAM(const LayerReprConfig& config) {
    auto format = config.format ? config.format : LayerConfig::defaultFormat;
    return dispatching::singleDispatch<std::shared_ptr<LayerRAM>, dispatching::filter::All>(
        format->getId(),
        [&]<typename T>() { return std::make_shared<LayerRAMPrecision<T>>(config); });
}

}  // namespace inviwo
