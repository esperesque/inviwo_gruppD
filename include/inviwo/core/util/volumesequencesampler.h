/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/spatial4dsampler.h>
#include <inviwo/core/util/volumesampler.h>

namespace inviwo {

class IVW_CORE_API VolumeSequenceSampler : public Spatial4DSampler<dvec3> {
    struct Wrapper {
        std::weak_ptr<Wrapper> next_;
        double duration_;
        double timestamp_;
        std::shared_ptr<const Volume> volume_;
        VolumeSampler<dvec4> sampler_;

        Wrapper(std::shared_ptr<const Volume> volume)
            : next_()
            , duration_(std::numeric_limits<double>::infinity())
            , timestamp_(std::numeric_limits<double>::infinity())
            , volume_(volume)
            , sampler_(volume) {
            if (volume_->hasMetaData<DoubleMetaData>("timestamp")) {
                timestamp_ = volume_->getMetaData<DoubleMetaData>("timestamp")->get();
            }
            if (volume_->hasMetaData<DoubleMetaData>("duration")) {
                duration_ = volume_->getMetaData<DoubleMetaData>("duration")->get();
            }
        }

        bool operator<(const Wrapper& w) const { return timestamp_ < w.timestamp_; }
    };

public:
    VolumeSequenceSampler(std::shared_ptr<const VolumeSequence> volumeSequence,
                          bool allowLooping = true);
    virtual ~VolumeSequenceSampler();

    void setAllowedLooping(bool allowed = true) { allowLooping_ = allowed; }

protected:
    virtual dvec3 sampleDataSpace(const dvec4& pos) const;
    virtual bool withinBoundsDataSpace(const dvec4& pos) const;

private:
    std::vector<std::shared_ptr<Wrapper>> wrappers_;

    bool allowLooping_;
    dvec2 timeRange_;
    double totDuration_;
};

}  // namespace inviwo
