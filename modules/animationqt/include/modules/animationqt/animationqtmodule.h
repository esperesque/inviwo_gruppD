/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/animationqt/animationqtmoduledefine.h>  // for IVW_MODULE_ANIMATIONQT_API

#include <inviwo/core/common/inviwomodule.h>                      // for InviwoModule
#include <modules/animationqt/animationqtsupplier.h>              // for AnimationQtSupplier
#include <modules/animationqt/factories/sequenceeditorfactory.h>  // for SequenceEditorFactory
#include <modules/animationqt/factories/trackwidgetqtfactory.h>   // for TrackWidgetQtFactory

#include <memory>  // for unique_ptr

class QMenu;

namespace inviwo {
class InviwoApplication;

namespace animation {
class AnimationEditorDockWidgetQt;
class DemoNavigatorDockWidgetQt;
}  // namespace animation

class IVW_MODULE_ANIMATIONQT_API AnimationQtModule : public InviwoModule,
                                                     public animation::AnimationQtSupplier {
public:
    AnimationQtModule(InviwoApplication* app);
    virtual ~AnimationQtModule();

    animation::TrackWidgetQtFactory& getTrackWidgetQtFactory();
    const animation::TrackWidgetQtFactory& getTrackWidgetQtFactory() const;

    animation::SequenceEditorFactory& getSequenceEditorFactory();
    const animation::SequenceEditorFactory& getSequenceEditorFactory() const;

private:
    animation::TrackWidgetQtFactory trackWidgetQtFactory_;
    animation::SequenceEditorFactory sequenceEditorFactory_;

    // Keep references to added widgets so that they can be removed in destructor
    std::unique_ptr<animation::AnimationEditorDockWidgetQt> editor_;
    std::unique_ptr<animation::DemoNavigatorDockWidgetQt> navigator_;

    std::unique_ptr<QMenu> menu_;  // Show/hide animation editor
};

}  // namespace inviwo
