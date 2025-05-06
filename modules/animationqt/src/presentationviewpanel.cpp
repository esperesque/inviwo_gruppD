#include "modules/animationqt/presentationviewpanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QToolButton>
#include <QWidget>

#include <string>

#include <string>

#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/animationtime.h>
#include <glm/gtc/matrix_transform.hpp>

namespace inviwo {
namespace animation {

/* ------------------------------------------------------------------------- */


PresentationViewPanel::PresentationViewPanel(
    WorkspaceAnimations& animations, AnimationController* controller, QWidget* parent)
    : QWidget(parent), uiTimer_(this), workspaceAnimations(animations) {

    setController(controller);
    setupUI();

    resize(1280, 720);

    connect(&uiTimer_, &QTimer::timeout, this, &PresentationViewPanel::updatedisplay);

    // Connects animations.onChanged to a function so that we can update the animation list
    // Not totally sure how this works, see workspaceanimationsmodel.cpp for better examples
    onChangedHandle_ = animations.onChanged_.add([this](size_t index, Animation& anim) {
        updateAnimationList();
    });

    uiTimer_.start(100);  // 10 fps
}

/* ------------------------------------------------------------------------- */
/*                                UI-layout                                  */
/* ------------------------------------------------------------------------- */

void PresentationViewPanel::setupUI() {

    /* ========= ÖVERSTA TOOLBAR ========= */
    auto* barLayout = new QHBoxLayout;
    barLayout->setSpacing(4);

    auto makeTool = [&](const QString& txt) {
        auto* b = new QToolButton;
        b->setText(txt);  // placeholder – byt till ikon senare
        connect(b, &QToolButton::clicked, this, &PresentationViewPanel::onToolbarClicked);
        barLayout->addWidget(b);
        return b;
    };

    tbBreak_ = makeTool("⏸");
    tbAutoplay_ = makeTool("▶");
    tbSpeed_ = makeTool("⚡");
    tbTransition_ = makeTool("⇄");
    tbExit_ = makeTool("⏏");
    tbFullscreen_ = makeTool("⛶");  // Ny knapp för helskärm
    connect(tbFullscreen_, &QToolButton::clicked, this, &PresentationViewPanel::toggleFullscreen);

    barLayout->addStretch(1);

    /* ========= “Custom animations”-sektion ========= */
    customAnimList_ = new QListWidget;
    customAnimList_->setFixedHeight(60);

    customAnims_ = new QHBoxLayout;
    customAnims_->setSpacing(4);

    // Create an animation button for each defined animation
    updateAnimationList();

    /* Separator-linje */
    auto* line = new QFrame;
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    /* ========= Tre huvudkolumner ========= */
    /* (1) Script-panel */
    scriptEdit_ = new QTextEdit;
    scriptEdit_->setPlaceholderText("Presentation notes ...");
    auto* scriptBox = new QGroupBox("Script");
    auto* scrLay = new QVBoxLayout(scriptBox);
    scrLay->addWidget(scriptEdit_);

    /* (2) Preset-panel */
    auto* presetBox = new QGroupBox("Animation select");
    auto* grid = new QGridLayout(presetBox);

    btnRotate_ = new QPushButton("Rotate");
    btnZoom_ = new QPushButton("Zoom In/Out");
    btnShake_ = new QPushButton("Shake");

    for (auto* b : {btnRotate_, btnZoom_, btnShake_}) {
        connect(b, &QPushButton::clicked, this, &PresentationViewPanel::onPresetClicked);
    }
    grid->addWidget(btnRotate_, 0, 0);
    grid->addWidget(btnZoom_, 0, 1);
    grid->addWidget(btnShake_, 0, 2);

    /* (3) View controls-panel */
    auto* viewBox = new QGroupBox("View controls");
    auto* viewLay = new QVBoxLayout(viewBox);

    speedLabel_ = new QLabel("Speed:");
    speedSlider_ = new QSlider(Qt::Horizontal);
    speedSlider_->setRange(1, 100);
    speedSlider_->setValue(50);

    viewLay->addWidget(speedLabel_);
    viewLay->addWidget(speedSlider_);
    viewLay->addStretch(1);

    /* layouta tre boxar horisontellt */
    auto* threeCols = new QHBoxLayout;
    threeCols->addWidget(scriptBox, 2);
    threeCols->addWidget(presetBox, 3);
    threeCols->addWidget(viewBox, 1);

    /* ========= Bottenknappar ========= */
    playButton_ = new QPushButton("Play");
    pauseButton_ = new QPushButton("Pause");
    nextButton_ = new QPushButton("Next");

    connect(playButton_, &QPushButton::clicked, this, &PresentationViewPanel::playanimation);
    connect(pauseButton_, &QPushButton::clicked, this, &PresentationViewPanel::pauseanimation);
    connect(nextButton_, &QPushButton::clicked, this, &PresentationViewPanel::nextanimation);

    auto* bottom = new QHBoxLayout;
    bottom->addWidget(playButton_);
    bottom->addWidget(pauseButton_);
    bottom->addWidget(nextButton_);

    /* ========= Tid-etikett ========= */
    timeLabel_ = new QLabel("Current Time: 0.00 s");

    /* ========= Huvud-layout ========= */
    auto* main = new QVBoxLayout(this);
    main->addLayout(barLayout);
    main->addLayout(customAnims_);
    main->addWidget(line);
    main->addLayout(threeCols);
    main->addWidget(timeLabel_);
    main->addLayout(bottom);
}

/* ------------------------------------------------------------------------- */
/*                       Kontroller och display-timing                       */
/* ------------------------------------------------------------------------- */

void PresentationViewPanel::updatedisplay() {
    if (controller_) {
        const double t = controller_->getCurrentTime().count();
        timeLabel_->setText(QString("Current Time: %1 s").arg(t, 0, 'f', 2));
    }
}

void PresentationViewPanel::updateAnimationList() {
    //customAnims_->; // Todo: Clear or don't add existing anims
    for (int i = 0; i < workspaceAnimations.size(); i++) {
        std::string anim_name = workspaceAnimations.get(i).getName();
        auto* butt = new QPushButton(QString::fromStdString(anim_name));
        butt->setFixedHeight(50);
        connect(butt, &QPushButton::clicked, this, &PresentationViewPanel::playanimation);
        // Not sure how to send the animation ID. I think we may have to extend QPushButton with a new signal that contains
        // an id
        customAnims_->addWidget(butt);
        //customAnimList_->addItem(QString::fromStdString(anim_name));
        //customAnimList_->addItem(new QPushButton("Play"));

        // Connect the button
    }
}

/* ------------------------------------------------------------------------- */
/*                      Spela, pausa, nästa (enkla)                          */
/* ------------------------------------------------------------------------- */

void PresentationViewPanel::playAnimationById(int anim_id) {
    std::cout << "called \n";
    std::cout << anim_id;
}

void PresentationViewPanel::playanimation() {
    if (controller_) controller_->play();
}
void PresentationViewPanel::pauseanimation() {
    if (controller_) controller_->pause();
}
void PresentationViewPanel::nextanimation() { /* TODO */ }

/* ------------------------------------------------------------------------- */
/*                            PRESET-HANTERING                               */
/* ------------------------------------------------------------------------- */

void PresentationViewPanel::onPresetClicked() {
    if (sender() == btnRotate_) {
        addRotatePreset();
    } else if (sender() == btnZoom_) {
        /* TODO */
    } else if (sender() == btnShake_) {
        /* TODO */
    }
}

void PresentationViewPanel::addRotatePreset() {
    if (!controller_ || !camera_) return;

    auto& anim = controller_->getAnimation();
    Seconds t0 = controller_->getCurrentTime();
    Seconds t1 = t0 + Seconds{2.0};  // 2 sekunders rotation

    /* Nyckelruta A */
    anim.addKeyframe(camera_, t0);

    /* Roterad kamera och nyckelruta B */
    rotateCameraBy(glm::radians(90.f));
    anim.addKeyframe(camera_, t1);

    controller_->play();
}

void PresentationViewPanel::rotateCameraBy(float angleRad) {
    auto from = camera_->getLookFrom();
    auto to = camera_->getLookTo();

    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angleRad, glm::vec3(0.f, 1.f, 0.f));
    glm::vec4 newFrom = rot * glm::vec4(from, 1.0f);

    camera_->setLookFrom(glm::vec3(newFrom));
}

/* ------------------------------------------------------------------------- */
/*                            FULLSKÄRM/VYNÄT                               */
/* ------------------------------------------------------------------------- */

void PresentationViewPanel::toggleFullscreen() {
    QWidget* w = window();
    if (w->isFullScreen()) {
        w->showNormal();
        //återställ storlek
        w->resize(1280, 720);
    } else {
        w->showFullScreen();
    }
}


/* ------------------------------------------------------------------------- */

void PresentationViewPanel::onToolbarClicked() { /* TODO i senare steg */ }

void PresentationViewPanel::setController(AnimationController* ctrl) { controller_ = ctrl; }
void PresentationViewPanel::setCamera(CameraProperty* camera) { camera_ = camera; }

}  // namespace animation
}  // namespace inviwo
