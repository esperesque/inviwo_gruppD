/*********************************************************************************
 * Inviwo - Interactive Visualization Workshop
 *********************************************************************************/
#pragma once

#include <modules/animationqt/animationqtmoduledefine.h>

#include <inviwo/core/common/inviwomodule.h>
#include <modules/animationqt/animationqtsupplier.h>
#include <modules/animationqt/factories/sequenceeditorfactory.h>
#include <modules/animationqt/factories/trackwidgetqtfactory.h>

#include <memory>

class QMenu;  // <- OK att ligga globalt, används utan namespace

/* ------------------------------------------------------------------ */
/*  ALLT UNDER LIGGER NU INUTI NAMESPACE INVIWO                        */
/* ------------------------------------------------------------------ */
namespace inviwo {

class InviwoApplication;
/* ►► Lägg forward-deklarationen HÄR istället för globalt ◄◄ */
class InviwoDockWidget;  // <- nu samma typ som i .cpp

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
    /* factories */
    animation::TrackWidgetQtFactory trackWidgetQtFactory_;
    animation::SequenceEditorFactory sequenceEditorFactory_;

    /* widgets vi behåller pekare till */
    std::unique_ptr<animation::AnimationEditorDockWidgetQt> editor_;
    std::unique_ptr<animation::DemoNavigatorDockWidgetQt> navigator_;
    std::unique_ptr<InviwoDockWidget> presentationView_;  // ← samma typ

    std::unique_ptr<QMenu> menu_;
};

}  // namespace inviwo
