/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Copyright (c) 2016‑2025 Inviwo Foundation
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
#include <QShortcut>
#include <QVBoxLayout>

namespace inviwo {
namespace animation {

/* ------------------------------------------------------------------------- */
/*                     PresentationViewPanel – huvudklass                    */
/* ------------------------------------------------------------------------- */
class PresentationViewPanel : public QWidget {
public:
    explicit PresentationViewPanel(WorkspaceAnimations& animations,
                                   AnimationController* controller = nullptr,
                                   QWidget* parent = nullptr);

    void setController(AnimationController* controller);
    void setCamera(CameraProperty* camera);

private:
    /* ---------- bygg UI ---------- */
    void setupUI();
    void updatedisplay();

    /* ---------- bibliotek / tidslinje ---------- */
    void updateAnimationLibrary();        // fyll blåa knappar
    void updateTimelineHighlight();       // markera aktivt block
    void ensureStartItem();               // ser till att START finns
    void onLibraryButtonClicked(int id);  // lägg till i TL
    void onTimelineDoubleClicked(QListWidgetItem* item);

    /* ---------- spel‑/nav‑funktioner ---------- */
    void playAnimationById(int id);
    void jumpRelative(int delta);  // –1 / +1
    void restartPresentation();    // ↺ återställ allt
    void playanimation();
    void pauseanimation();
    void nextanimation();

    /* ---------- presets ---------- */
    void onPresetClicked();
    void addRotatePreset();
    void rotateCameraBy(float angleRad);

    /* ---------- toolbar ---------- */
    void onToolbarClicked();
    void toggleFullscreen();

    /* ------------------------------------------------------------------ */
    static constexpr int StartId{-1};  // ID för ”START”

    WorkspaceAnimations& workspaceAnimations_;
    WorkspaceAnimations::OnChangedDispatcher::Handle onChangedHandle_;
    AnimationController* controller_{nullptr};
    CameraProperty* camera_{nullptr};

    QTimer uiTimer_;

    /* ---------- UI‑element ---------- */
    // toolbar
    QToolButton *tbBreak_, *tbAutoplay_, *tbSpeed_, *tbTransition_, *tbExit_, *tbFullscreen_,
        *tbRestart_;
    // bibliotek
    QHBoxLayout* libraryLayout_;
    // tidslinje
    QListWidget* timeline_;
    // script
    QTextEdit* scriptEdit_;
    // presets
    QPushButton *btnRotate_, *btnZoom_, *btnShake_;
    // view‑controls
    QSlider* speedSlider_;
    QLabel* speedLabel_;
    // play/pause
    QPushButton *playButton_, *pauseButton_, *nextButton_;
    // tid
    QLabel* timeLabel_;

    QSize previousWindowSize;
    /* ---------- genvägar ---------- */
    QShortcut *shortcutNext_, *shortcutPrev_;
};

}  // namespace animation
}  // namespace inviwo
