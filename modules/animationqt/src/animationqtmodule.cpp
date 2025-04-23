/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
 * All rights reserved.
 *
 *********************************************************************************/

#include <modules/animationqt/animationqtmodule.h>

#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/moduleutils.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/staticstring.h>

#include <modules/animation/animationmodule.h>
#include <modules/animation/datastructures/buttontrack.h>
#include <modules/animation/datastructures/callbacktrack.h>
#include <modules/animation/datastructures/invalidationtrack.h>
#include <modules/animation/datastructures/camerakeyframe.h>
#include <modules/animation/datastructures/cameratrack.h>
#include <modules/animation/datastructures/controltrack.h>
#include <modules/animation/datastructures/propertytrack.h>
#include <modules/animation/datastructures/valuekeyframe.h>
#include <modules/animation/datastructures/valuekeyframesequence.h>
#include <modules/animation/interpolation/interpolation.h>

#include <modules/animationqt/animationeditordockwidgetqt.h>
#include <modules/animationqt/animationqtsupplier.h>
#include <modules/animationqt/demo/demonavigatordockwidgetqt.h>
#include <modules/animationqt/factories/sequenceeditorfactoryobject.h>
#include <modules/animationqt/sequenceeditor/controlsequenceeditor.h>
#include <modules/animationqt/sequenceeditor/invalidationsequenceeditor.h>
#include <modules/animationqt/sequenceeditor/propertysequenceeditor.h>
#include <modules/animationqt/widgets/controltrackwidgetqt.h>
#include <modules/animationqt/widgets/propertytrackwidgetqt.h>

#include <modules/animationqt/presentationviewpanel.h>  // Presentation-fönstret
#include <modules/qtwidgets/inviwodockwidget.h>         // InviwoDockWidget
#include <modules/qtwidgets/inviwoqtutils.h>

#include <algorithm>
#include <tuple>
#include <vector>

#include <QAction>
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QString>
#include <Qt>

#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace inviwo {
namespace animation {
class SequenceEditorFactory;
class TrackWidgetQtFactory;
}  // namespace animation
}  // namespace inviwo

#ifndef INVIWO_ALL_DYN_LINK
struct InitQtAnimationResources {
    InitQtAnimationResources() { Q_INIT_RESOURCE(animation); }
    ~InitQtAnimationResources() { Q_CLEANUP_RESOURCE(animation); }
} initQtAnimationResources;
#endif

namespace inviwo {

/* ------------------------------------------------------------------------- */
/*                               Hjälpfunktioner                              */
/* ------------------------------------------------------------------------- */

namespace {

template <typename PropertyType, typename Keyframe,
          typename KeyframeSequence = animation::KeyframeSequenceTyped<Keyframe>>
void registerPropertyTrackHelper(animation::AnimationQtSupplier& as) {
    using namespace animation;
    using TrackType = PropertyTrack<PropertyType, Keyframe, KeyframeSequence>;

    as.registerTrackToWidgetMap(TrackType::classIdentifier(),
                                PropertyTrackWidgetQt::classIdentifier());
    as.registerTrackToSequenceEditorMap(TrackType::classIdentifier(),
                                        PropertySequenceEditor::classIdentifier());
}

template <template <typename> class Prop>
struct Reghelper {
    template <typename T>
    auto operator()(animation::AnimationQtSupplier& as) {
        using namespace animation;
        registerPropertyTrackHelper<Prop<T>, ValueKeyframe<typename Prop<T>::value_type>>(as);
    }
};

}  // namespace

/* ------------------------------------------------------------------------- */
/*                               Konstruktor                                 */
/* ------------------------------------------------------------------------- */

AnimationQtModule::AnimationQtModule(InviwoApplication* app)
    : InviwoModule(app, "AnimationQt")
    , animation::AnimationQtSupplier(*this)
    , trackWidgetQtFactory_{}
    , sequenceEditorFactory_{} {

    using namespace animation;

    /* -------------------- skapa/utöka View-menyn -------------------------- */
    if (auto win = utilqt::getApplicationMainWindow()) {
        QString animationMenuName("Animation");
        QMenu* menu = nullptr;

        auto menus = win->menuBar()->findChildren<QMenu*>();
        auto viewMenuIt = std::find_if(menus.begin(), menus.end(), [](auto& m) {
            return m->title().compare(QObject::tr("&View"), Qt::CaseInsensitive) == 0;
        });

        if (viewMenuIt != menus.end()) {
            menu = (*viewMenuIt);
        } else {
            menu_ = std::make_unique<QMenu>(animationMenuName);
            win->menuBar()->addMenu(menu_.get());
            menu = menu_.get();
            QObject::connect(menu_.get(), &QObject::destroyed, [&](QObject*) { menu_.release(); });
        }

        /* ------ hjälpare som skapar widgeten första gången man klickar ----- */
        const auto addWidgetLazy = [&](QString name, Qt::DockWidgetArea area, auto factory) {
            auto* action = menu->addAction(name);
            action->setCheckable(true);
            QObject::connect(action, &QAction::triggered, [action, win, area, menu, factory]() {
                util::OnScopeExit cleanup([action]() { delete action; });

                auto widget = factory();  // skapas nu
                win->addDockWidget(area, widget);
                widget->hide();
                widget->loadState();
                widget->setVisible(true);

                menu->insertAction(action, widget->toggleViewAction());
                menu->removeAction(action);
            });
        };

        /* ---------------------- Animation Editor -------------------------- */
        addWidgetLazy("Animation Editor", Qt::BottomDockWidgetArea, [this, win]() {
            auto* animationModule = util::getModuleByType<AnimationModule>();
            editor_ = std::make_unique<AnimationEditorDockWidgetQt>(
                animationModule->getWorkspaceAnimations(), animationModule->getAnimationManager(),
                "Animation Editor", getTrackWidgetQtFactory(), getSequenceEditorFactory(), win);

            QObject::connect(editor_.get(), &QObject::destroyed,
                             [this](QObject*) { editor_.release(); });
            return editor_.get();
        });

        /* ---------------------- Demo Navigator ---------------------------- */
        addWidgetLazy("Demo Navigator", Qt::RightDockWidgetArea, [this, win]() {
            auto* animationModule = util::getModuleByType<AnimationModule>();
            auto& demoController = animationModule->getDemoController();
            navigator_ =
                std::make_unique<DemoNavigatorDockWidgetQt>(demoController, "Demo Navigator", win);

            QObject::connect(navigator_.get(), &QObject::destroyed,
                             [this](QObject*) { navigator_.release(); });
            return navigator_.get();
        });

        /* ---------------------- Presentation View (NY) ------------------- */
        addWidgetLazy("Presentation View", Qt::BottomDockWidgetArea, [this, win]() {
            /* Skapa själva dock-widgeten */
            auto* dock = new InviwoDockWidget(utilqt::toQString(std::string("Presentation View")),
                                              win, "PresentationViewWidget");

            dock->setFloating(true);
            dock->setSticky(true);
            dock->setWindowIcon(QIcon(":/animation/icons/monitor.svg"));
            dock->setWidget(new PresentationViewPanel());

            presentationView_.reset(dock);  // spara pekaren

            QObject::connect(dock, &QObject::destroyed,
                             [this](QObject*) { presentationView_.release(); });
            return dock;
        });
    }

    /* ------------------- registrera track-widgets m.m. -------------------- */

    registerTrackWidgetQt<PropertyTrackWidgetQt>();
    registerTrackWidgetQt<ControlTrackWidgetQt>();

    registerSequenceEditor<PropertySequenceEditor>();
    registerSequenceEditor<ControlSequenceEditor>();
    registerSequenceEditor<InvalidationSequenceEditor>();

    using Types = std::tuple<float, vec2, vec3, vec4, mat2, mat3, mat4, double, dvec2, dvec3, dvec4,
                             dmat2, dmat3, dmat4, int, ivec2, ivec3, ivec4, unsigned int, uvec2,
                             uvec3, uvec4, size_t, size2_t, size3_t, size4_t>;

    util::for_each_type<Types>{}(Reghelper<OrdinalProperty>{}, *this);
    util::for_each_type<Types>{}(Reghelper<OrdinalRefProperty>{}, *this);

    util::for_each_type<std::tuple<float, double, int, unsigned int, size_t>>{}(
        Reghelper<MinMaxProperty>{}, *this);

    util::for_each_type<std::tuple<float, double, int, unsigned int, size_t, std::string>>{}(
        Reghelper<OptionProperty>{}, *this);

    util::for_each_type<std::tuple<BoolProperty, FileProperty, StringProperty>>{}(
        [&]<typename Prop>() {
            registerPropertyTrackHelper<Prop, ValueKeyframe<typename Prop::value_type>>(*this);
        });

    registerPropertyTrackHelper<CameraProperty, CameraKeyframe>(*this);

    registerTrackToWidgetMap(ButtonTrack::classIdentifier(),
                             PropertyTrackWidgetQt::classIdentifier());
    registerTrackToWidgetMap(CallbackTrack::classIdentifier(),
                             PropertyTrackWidgetQt::classIdentifier());
    registerTrackToWidgetMap(ControlTrack::classIdentifier(),
                             ControlTrackWidgetQt::classIdentifier());
    registerTrackToWidgetMap(InvalidationTrack::classIdentifier(),
                             ControlTrackWidgetQt::classIdentifier());

    registerTrackToSequenceEditorMap(ControlTrack::classIdentifier(),
                                     ControlSequenceEditor::classIdentifier());
    registerTrackToSequenceEditorMap(InvalidationTrack::classIdentifier(),
                                     InvalidationSequenceEditor::classIdentifier());
}

/* ------------------------------------------------------------------------- */
/*                                  Destruktor                               */
/* ------------------------------------------------------------------------- */

AnimationQtModule::~AnimationQtModule() {
    /* Se till att unregister sker innan fabrikerna förstörs */
    unRegisterAll();
}

/* ------------------------------------------------------------------------- */
/*                               Access-funktioner                           */
/* ------------------------------------------------------------------------- */

animation::TrackWidgetQtFactory& AnimationQtModule::getTrackWidgetQtFactory() {
    return trackWidgetQtFactory_;
}
const animation::TrackWidgetQtFactory& AnimationQtModule::getTrackWidgetQtFactory() const {
    return trackWidgetQtFactory_;
}

animation::SequenceEditorFactory& AnimationQtModule::getSequenceEditorFactory() {
    return sequenceEditorFactory_;
}
const animation::SequenceEditorFactory& AnimationQtModule::getSequenceEditorFactory() const {
    return sequenceEditorFactory_;
}

}  // namespace inviwo
