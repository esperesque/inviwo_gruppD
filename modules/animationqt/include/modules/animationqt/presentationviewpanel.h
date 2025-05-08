/*********************************************************************************
 * Inviwo - Interactive Visualization Workshop
 * Copyright (c) 2016-2025 Inviwo Foundation
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
#include <unordered_set>

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
    /* ---------- bygg UI ---------- */
    void setupUI();
    void updatedisplay();

    /* ---------- bibliotek / tidslinje ---------- */
    void ensureStartItem();
    void updateAnimationLibrary();
    void updateTimelineHighlight();
    void onLibraryButtonClicked(int id);
    void onTimelineDoubleClicked(QListWidgetItem* item);

    /* ---------- spel / navigation ---------- */
    void playAnimationById(int id);
    void jumpRelative(int delta);
    void restartPresentation();  // ↺
    void deleteCurrentBox();     // 🗑
    void clearTimelineBoxes();   // 🧹
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
    static constexpr int StartId{-1};

    WorkspaceAnimations& workspaceAnimations_;
    WorkspaceAnimations::OnChangedDispatcher::Handle onChangedHandle_;
    AnimationController* controller_{nullptr};
    CameraProperty* camera_{nullptr};

    QTimer uiTimer_;

    /* ---------- UI-element ---------- */
    // toolbar-knappar
    QToolButton *tbBreak_, *tbAutoplay_, *tbExit_, *tbFullscreen_, *tbRestart_, *tbDelete_,
        *tbClear_;
    // bibliotek
    QHBoxLayout* libraryLayout_{nullptr};
    // tidslinje
    QListWidget* timeline_{nullptr};
    // script / presets / view-controls
    QTextEdit* scriptEdit_{nullptr};
    QPushButton *btnRotate_{nullptr}, *btnZoom_{nullptr}, *btnShake_{nullptr};
    QSlider* speedSlider_{nullptr};
    QLabel* speedLabel_{nullptr};
    // play/pause/next
    QPushButton *playButton_{nullptr}, *pauseButton_{nullptr}, *nextButton_{nullptr};
    // tid
    QLabel* timeLabel_{nullptr};

    /* ---------- genvägar ---------- */
    QShortcut *shortcutNext_{nullptr}, *shortcutPrev_{nullptr};

    /* ---------- fullscreen-state ---------- */
    QSize previousWindowSize;
};

}  // namespace animation
}  // namespace inviwo
