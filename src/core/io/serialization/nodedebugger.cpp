/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/io/serialization/nodedebugger.h>
#include <inviwo/core/util/stringconversion.h>
#include <ticpp/tinyxml.h>

#include <sstream>

namespace inviwo {

NodeDebugger::NodeDebugger(TiXmlElement* elem) {
    while (elem) {
        const auto id = elem->Attribute("identifier");
        const auto type = elem->Attribute("type");
        nodes_.emplace_back(elem->Value(), id.value_or(""), type.value_or(""));
        elem = elem->Parent()->ToElement();
    }
}

const NodeDebugger::Node& NodeDebugger::operator[](std::size_t idx) const { return nodes_[idx]; }

std::string NodeDebugger::toString(std::size_t idx) const {
    if (idx < nodes_.size()) {
        return nodes_[idx].key + ": \"" + nodes_[idx].identifier + "\" of class \"" +
               nodes_[idx].type;
    } else {
        return "";
    }
}

std::vector<std::string> NodeDebugger::getPath() const {
    std::vector<std::string> path;
    for (std::vector<Node>::const_reverse_iterator it = nodes_.rbegin(); it != nodes_.rend();
         ++it) {
        if (!it->identifier.empty()) path.push_back(it->identifier);
    }
    return path;
}

std::string NodeDebugger::getDescription() const {
    std::vector<std::string> parts;
    for (const auto& elem : nodes_) {
        if (!elem.identifier.empty()) {
            std::stringstream ss;
            ss << elem.key << " '" << elem.identifier << "' of class '" << elem.type << "'";
            parts.push_back(ss.str());
        }
    }
    return joinString(parts, " in\n   ");
}

size_t NodeDebugger::size() const { return nodes_.size(); }

NodeDebugger::Node::Node(std::string_view k, std::string_view i, std::string_view t)
    : key(k), identifier(i), type(t) {}

}  // namespace inviwo
