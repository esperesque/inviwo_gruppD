/*********************************************************************************
 * Inviwo - Interactive Visualization Workshop
 * Copyright (c) 2016‑2025 Inviwo Foundation
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
#include <QShortcut>
#include <QVBoxLayout>

namespace inviwo {
namespace animation {

class PresentationViewPanel : public QWidget {
public:
    explicit PresentationViewPanel(WorkspaceAnimations& animations,
                                   AnimationController* controller = nullptr,
                                   QWidget* parent = nullptr);

    void setController(AnimationController* controller);
    void setCamera(CameraProperty* camera);

private:
    /* ---------- bygg UI + timers ---------- */
    void setupUI();
    void updatedisplay();
    void updateAnimationLibrary();
    void updateTimelineHighlight();

    /* ---------- timeline‑hjälp ---------- */
    void clearTimeline();

    /* ---------- animation‑kontroller ---------- */
    void playAnimationById(int animId);
    void jumpRelative(int delta);
    void playanimation();
    void pauseanimation();
    void nextanimation();

    /* ---------- bibliotek & tidslinje ---------- */
    void onLibraryButtonClicked(int animId);
    void onTimelineDoubleClicked(QListWidgetItem* item);

    /* ---------- preset‑exempel ---------- */
    void onPresetClicked();
    void addRotatePreset();
    void rotateCameraBy(float angleRad);

    /* ---------- toolbar ---------- */
    void onToolbarClicked();
    void toggleFullscreen();

    /* ---------- data ---------- */
    WorkspaceAnimations& workspaceAnimations_;
    WorkspaceAnimations::OnChangedDispatcher::Handle onChangedHandle_;
    AnimationController* controller_{nullptr};
    CameraProperty* camera_{nullptr};
    QTimer uiTimer_;

    /* ---------- UI‑element ---------- */
    // toolbar
    QToolButton *tbBreak_, *tbAutoplay_, *tbSpeed_, *tbTransition_, *tbExit_, *tbFullscreen_,
        *tbResetTL_;
    // bibliotek
    QHBoxLayout* libraryLayout_;
    // tidslinje
    QListWidget* timeline_;
    // script‑panel
    QTextEdit* scriptEdit_;
    // preset‑knappar
    QPushButton *btnRotate_, *btnZoom_, *btnShake_;
    // view controls
    QSlider* speedSlider_;
    QLabel* speedLabel_;
    // play/pause
    QPushButton *playButton_, *pauseButton_, *nextButton_;
    // tids‑etikett
    QLabel* timeLabel_;

    /* ---------- genvägar ---------- */
    QShortcut *shortcutNext_, *shortcutPrev_;
};

}  // namespace animation
}  // namespace inviwo
