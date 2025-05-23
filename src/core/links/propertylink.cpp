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

#include <inviwo/core/links/propertylink.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyowner.h>

#include <fmt/format.h>

namespace inviwo {

PropertyLink::PropertyLink() : src_(nullptr), dst_(nullptr) {}

PropertyLink::operator bool() const { return src_ && dst_; }

PropertyLink::PropertyLink(Property* src, Property* dst) : src_(src), dst_(dst) {}

bool PropertyLink::involves(Processor* processor) const {
    return src_->getOwner()->getProcessor() == processor ||
           dst_->getOwner()->getProcessor() == processor;
}
bool PropertyLink::involves(Property* property) const {
    return src_ == property || dst_ == property;
}

bool operator==(const PropertyLink& lhs, const PropertyLink& rhs) {
    return lhs.src_ == rhs.src_ && lhs.dst_ == rhs.dst_;
}
bool operator!=(const PropertyLink& lhs, const PropertyLink& rhs) { return !operator==(lhs, rhs); }

bool operator<(const PropertyLink& lhs, const PropertyLink& rhs) {
    if (lhs.src_ != rhs.src_) {
        return lhs.src_ < rhs.src_;
    } else {
        return lhs.dst_ < rhs.dst_;
    }
}

}  // namespace inviwo
