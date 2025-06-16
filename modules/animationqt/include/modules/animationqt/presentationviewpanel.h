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
    void captureVisibleCanvasImages(QListWidgetItem* it);
    /* ---------- spel / navigation ---------- */
    void playAnimationById(int id);
    void jumpRelative(int delta);
    void restartPresentation();  // ↺
    void deleteCurrentBox();     // 🗑
    void clearTimelineBoxes();   // 🧹
    void playanimation();
    void pauseanimation();
    void nextanimation();
    void toggleAutoRotate();
    void autoRotate();

    /* ---------- presets ---------- */
    void onPresetClicked();
    void createIdleRotate();
    void createIdleZoom();
    void createIdleShake();
    void addRotatePreset();
    void addZoomPreset();
    void addShakePreset();
    void rotateCameraBy(float angleRad);
    int makeIdleAnim(const std::string& name);
    void startIdleCameraRotate();
    void stopIdleCameraRotate();
    void startIdleZoom();
    void stopIdleZoom();
    void startIdleShake();
    void stopIdleShake();
    /* ---------- toolbar ---------- */
    void onToolbarClicked();
    void toggleFullscreen();

    // ---------- font‐storleksändring ----------
    void increaseScriptFont();
    void decreaseScriptFont();
    void createTransition();
    void buildRuntimeTransition();
    void buildRuntimeCameraTransition();
    void onPlayBakedClicked();  // slot


    private slots:
    void importNotes();

    void bakePresentation();  // render-knappen anropar den
    void onRenderClicked();   // slot

    // Hjälper till att kopiera eller generera keyframes
    void appendAnimation(animation::Animation& src, animation::Seconds off,
                         animation::Animation& dst);
    void addIdleRotate(Animation& dst, Seconds offset);
    void addIdleZoom(Animation& dst, Seconds offset);
    void addIdleShake(Animation& dst, Seconds offset);
    void addCrossfade(Animation& prev, Animation& next, Seconds offset, Seconds dur,
                      Animation& dst);
    void markDirty();  // sätter needsRebake_ och aktiverar Render-knappen
  

    /* ------------------------------------------------------------------ */
    static constexpr int StartId{-1};       ///< ”START”-boxen i tidslinjen
    static constexpr int IdleRotateId{-2};  ///< genererad av   btnRotate_
    static constexpr int IdleZoomId{-3};    ///< genererad av   btnZoom_
    static constexpr int IdleShakeId{-4};   ///< genererad av   btnShake_
    static constexpr int TransitionDummyId{-5};  ///< markerar ”X-fade”-block

    WorkspaceAnimations& workspaceAnimations_;
    WorkspaceAnimations::OnChangedDispatcher::Handle onChangedHandle_;
    AnimationController* controller_{nullptr};
    CameraProperty* camera_{nullptr};
    QTimer uiTimer_;
    QTimer autoRotateTimer_;
    QTimer cameraRotateTimer_;
    QTimer* idleZoomTimer_ = nullptr;
    QTimer* idleShakeTimer_ = nullptr;
    float currentAngle_ = 0.0f;
    QToolButton* btnIdleRotate_ = nullptr;
    bool idleRotateActive_ = false;
    // Save state for consistent rotation
    glm::vec3 idleCenter_;
    glm::vec3 idleUp_;
    glm::vec3 idleInitialFrom_;
    QToolButton* btnIdleZoom_ = nullptr;
    QToolButton* btnIdleShake_ = nullptr;

    bool autoRotateActive = false; // If active, enables idle rotate when an animation finishes
    bool idleZoomActive_ = false;
    bool idleShakeActive_ = false;

    QToolButton* tbRender_{nullptr};  // själva knappen
    Animation* bakedAnim_{nullptr};   // pekar på __pv_baked__
    bool needsRebake_{true};          // tidslinjen ändrad?
    QToolButton* tbPlayBaked_{nullptr};
    bool bakedReady_{false};  // true när __pv_baked__ speglar tidslinjen
    Seconds bakedSegmentEnd_{0};
    bool bakedSegmentPlaying_{false};

              // ny knapp

    QLabel* statusLabel_{nullptr};                 // texten “✔ Rendered!”
                           // true när __pv_baked__ är giltig
    std::vector<animation::Seconds> bakedOffset_;  // t0 för varje ruta
    int currentBakedRow_{0};                       // index i bakedOffset_


    /* ---------- UI-element ---------- */
    // toolbar-knappar
    QToolButton *tbBreak_, *tbAutoplay_, *tbExit_, *tbFullscreen_, *tbRestart_, *tbDelete_,
        *tbClear_, *tbTransition_, *tbAutoRotate_;
    // bibliotek
    QHBoxLayout* libraryLayout_{nullptr};
    // tidslinje
    QListWidget* timeline_{nullptr};
    QWidget* libraryWidget_{nullptr};

    // script / presets / view-controls
         QTextEdit* scriptEdit_{nullptr};
      // --- preset‐ikoner ---
        QToolButton *btnRotate_,
        *btnZoom_, *btnShake_;
    // --- font‐kontroller för scriptEdit_ ---
        QToolButton *btnFontDecrease_, *btnFontIncrease_;
        QPushButton* btnImport_;
        QLabel* scriptFontSizeLabel_{nullptr};
        int scriptFontSize_{16};  

    QSlider* speedSlider_{nullptr};
    QLabel* speedLabel_{nullptr};
    // Transition duration control
    double transitionDuration_{1.0};              // Default duration
    QSlider* transitionDurationSlider_{nullptr};  // The slider UI
    QLabel* transitionDurationLabel_{nullptr};    // Label showing the current duration
    // play/pause/next
    QPushButton *playButton_{nullptr}, *pauseButton_{nullptr}, *nextButton_{nullptr};
    // tid
    QLabel* timeLabel_{nullptr};

    /* ---------- genvägar ---------- */
    QShortcut *shortcutNext_{nullptr}, *shortcutPrev_{nullptr};

    /* ---------- fullscreen-state ---------- */
    QSize previousWindowSize;
    int pendingNextId_{-1};
};

}  // namespace animation
}  // namespace inviwo
