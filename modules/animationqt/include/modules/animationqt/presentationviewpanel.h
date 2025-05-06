/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Copyright (c) 2016-2025 Inviwo Foundation
 *
 *********************************************************************************/
#pragma once

#include <modules/animationqt/animationqtmoduledefine.h>
#include <modules/animation/animationcontroller.h>
#include <modules/animation/workspaceanimations.h>

#include <inviwo/core/datastructures/camera/perspectivecamera.h>
#include <inviwo/core/properties/cameraproperty.h>

#include <QWidget>
#include <QToolButton>
#include <QListWidget>
#include <QTextEdit>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QTimer>

namespace inviwo {
namespace animation {

    //class WorkspaceAnimations;

class PresentationViewPanel : public QWidget {
public:
    explicit PresentationViewPanel(
        WorkspaceAnimations& animations, AnimationController* controller = nullptr,
                                   QWidget* parent = nullptr);

    void setController(AnimationController* controller);
    void setCamera(CameraProperty* camera);

private:
    /* bygg UI + timers */
    void setupUI();
    void updatedisplay();

    /* anim-knappar */
    void playanimation();
    void pauseanimation();
    void nextanimation();

    /* preset-funktioner */
    void onPresetClicked();
    void addRotatePreset();  // första färdiga
    void rotateCameraBy(float angleRad);

    /* toolbar */
    void onToolbarClicked();
    void toggleFullscreen();

    // Update functions
    void updateAnimationList(); // Update the animations listed

    /* ------------------------------------------------------------------ */

    /* kontroller och timing */
    Dispatcher<void()> onAnimationChangeDispatcher_;
    WorkspaceAnimations& workspaceAnimations;
    WorkspaceAnimations::OnChangedDispatcher::Handle onChangedHandle_; // Use this to detect changes in WorkspaceAnimations
    AnimationController* controller_{nullptr};
    CameraProperty* camera_{nullptr};
    QTimer uiTimer_;

    /* toolbar */
    QToolButton *tbBreak_, *tbAutoplay_, *tbSpeed_, *tbTransition_, *tbExit_,
        *tbFullscreen_;

    /* listan ovan */
    QListWidget* customAnimList_;

    /* script-panel */
    QTextEdit* scriptEdit_;

    /* preset-knappar */
    QPushButton *btnRotate_, *btnZoom_, *btnShake_;

    /* view controls */
    QSlider* speedSlider_;
    QLabel* speedLabel_;

    /* play/pause/next */
    QPushButton *playButton_, *pauseButton_, *nextButton_;

    QLabel* timeLabel_;
};

}  // namespace animation
}  // namespace inviwo
