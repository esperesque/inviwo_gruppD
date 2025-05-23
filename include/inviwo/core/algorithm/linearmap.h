/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/glmvec.h>

namespace inviwo::util {

template <typename T>
constexpr T linearMap(T val, glm::vec<2, T> from, glm::vec<2, T> to) noexcept {
    static_assert(std::is_floating_point_v<T>);
    return (val - from.x) / (from.y - from.x) * (to.y - to.x) + to.x;
}

template <glm::length_t L, typename T>
constexpr glm::vec<L, T> linearMap(glm::vec<L, T> val, glm::vec<2, T> from,
                                   glm::vec<2, T> to) noexcept {
    static_assert(std::is_floating_point_v<T>);
    return (val - from.x) / (from.y - from.x) * (to.y - to.x) + to.x;
}

template <glm::length_t C, glm::length_t R, typename T>
constexpr glm::mat<C, R, T> linearMap(glm::mat<C, R, T> val, glm::vec<2, T> from,
                                      glm::vec<2, T> to) noexcept {
    static_assert(std::is_floating_point_v<T>);
    return (val - from.x) / (from.y - from.x) * (to.y - to.x) + to.x;
}

template <typename T>
constexpr T linearMapToNormalized(T val, glm::vec<2, T> from) noexcept {
    static_assert(std::is_floating_point_v<T>);
    return (val - from.x) / (from.y - from.x);
}

template <glm::length_t L, typename T>
constexpr glm::vec<L, T> linearMapToNormalized(glm::vec<L, T> val, glm::vec<2, T> from) noexcept {
    static_assert(std::is_floating_point_v<T>);
    return (val - from.x) / (from.y - from.x);
}

template <glm::length_t C, glm::length_t R, typename T>
constexpr glm::mat<C, R, T> linearMapToNormalized(glm::mat<C, R, T> val,
                                                  glm::vec<2, T> from) noexcept {
    static_assert(std::is_floating_point_v<T>);
    return (val - from.x) / (from.y - from.x);
}

template <typename T>
constexpr T linearMapFromNormalized(T val, glm::vec<2, T> to) noexcept {
    static_assert(std::is_floating_point_v<T>);
    return val * (to.y - to.x) + to.x;
}

template <glm::length_t L, typename T>
constexpr glm::vec<L, T> linearMapFromNormalized(glm::vec<L, T> val, glm::vec<2, T> to) noexcept {
    static_assert(std::is_floating_point_v<T>);
    return val * (to.y - to.x) + to.x;
}

template <glm::length_t C, glm::length_t R, typename T>
constexpr glm::mat<C, R, T> linearMapFromNormalized(glm::mat<C, R, T> val,
                                                    glm::vec<2, T> to) noexcept {
    static_assert(std::is_floating_point_v<T>);
    return val * (to.y - to.x) + to.x;
}

}  // namespace inviwo::util
