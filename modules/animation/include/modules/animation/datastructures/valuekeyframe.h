/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <inviwo/core/io/serialization/serializer.h>    // IWYU pragma: keep
#include <inviwo/core/io/serialization/deserializer.h>  // IWYU pragma: keep

#include <modules/animation/datastructures/animationtime.h>  // for Seconds
#include <modules/animation/datastructures/basekeyframe.h>   // for BaseKeyframe

namespace inviwo {

namespace animation {

/** \class ValueKeyframe
 * Keyframe of a given value type (float, integer, vec3 and so on).
 * Stores the KeyFrame value at a given time.
 * @see Keyframe
 */
template <typename T>
class ValueKeyframe : public BaseKeyframe {
public:
    using value_type = T;
    ValueKeyframe() = default;
    ValueKeyframe(Seconds time);
    ValueKeyframe(Seconds time, const T& value);
    virtual ~ValueKeyframe() = default;
    virtual ValueKeyframe* clone() const override;

    ValueKeyframe(const ValueKeyframe& rhs) = default;
    ValueKeyframe& operator=(const ValueKeyframe& that);

    const T& getValue() const;
    T& getValue();

    void setValue(const T& value);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    T value_{};
};

template <typename T>
bool operator==(const ValueKeyframe<T>& a, const ValueKeyframe<T>& b) {
    return a.getTime() == b.getTime() && a.getValue() == b.getValue();
}
template <typename T>
bool operator!=(const ValueKeyframe<T>& a, const ValueKeyframe<T>& b) {
    return !(a == b);
}

template <typename T>
ValueKeyframe<T>::ValueKeyframe(Seconds time) : BaseKeyframe{time}, value_{} {}

template <typename T>
ValueKeyframe<T>::ValueKeyframe(Seconds time, const T& value) : BaseKeyframe{time}, value_(value) {}

template <typename T>
ValueKeyframe<T>& ValueKeyframe<T>::operator=(const ValueKeyframe<T>& that) {
    if (this != &that) {
        BaseKeyframe::operator=(that);
        value_ = that.value_;
    }
    return *this;
}

template <typename T>
ValueKeyframe<T>* ValueKeyframe<T>::clone() const {
    return new ValueKeyframe<T>(*this);
}

template <typename T>
const T& ValueKeyframe<T>::getValue() const {
    return value_;
}
template <typename T>
T& ValueKeyframe<T>::getValue() {
    return value_;
}

template <typename T>
void ValueKeyframe<T>::setValue(const T& value) {
    value_ = value;
}

template <typename T>
void ValueKeyframe<T>::serialize(Serializer& s) const {
    BaseKeyframe::serialize(s);
    s.serialize("value", value_);
}

template <typename T>
void ValueKeyframe<T>::deserialize(Deserializer& d) {
    BaseKeyframe::deserialize(d);
    d.deserialize("value", value_);
}
}  // namespace animation

}  // namespace inviwo
