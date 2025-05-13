#include "modules/animationqt/presentationviewpanel.h"
#include <modules/animation/datastructures/keyframesequence.h>   //  <-- NY!
#include <modules/animation/datastructures/valuekeyframe.h>

#include <QIcon>
#include <QSize>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QToolButton>
#include <QListWidgetItem>
#include <QStyle>
#include <QFont>
#include <QLabel>
#include <Qt>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

namespace inviwo {
namespace animation {

/* ------------------------------------------------------------------------- */
PresentationViewPanel::PresentationViewPanel(WorkspaceAnimations& animations,
                                             AnimationController* controller, QWidget* parent)
    : QWidget(parent), workspaceAnimations_{animations}, controller_{controller}, uiTimer_{this} {

    setupUI();

    connect(&uiTimer_, &QTimer::timeout, this, &PresentationViewPanel::updatedisplay);
    uiTimer_.start(100);

    onChangedHandle_ =
        animations.onChanged_.add([this](size_t, Animation&) { updateAnimationLibrary(); });
}

/* ------------------------------------------------------------------------- */
void PresentationViewPanel::setupUI() {

    /* ---------- Toolbar ---------- */
    auto* barLayout = new QHBoxLayout;
    barLayout->setSpacing(4);
    const auto makeTool = [&](const QString& txt, auto slot) {
        auto* b = new QToolButton;
        b->setText(txt);
        connect(b, &QToolButton::clicked, this, slot);
        QFont font = b->font();
        font.setPointSize(16);  // ← prova t.ex. 16 eller 20
        b->setFont(font);
        barLayout->addWidget(b);
        return b;
    };

// pausa (“break”)
    tbBreak_ = makeTool("", &PresentationViewPanel::onToolbarClicked);
    tbBreak_->setIcon(QIcon(":/animation/icons/film_movie_pause_player_sound_icon_128.svg"));
    tbBreak_->setIconSize(QSize(24, 24));
    tbBreak_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tbBreak_->setToolTip("Pause");

    // autoplay
    tbAutoplay_ = makeTool("", &PresentationViewPanel::onToolbarClicked);
    tbAutoplay_->setIcon(
        QIcon(":/animation/icons/arrow_play_player_record_right_start_icon_128.svg"));
    tbAutoplay_->setIconSize(QSize(24, 24));
    tbAutoplay_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tbAutoplay_->setToolTip("Play");

    // restart
    tbRestart_ = makeTool("", &PresentationViewPanel::restartPresentation);
    tbRestart_->setIcon(
        QIcon(":/animation/icons/arrow_direction_refresh_repeat_restart_icon_128.svg"));
    tbRestart_->setIconSize(QSize(24, 24));
    tbRestart_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tbRestart_->setToolTip("Restart Presentation");

    // delete current box
    tbDelete_ = makeTool("", &PresentationViewPanel::deleteCurrentBox);
    tbDelete_->setIcon(QIcon(":/animation/icons/basket_delete_garbage_trash_waste_icon_128.svg"));
    tbDelete_->setIconSize(QSize(24, 24));
    tbDelete_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tbDelete_->setToolTip("Delete Current Box");

    // clear timeline
    tbClear_ = makeTool("", &PresentationViewPanel::clearTimelineBoxes);
    tbClear_->setIcon(
        QIcon(":/animation/icons/check_checkmark_good_improve_improved_ok_success_icon_128.svg"));
    tbClear_->setIconSize(QSize(24, 24));
    tbClear_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tbClear_->setToolTip("Clear Timeline");

    // fullscreen
    tbFullscreen_ = makeTool("", &PresentationViewPanel::toggleFullscreen);
    tbFullscreen_->setIcon(QIcon(":/animation/icons/full_movie_screen_video_watch_icon_128.svg"));
    tbFullscreen_->setIconSize(QSize(24, 24));
    tbFullscreen_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tbFullscreen_->setToolTip("Fullscreen");

    // exit
    tbExit_ = new QToolButton;
    tbExit_->setIcon(QIcon(":/animation/icons/arrow_back_direction_reply_icon_128.svg"));
    tbExit_->setIconSize(QSize(24, 24));
    tbExit_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tbExit_->setToolTip("Leave Presentation View");
    // När man klickar: stäng hela widget-fönstret
    connect(tbExit_, &QToolButton::clicked, this, [this]() {
        // stäng hela fönstret (dock-widget eller dialog) som innehåller oss
        window()->close();
    });
    // Lägg till i toolbar-layouten
    barLayout->addWidget(tbExit_);


    barLayout->addStretch(1);

    /* ---------- Bibliotek ---------- */
    libraryLayout_ = new QHBoxLayout;
    libraryLayout_->setSpacing(4);
    libraryWidget_ = new QWidget;  // <-- NYTT
    libraryWidget_->setLayout(libraryLayout_);
    libraryWidget_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);  
    updateAnimationLibrary();

    /* ---------- Timeline ---------- */
    timeline_ = new QListWidget;
    timeline_->setViewMode(QListView::IconMode);
    timeline_->setFlow(QListView::LeftToRight);
    timeline_->setWrapping(false);
    timeline_->setDragDropMode(QAbstractItemView::InternalMove);
    timeline_->setDefaultDropAction(Qt::MoveAction);
    timeline_->setFixedHeight(70);
    timeline_->setSpacing(4);
    connect(timeline_, &QListWidget::itemDoubleClicked, this,
            &PresentationViewPanel::onTimelineDoubleClicked);

    ensureStartItem();


/* ---------- Script / Presets / View-controls ---------- */
    // Layout för anteckningar + font‐kontroller
    auto* notesLayout = new QVBoxLayout;

    // 1) Rad med reduce-ikon, siffra, increase-ikon
    auto* fontCtrlLay = new QHBoxLayout;

    // Minus-knapp
    btnFontDecrease_ = new QToolButton;
    btnFontDecrease_->setIcon(QIcon(":/animation/icons/arrow_bottom_direction_down_icon_128.svg"));
    btnFontDecrease_->setIconSize(QSize(18, 18));
    btnFontDecrease_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnFontDecrease_->setAutoRaise(true);
    btnFontDecrease_->setToolTip("Minska fontstorlek");
    connect(btnFontDecrease_, &QToolButton::clicked, this,
            &PresentationViewPanel::decreaseScriptFont);

    // Fontstorleks-label
    scriptFontSizeLabel_ = new QLabel(QString::number(scriptFontSize_));
    scriptFontSizeLabel_->setAlignment(Qt::AlignCenter);

    // Plus-knapp
    btnFontIncrease_ = new QToolButton;
    btnFontIncrease_->setIcon(QIcon(":/animation/icons/arrow_direction_top_up_icon_128.svg"));
    btnFontIncrease_->setIconSize(QSize(18, 18));
    btnFontIncrease_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnFontIncrease_->setAutoRaise(true);
    btnFontIncrease_->setToolTip("Öka fontstorlek");
    connect(btnFontIncrease_, &QToolButton::clicked, this,
            &PresentationViewPanel::increaseScriptFont);

    // Bygg raden
    fontCtrlLay->addWidget(btnFontDecrease_);
    fontCtrlLay->addWidget(scriptFontSizeLabel_);
    fontCtrlLay->addWidget(btnFontIncrease_);
    fontCtrlLay->addStretch(1);
    notesLayout->addLayout(fontCtrlLay);

    // 2) Själva anteckningsrutan
    scriptEdit_ = new QTextEdit;
    scriptEdit_->setPlaceholderText("Presentation notes …");
    {
        QFont f = scriptEdit_->font();
        f.setPointSize(scriptFontSize_);
        scriptEdit_->setFont(f);
    }
    notesLayout->addWidget(scriptEdit_);

    // Wrap i GroupBox
    auto* scriptBox = new QGroupBox("Script");
    scriptBox->setLayout(notesLayout);


    auto* presetBox = new QGroupBox("Animation select");
    auto* grid = new QGridLayout(presetBox);
    grid->setContentsMargins(4, 4, 4, 4);
    grid->setSpacing(4);

    // Helper för att skapa en ikon-knapp
    auto makePresetButton = [&](const QString& iconName, QToolButton*& btn) {
        btn = new QToolButton;
        btn->setIcon(QIcon(QString(":/animation/icons/%1.svg").arg(iconName)));
        btn->setIconSize(QSize(144, 144));                  // gör ikonen större
        btn->setToolButtonStyle(Qt::ToolButtonIconOnly);  // bara ikon, ingen text
        btn->setAutoRaise(true);                          // platt stil
        btn->setFixedSize(144, 144);                        // fyrkantig knapp
        connect(btn, &QToolButton::clicked, this, &PresentationViewPanel::onPresetClicked);
    };

    makePresetButton("rotate-animation", btnRotate_);
    makePresetButton("zoom-animation", btnZoom_);
    makePresetButton("shakey-animation", btnShake_);

    grid->addWidget(btnRotate_, 0, 0);
    grid->addWidget(btnZoom_, 0, 1);
    grid->addWidget(btnShake_, 0, 2);

    auto* viewBox = new QGroupBox("View controls");
    auto* vLay = new QVBoxLayout(viewBox);
    speedLabel_ = new QLabel("Speed: 50%");
    speedSlider_ = new QSlider(Qt::Horizontal);
    speedSlider_->setRange(1, 100);
    speedSlider_->setValue(50);
    connect(speedSlider_, &QSlider::valueChanged, this,
            [this](int v) { speedLabel_->setText(QString("Speed: %1%").arg(v)); });
    vLay->addWidget(speedLabel_);
    vLay->addWidget(speedSlider_);
    vLay->addStretch(1);

    auto* threeCols = new QHBoxLayout;
    threeCols->addWidget(scriptBox, 2);
    threeCols->addWidget(presetBox, 3);
    threeCols->addWidget(viewBox, 1);

    /* ---------- Bottenknappar ---------- */
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

    /* ---------- Shortcuts ---------- */
    shortcutNext_ = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(shortcutNext_, &QShortcut::activated, [this]() { jumpRelative(+1); });
    shortcutPrev_ = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(shortcutPrev_, &QShortcut::activated, [this]() { jumpRelative(-1); });
    auto* rightArrow = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(rightArrow, &QShortcut::activated, [this]() { jumpRelative(+1); });

    /* ---------- Tid-etikett ---------- */
    timeLabel_ = new QLabel("Current Time: 0.00 s");

    /* ---------- Huvudlayout ---------- */
    auto* main = new QVBoxLayout(this);
    main->addLayout(barLayout);
    main->addWidget(libraryWidget_);  // <-- NYTT
    main->addWidget(timeline_);
    main->addLayout(threeCols);
    main->addWidget(timeLabel_);
    main->addLayout(bottom);
}

/* ------------------------------------------------------------------------- */
/*                         Bibliotek / tidslinje-hjälp                       */
void PresentationViewPanel::ensureStartItem() {
    if (timeline_->count() == 0 || timeline_->item(0)->data(Qt::UserRole).toInt() != StartId) {
        auto* start = new QListWidgetItem("START", timeline_);
        start->setData(Qt::UserRole, StartId);
        start->setSizeHint(QSize(100, 50));
        timeline_->insertItem(0, start);
    }
    timeline_->setCurrentRow(0);
    updateTimelineHighlight();
}

void PresentationViewPanel::updateAnimationLibrary() {
    QLayoutItem* child = nullptr;
    while ((child = libraryLayout_->takeAt(0))) {
        if (auto* w = child->widget()) {  // ❶ frigör själva knappen
            w->deleteLater();             //   (Qt-säkert)
        }
        delete child;  // ❷ radera layoutItem
    }

    for (int i = 0; i < static_cast<int>(workspaceAnimations_.size()); ++i) {
        const auto& anim = workspaceAnimations_.get(i);

        /* hoppa över alla preset-animationer */
        if (anim.getName().rfind("Idle ", 0) == 0) continue;

        auto* btn = new QPushButton(QString::fromStdString(anim.getName()));
        btn->setFixedHeight(28);
        const int maxBtnW = 140;                 // • hur bred en knapp FÅR bli
        btn->setMaximumWidth(maxBtnW);           //   (justera om du vill)
        btn->setSizePolicy(QSizePolicy::Fixed,   // • får inte växa i X-led
                           QSizePolicy::Fixed);  // • eller i Y-led
        connect(btn, &QPushButton::clicked, [this, i]() { onLibraryButtonClicked(i); });
        libraryLayout_->addWidget(btn);
    }

    libraryLayout_->addStretch(1);
}

void PresentationViewPanel::updateTimelineHighlight() {
    for (int i = 0; i < timeline_->count(); ++i) timeline_->item(i)->setBackground(Qt::NoBrush);

    if (auto* cur = timeline_->currentItem())
        cur->setBackground(palette().brush(QPalette::Highlight));
}

/* ------------------------------------------------------------------------- */
void PresentationViewPanel::updatedisplay() {
    if (controller_)
        timeLabel_->setText(
            QString("Current Time: %1 s").arg(controller_->getCurrentTime().count(), 0, 'f', 2));
}

/* ------------------------------------------------------------------------- */
void PresentationViewPanel::onLibraryButtonClicked(int id) {
    ensureStartItem();
    auto* it = new QListWidgetItem(QString::fromStdString(workspaceAnimations_.get(id).getName()),
                                   timeline_);
    it->setData(Qt::UserRole, id);
    it->setSizeHint(QSize(100, 50));
    timeline_->setCurrentItem(it);
    updateTimelineHighlight();
}

void PresentationViewPanel::onTimelineDoubleClicked(QListWidgetItem* item) {
    playAnimationById(item->data(Qt::UserRole).toInt());
}

/* ------------------------------------------------------------------------- */
void PresentationViewPanel::playAnimationById(int id) {
    if (!controller_) return;

    /* --- START-block --- */
    if (id == StartId) {
        controller_->stop();
        return;
    }

    /* --- Idle-presets ------------------------------------ */
    if (id == IdleRotateId) {
        addRotatePreset();
        return;
    }
    if (id == IdleZoomId) {
        addZoomPreset();
        return;
    }
    if (id == IdleShakeId) {
        addShakePreset();
        return;
    }

    /* --- Vanliga (>=0) animationer -------------------------------- */
    if (id >= 0 && id < static_cast<int>(workspaceAnimations_.size())) {

        Animation& anim = workspaceAnimations_.get(id);
        controller_->setAnimation(anim);

        /* -- ställ in lokalt spelläge -- */
        controller_->playModeLocal.set(true);

        /* -- Idle-animationer loopar, andra körs en gång -- */
        const bool isIdle = anim.getName().rfind("Idle ", 0) == 0;
        controller_->playMode.set(isIdle ? PlaybackMode::Loop : PlaybackMode::Once);

        controller_->play();
    }
}


void PresentationViewPanel::jumpRelative(int d) {
    if (timeline_->count() == 0) return;
    int r = timeline_->currentRow();
    do {
        r = (r + d + timeline_->count()) % timeline_->count();
    } while (timeline_->item(r)->data(Qt::UserRole).toInt() == StartId && d != 0);
    timeline_->setCurrentRow(r);
    playAnimationById(timeline_->currentItem()->data(Qt::UserRole).toInt());
    updateTimelineHighlight();
}

/* ---------- 🗑  Delete markerad box ---------- */
void PresentationViewPanel::deleteCurrentBox() {
    if (!timeline_->currentItem() || timeline_->currentRow() == 0) return;  // skydda START
    delete timeline_->takeItem(timeline_->currentRow());
    ensureStartItem();
}

/* ---------- 🧹 Rensa alla animations-boxar ---------- */
void PresentationViewPanel::clearTimelineBoxes() {
    while (timeline_->count() > 1) delete timeline_->takeItem(1);
    ensureStartItem();
}

/* ---------- ↺ Reset allt ---------- */
void PresentationViewPanel::restartPresentation() {
    if (!controller_) return;

    std::unordered_set<int> done;
    for (int i = 0; i < timeline_->count(); ++i) {
        int id = timeline_->item(i)->data(Qt::UserRole).toInt();
        if (id >= 0 && done.insert(id).second) {
            controller_->setAnimation(workspaceAnimations_.get(id));
            controller_->stop();
        }
    }

    ensureStartItem();
    playAnimationById(StartId);
    updatedisplay();
}

/* ------------------------------------------------------------------------- */
void PresentationViewPanel::playanimation() {
    if (controller_) controller_->play();
}
void PresentationViewPanel::pauseanimation() {
    if (controller_) controller_->pause();
}
void PresentationViewPanel::nextanimation() { jumpRelative(+1); }

/* ------------------------------------------------------------------------- */
/* ======================  Idle-preset-skapare  ============================= */
void PresentationViewPanel::createIdleRotate() {
    if (!camera_) return;
    // 1) Skapa en ny animation
    const std::string name = "Idle Rotate " + std::to_string(workspaceAnimations_.size());
    Animation& anim = workspaceAnimations_.add(name);

    // 2) Två keyframes (0 s och 2 s)
    Seconds t0{0}, t1{2};
    anim.addKeyframe(camera_, t0);

    // Rotera 90° runt Y-axeln med bibehållet avstånd
    auto to = camera_->getLookTo();
    auto from = camera_->getLookFrom();
    glm::vec3 dir = from - to;
    glm::vec3 newFrom =
        glm::vec3(glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0, 1, 0)) *
                  glm::vec4(dir, 0)) +
        to;
    camera_->setLookFrom(newFrom);
    anim.addKeyframe(camera_, t1);

    // Återställ property-värdet
    camera_->setLookFrom(from);

    // 3) Lägg blocket sist i tidslinjen
    onLibraryButtonClicked(static_cast<int>(workspaceAnimations_.size() - 1));
}

void PresentationViewPanel::createIdleZoom() {
    if (!camera_) return;
    const std::string name = "Idle Zoom " + std::to_string(workspaceAnimations_.size());
    Animation& anim = workspaceAnimations_.add(name);

    Seconds t0{0}, t1{1}, t2{2};
    auto from = camera_->getLookFrom();
    auto to = camera_->getLookTo();

    anim.addKeyframe(camera_, t0);
    camera_->setLookFrom(to + (from - to) * 0.85f);  // 15 % in
    anim.addKeyframe(camera_, t1);
    camera_->setLookFrom(from);  // tillbaka
    anim.addKeyframe(camera_, t2);

    onLibraryButtonClicked(static_cast<int>(workspaceAnimations_.size() - 1));
}



void PresentationViewPanel::createIdleShake() {
    if (!camera_) return;
    const std::string name = "Idle Shake " + std::to_string(workspaceAnimations_.size());
    Animation& anim = workspaceAnimations_.add(name);

    Seconds t0{0}, t1{0.25}, t2{0.5}, t3{0.75}, t4{1};
    auto base = camera_->getLookFrom();

    auto addKF = [&](Seconds t, float dx, float dy) {
        camera_->setLookFrom(base + glm::vec3(dx, dy, 0));
        anim.addKeyframe(camera_, t);
    };

    anim.addKeyframe(camera_, t0);
    addKF(t1, 0.02f, 0.01f);
    addKF(t2, -0.02f, -0.015f);
    addKF(t3, 0.01f, -0.02f);
    camera_->setLookFrom(base);
    anim.addKeyframe(camera_, t4);

    onLibraryButtonClicked(static_cast<int>(workspaceAnimations_.size() - 1));
}

int PresentationViewPanel::makeIdleAnim(const std::string& name) {
    // 1) skapa Animationen
    Animation& idle = workspaceAnimations_.add(name);

    // 2) få dess index
    const int idx = static_cast<int>(
        std::distance(workspaceAnimations_.begin(), workspaceAnimations_.find(&idle)));

    // 3) uppdatera biblioteket + lägg in i tidslinjen
    updateAnimationLibrary();
    onLibraryButtonClicked(idx);

    // 4) se till att editeraren jobbar mot denna nya animation
    controller_->setAnimation(idle);
    controller_->stop();  // starta i tid = 0

    return idx;
}
void PresentationViewPanel::onPresetClicked() {
    if (!controller_) return;

    // Lägg inte till några rutor här – låt playAnimationById()
    // skapa (eller återanvända) den riktiga idle-animationen.
    if (sender() == btnRotate_) {
        playAnimationById(IdleRotateId);
    } else if (sender() == btnZoom_) {
        playAnimationById(IdleZoomId);
    } else if (sender() == btnShake_) {
        playAnimationById(IdleShakeId);
    }
}



void PresentationViewPanel::addRotatePreset() {
    if (!controller_ || !camera_) return;

    const int idx = makeIdleAnim("Idle Rotate");
    Animation& anim = workspaceAnimations_.get(idx);

    // totalt 8 s → lugn hastighet
    const Seconds dt{2.0};  // 2 s mellan varje segment
    const int nSteps = 4;   // 4 segment ⇒ 4×90° = 360°
    const float dAngle = glm::radians(90.f);

    // spara ursprungs-positionen så vi kan återställa GUI-värdet sedan
    const glm::vec3 startFrom = camera_->getLookFrom();

    // 0-ramen
    anim.addKeyframe(camera_, Seconds{0});

    // fyra 90°-steg: 0 s, 2 s, 4 s, 6 s, 8 s
    for (int i = 1; i <= nSteps; ++i) {
        rotateCameraBy(dAngle);             // +90° runt Y
        anim.addKeyframe(camera_, dt * i);  // nästa keyframe
    }

    // återställ property-värdet i editorn
    camera_->setLookFrom(startFrom);

    // se till att den loopar
    controller_->playModeLocal.set(true);
    controller_->playMode.set(animation::PlaybackMode::Loop);
    controller_->setAnimation(anim);
    controller_->play();
}




void PresentationViewPanel::addZoomPreset() {
    if (!controller_ || !camera_) return;

    const int idx = makeIdleAnim("Idle Zoom");
    Animation& anim = workspaceAnimations_.get(idx);

    Seconds t0{0}, tMid{1.5}, tEnd{3};

    const glm::vec3 from0 = camera_->getLookFrom();
    const glm::vec3 to0 = camera_->getLookTo();
    const glm::vec3 dir = glm::normalize(to0 - from0);

    anim.addKeyframe(camera_, t0);  // start

    camera_->setLookFrom(from0 + dir * 0.30f);  // zoom in ≈30 %
    anim.addKeyframe(camera_, tMid);            // mitten

    camera_->setLookFrom(from0);      // zoom tillbaka
    anim.addKeyframe(camera_, tEnd);  // slut (→ loopar snyggt)
}


void PresentationViewPanel::addShakePreset() {
    if (!controller_ || !camera_) return;

    const int idx = makeIdleAnim("Idle Shake");
    Animation& anim = workspaceAnimations_.get(idx);

    const Seconds dt{0.4};
    const int nSteps = 10;
    const glm::vec3 startPos = camera_->getLookFrom();

    for (int i = 0; i <= nSteps; ++i) {
        float f = (i % 2 == 0 ? 1.f : -1.f);
        camera_->setLookFrom(startPos + glm::vec3{f * 0.05f, 0.f, f * 0.05f});
        anim.addKeyframe(camera_, dt * i);
    }
    camera_->setLookFrom(startPos);  // återställ
}




void PresentationViewPanel::rotateCameraBy(float a) {
    if (!camera_) return;
    camera_->setLookFrom(glm::vec3(glm::rotate(glm::mat4(1.f), a, {0, 1, 0}) *
                                   glm::vec4(camera_->getLookFrom(), 1)));
}

/* ------------------------------------------------------------------------- */
void PresentationViewPanel::onToolbarClicked() { /* reserverad */ }
void PresentationViewPanel::toggleFullscreen() {
    if (window()->isFullScreen()) {
        window()->showNormal();
        window()->resize(previousWindowSize);
    } else {
        previousWindowSize = window()->size();
        window()->showFullScreen();
    }
}

void PresentationViewPanel::increaseScriptFont() {
    scriptFontSize_ = qMin(scriptFontSize_ + 1, 72);
    QFont f = scriptEdit_->font();
    f.setPointSize(scriptFontSize_);
    scriptEdit_->setFont(f);
    // NY RAD:
    scriptFontSizeLabel_->setText(QString::number(scriptFontSize_));
}

void PresentationViewPanel::decreaseScriptFont() {
    scriptFontSize_ = qMax(scriptFontSize_ - 1, 8);
    QFont f = scriptEdit_->font();
    f.setPointSize(scriptFontSize_);
    scriptEdit_->setFont(f);
    // NY RAD:
    scriptFontSizeLabel_->setText(QString::number(scriptFontSize_));
}


/* ------------------------------------------------------------------------- */
void PresentationViewPanel::setController(AnimationController* c) { controller_ = c; }
void PresentationViewPanel::setCamera(CameraProperty* cam) { camera_ = cam; }

}  // namespace animation
}  // namespace inviwo
