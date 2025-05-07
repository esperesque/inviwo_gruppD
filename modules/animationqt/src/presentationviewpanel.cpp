#include "modules/animationqt/presentationviewpanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QToolButton>
#include <QListWidgetItem>
#include <QStyle>
#include <glm/gtc/matrix_transform.hpp>

namespace inviwo {
namespace animation {

/* ------------------------------------------------------------------------- */
/*                               Konstruktor                                 */
/* ------------------------------------------------------------------------- */
PresentationViewPanel::PresentationViewPanel(WorkspaceAnimations& animations,
                                             AnimationController* controller, QWidget* parent)
    : QWidget(parent), workspaceAnimations_{animations}, controller_{controller}, uiTimer_{this} {

    setupUI();

    /* uppdatera tiden var 100 ms */
    connect(&uiTimer_, &QTimer::timeout, this, &PresentationViewPanel::updatedisplay);
    uiTimer_.start(100);

    /* lyssna på förändringar i WorkspaceAnimations */
    onChangedHandle_ =
        animations.onChanged_.add([this](size_t, Animation&) { updateAnimationLibrary(); });
}

/* ------------------------------------------------------------------------- */
/*                               UI‑bygge                                    */
/* ------------------------------------------------------------------------- */
void PresentationViewPanel::setupUI() {
    /* ---------- Toolbar ---------- */
    auto* barLayout = new QHBoxLayout;
    barLayout->setSpacing(4);
    const auto makeTool = [&](const QString& txt) {
        auto* b = new QToolButton;
        b->setText(txt);
        connect(b, &QToolButton::clicked, this, &PresentationViewPanel::onToolbarClicked);
        barLayout->addWidget(b);
        return b;
    };
    tbBreak_ = makeTool("⏸");
    tbAutoplay_ = makeTool("▶");
    tbSpeed_ = makeTool("⚡");
    tbTransition_ = makeTool("⇄");
    tbExit_ = makeTool("⏏");
    tbFullscreen_ = makeTool("⛶");
    tbRestart_ = makeTool("↺");  // ↺ reset‑knapp
    connect(tbFullscreen_, &QToolButton::clicked, this, &PresentationViewPanel::toggleFullscreen);
    connect(tbRestart_, &QToolButton::clicked, this, &PresentationViewPanel::restartPresentation);
    barLayout->addStretch(1);

    /* ---------- Bibliotek ---------- */
    libraryLayout_ = new QHBoxLayout;
    libraryLayout_->setSpacing(4);
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

    ensureStartItem();  // lägg in START först

    /* ---------- Script/preset/view‑boxar ---------- */
    scriptEdit_ = new QTextEdit;
    scriptEdit_->setPlaceholderText("Presentation notes …");
    auto* scriptBox = new QGroupBox("Script");
    auto* scrLay = new QVBoxLayout(scriptBox);
    scrLay->addWidget(scriptEdit_);

    auto* presetBox = new QGroupBox("Animation select");
    auto* grid = new QGridLayout(presetBox);
    btnRotate_ = new QPushButton("Rotate");
    btnZoom_ = new QPushButton("Zoom");
    btnShake_ = new QPushButton("Shake");
    for (auto* b : {btnRotate_, btnZoom_, btnShake_})
        connect(b, &QPushButton::clicked, this, &PresentationViewPanel::onPresetClicked);
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
            [this](int value) { speedLabel_->setText(QString("Speed: %1%").arg(value)); });
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
    auto* rightArrow = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(rightArrow, &QShortcut::activated, [this]() { jumpRelative(+1); });
    shortcutPrev_ = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(shortcutPrev_, &QShortcut::activated, [this]() { jumpRelative(-1); });

    /* ---------- Tid‑etikett ---------- */
    timeLabel_ = new QLabel("Current Time: 0.00 s");

    /* ---------- Huvudlayout ---------- */
    auto* main = new QVBoxLayout(this);
    main->addLayout(barLayout);
    main->addLayout(libraryLayout_);
    main->addWidget(timeline_);
    main->addLayout(threeCols);
    main->addWidget(timeLabel_);
    main->addLayout(bottom);
}

/* ------------------------------------------------------------------------- */
/*                       Bibliotek & tidslinje‑hjälp                         */
/* ------------------------------------------------------------------------- */
void PresentationViewPanel::ensureStartItem() {
    if (timeline_->count() == 0 || timeline_->item(0)->data(Qt::UserRole).toInt() != StartId) {

        auto* start = new QListWidgetItem("START", timeline_);
        start->setData(Qt::UserRole, StartId);
        start->setSizeHint(QSize(100, 50));
        timeline_->insertItem(0, start);
        timeline_->setCurrentRow(0);
    }
}

void PresentationViewPanel::updateAnimationLibrary() {
    QLayoutItem* child;
    while ((child = libraryLayout_->takeAt(0))) delete child;

    for (int i = 0; i < static_cast<int>(workspaceAnimations_.size()); ++i) {
        QString txt = QString::fromStdString(workspaceAnimations_.get(i).getName());
        auto* btn = new QPushButton(txt);
        btn->setFixedHeight(28);
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
    if (controller_) {
        double t = controller_->getCurrentTime().count();
        timeLabel_->setText(QString("Current Time: %1 s").arg(t, 0, 'f', 2));
    }
}

/* ------------------------------------------------------------------------- */
/*              Bibliotek → tidslinje & spelning / navigation                */
/* ------------------------------------------------------------------------- */
void PresentationViewPanel::onLibraryButtonClicked(int id) {
    ensureStartItem();

    auto txt = QString::fromStdString(workspaceAnimations_.get(id).getName());
    auto* it = new QListWidgetItem(txt, timeline_);
    it->setData(Qt::UserRole, id);
    it->setSizeHint(QSize(100, 50));
    timeline_->setCurrentItem(it);
    updateTimelineHighlight();
}

void PresentationViewPanel::onTimelineDoubleClicked(QListWidgetItem* item) {
    int id = item->data(Qt::UserRole).toInt();
    playAnimationById(id);
    updateTimelineHighlight();
}

/* ------------------------------------------------------------------------- */
/*                       Spel‑funktioner & navigation                         */
/* ------------------------------------------------------------------------- */
void PresentationViewPanel::playAnimationById(int id) {
    if (!controller_) return;

    if (id == StartId) {  // START‑block
        controller_->stop();
        return;
    }
    controller_->setAnimation(workspaceAnimations_.get(id));
    controller_->play();
}

void PresentationViewPanel::jumpRelative(int delta) {
    if (timeline_->count() == 0) return;

    int row = timeline_->currentRow();
    do {
        row = (row + delta + timeline_->count()) % timeline_->count();
    } while (timeline_->item(row)->data(Qt::UserRole).toInt() == StartId && delta != 0);

    timeline_->setCurrentRow(row);
    int id = timeline_->currentItem()->data(Qt::UserRole).toInt();
    playAnimationById(id);
    updateTimelineHighlight();
}

/* ---------- Återställ allt ---------- */
void PresentationViewPanel::restartPresentation() {
    if (!controller_) return;

    /* nollställ varje unik animation som finns i tidslinjen */
    std::unordered_set<int> done;
    for (int i = 0; i < timeline_->count(); ++i) {
        int id = timeline_->item(i)->data(Qt::UserRole).toInt();
        if (id != StartId && done.insert(id).second) {
            controller_->setAnimation(workspaceAnimations_.get(id));
            controller_->stop();
        }
    }

    /* hoppa till START */
    ensureStartItem();
    timeline_->setCurrentRow(0);
    playAnimationById(StartId);
    updateTimelineHighlight();
    updatedisplay();
}

void PresentationViewPanel::playanimation() {
    if (controller_) controller_->play();
}
void PresentationViewPanel::pauseanimation() {
    if (controller_) controller_->pause();
}
void PresentationViewPanel::nextanimation() { jumpRelative(+1); }

/* ------------------------------------------------------------------------- */
/*                               Presets                                     */
/* ------------------------------------------------------------------------- */
void PresentationViewPanel::onPresetClicked() {
    if (sender() == btnRotate_) addRotatePreset();
}

void PresentationViewPanel::addRotatePreset() {
    if (!controller_ || !camera_) return;
    auto& anim = controller_->getAnimation();
    Seconds t0 = controller_->getCurrentTime();
    Seconds t1 = t0 + Seconds{2.0};
    anim.addKeyframe(camera_, t0);
    rotateCameraBy(glm::radians(90.f));
    anim.addKeyframe(camera_, t1);
    controller_->play();
}

void PresentationViewPanel::rotateCameraBy(float angleRad) {
    if (!camera_) return;
    auto from = camera_->getLookFrom();
    glm::mat4 R = glm::rotate(glm::mat4(1.f), angleRad, glm::vec3(0, 1, 0));
    camera_->setLookFrom(glm::vec3(R * glm::vec4(from, 1)));
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

/* ------------------------------------------------------------------------- */
void PresentationViewPanel::setController(AnimationController* c) { controller_ = c; }
void PresentationViewPanel::setCamera(CameraProperty* cam) { camera_ = cam; }

}  // namespace animation
}  // namespace inviwo
