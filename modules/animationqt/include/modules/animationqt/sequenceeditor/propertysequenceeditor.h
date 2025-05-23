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

#include <modules/animationqt/animationqtmoduledefine.h>  // for IVW_MODULE_ANIMATIO...

#include <modules/animation/datastructures/valuekeyframesequence.h>   // for ValueKeyframeSequen...
#include <modules/animationqt/sequenceeditor/sequenceeditorwidget.h>  // for SequenceEditorWidget

#include <string>  // for string

class QComboBox;
class QWidget;

namespace inviwo {

namespace animation {

class AnimationManager;
class Keyframe;
class KeyframeSequence;
class Track;

class IVW_MODULE_ANIMATIONQT_API PropertySequenceEditor : public SequenceEditorWidget,
                                                          public ValueKeyframeSequenceObserver {
public:
    PropertySequenceEditor(KeyframeSequence& sequence, Track& track, AnimationManager& manager);
    virtual ~PropertySequenceEditor() = default;

    static std::string classIdentifier();

protected:
    virtual QWidget* create(Keyframe* key) override;

    // ValueKeyframeSequenceObserver overloads
    virtual void onValueKeyframeSequenceEasingChanged(ValueKeyframeSequence* seq) override;
    virtual void onValueKeyframeSequenceInterpolationChanged(ValueKeyframeSequence* seq) override;

    QComboBox* easingComboBox_{nullptr};
    QComboBox* interpolation_{nullptr};
};

}  // namespace animation

}  // namespace inviwo
