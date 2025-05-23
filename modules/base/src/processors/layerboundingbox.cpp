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

#include <modules/base/processors/layerboundingbox.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LayerBoundingBox::processorInfo_{
    "org.inviwo.LayerBoundingBox",  // Class identifier
    "Layer Bounding Box",           // Display name
    "Layer Operation",              // Category
    CodeState::Stable,              // Code state
    Tags::CPU,                      // Tags
    "Creates a mesh containing the bounding box of a layer, that is lines with "
    "adjacency information."_unindentHelp};

const ProcessorInfo& LayerBoundingBox::getProcessorInfo() const { return processorInfo_; }

LayerBoundingBox::LayerBoundingBox()
    : Processor{}
    , layer_{"layer", "Input layer"_help}
    , outport_{"mesh", "The bounding box mesh"_help}
    , color_("color", "Color", util::ordinalColor(vec4{1.0f}).set("Line color of the mesh"_help)) {

    addPorts(layer_, outport_);
    addProperties(color_);
}

void LayerBoundingBox::process() {
    using BBoxMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::TexCoordBuffer<2>,
                               buffertraits::ColorsBuffer>;

    auto mesh = std::make_shared<BBoxMesh>();
    mesh->setModelMatrix(layer_.getData()->getModelMatrix());
    mesh->setWorldMatrix(layer_.getData()->getWorldMatrix());

    mesh->addVertices({{vec3(0.0, 0.0, 0.0), vec2(0.0, 0.0), color_.get()},
                       {vec3(1.0, 0.0, 0.0), vec2(1.0, 0.0), color_.get()},
                       {vec3(1.0, 1.0, 0.0), vec2(1.0, 1.0), color_.get()},
                       {vec3(0.0, 1.0, 0.0), vec2(0.0, 1.0), color_.get()}});

    auto indices = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
    indices->add({3, 0, 1, 2, 3, 0, 1});

    outport_.setData(mesh);
}

}  // namespace inviwo
