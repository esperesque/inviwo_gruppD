#include "modules/animationqt/presentationviewpanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QToolButton>
#include <QListWidgetItem>
#include <glm/gtc/matrix_transform.hpp>

namespace inviwo {
namespace animation {

/* ------------------------------------------------------------------------- */
PresentationViewPanel::PresentationViewPanel(WorkspaceAnimations& animations,
                                             AnimationController* controller, QWidget* parent)
    : QWidget(parent), workspaceAnimations_{animations}, controller_{controller}, uiTimer_{this} {

    setupUI();

    connect(&uiTimer_, &QTimer::timeout, this, &PresentationViewPanel::updatedisplay);
    uiTimer_.start(100);

    onChangedHandle_ = workspaceAnimations_.onChanged_.add(
        [this](size_t, Animation&) { updateAnimationLibrary(); });
}

/* ------------------------------------------------------------------------- */
void PresentationViewPanel::setupUI() {
    /* ---------- Toolbar ---------- */
    auto* barLayout = new QHBoxLayout;
    barLayout->setSpacing(4);
    const auto makeTool = [&](const QString& txt) {
        auto* b = new QToolButton;
        b->setText(txt);
        b->setFixedSize(24, 24);
        connect(b, &QToolButton::clicked, this, &PresentationViewPanel::onToolbarClicked);
        barLayout->addWidget(b);
        return b;
    };
    tbBreak_ = makeTool("⏸");
    tbAutoplay_ = makeTool("▶");
    tbSpeed_ = makeTool("⚡");
    tbTransition_ = makeTool("⇄");
    tbResetTL_ = makeTool("🗑");  // <-- NY reset‑knapp
    tbExit_ = makeTool("⏏");
    tbFullscreen_ = makeTool("⛶");

    connect(tbFullscreen_, &QToolButton::clicked, this, &PresentationViewPanel::toggleFullscreen);
    connect(tbResetTL_, &QToolButton::clicked, this, &PresentationViewPanel::clearTimeline);

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

    /* ---------- Script / presets / view‑boxar ---------- */
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
    speedLabel_ = new QLabel("Speed:");
    speedSlider_ = new QSlider(Qt::Horizontal);
    speedSlider_->setRange(1, 100);
    speedSlider_->setValue(50);
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
    connect(shortcutNext_, &QShortcut::activated, [this] { jumpRelative(+1); });
    auto* rightArrow = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(rightArrow, &QShortcut::activated, [this] { jumpRelative(+1); });
    shortcutPrev_ = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(shortcutPrev_, &QShortcut::activated, [this] { jumpRelative(-1); });

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
void PresentationViewPanel::updateAnimationLibrary() {
    while (auto* child = libraryLayout_->takeAt(0)) delete child;
    for (int i = 0; i < static_cast<int>(workspaceAnimations_.size()); ++i) {
        QString txt = QString::fromStdString(workspaceAnimations_.get(i).getName());
        auto* b = new QPushButton(txt);
        b->setFixedHeight(28);
        connect(b, &QPushButton::clicked, [this, i] { onLibraryButtonClicked(i); });
        libraryLayout_->addWidget(b);
    }
    libraryLayout_->addStretch(1);
}

void PresentationViewPanel::updateTimelineHighlight() {
    for (int i = 0; i < timeline_->count(); ++i) timeline_->item(i)->setBackground(Qt::NoBrush);
    if (auto* cur = timeline_->currentItem())
        cur->setBackground(palette().brush(QPalette::Highlight));
}

/* ---------------- reset ---------------- */
void PresentationViewPanel::clearTimeline() { timeline_->clear(); }

/* ------------------------------------------------------------------------- */
void PresentationViewPanel::updatedisplay() {
    if (!controller_) return;
    double t = controller_->getCurrentTime().count();
    timeLabel_->setText(QString("Current Time: %1 s").arg(t, 0, 'f', 2));
}

/* ---------------- bibliotek → tidslinje ---------------- */
void PresentationViewPanel::onLibraryButtonClicked(int id) {
    QString txt = QString::fromStdString(workspaceAnimations_.get(id).getName());
    auto* it = new QListWidgetItem(txt, timeline_);
    it->setData(Qt::UserRole, id);
    it->setSizeHint(QSize(100, 50));
    timeline_->setCurrentItem(it);
    updateTimelineHighlight();
}

void PresentationViewPanel::onTimelineDoubleClicked(QListWidgetItem* it) {
    int id = it->data(Qt::UserRole).toInt();
    playAnimationById(id);
    updateTimelineHighlight();
}

/* ---------------- spelcontrol ---------------- */
void PresentationViewPanel::playAnimationById(int id) {
    if (!controller_) return;
    controller_->setAnimation(workspaceAnimations_.get(id));
    controller_->play();
}
void PresentationViewPanel::jumpRelative(int d) {
    if (!timeline_->count()) return;
    int r = timeline_->currentRow();
    r = (r + d + timeline_->count()) % timeline_->count();
    timeline_->setCurrentRow(r);
    int id = timeline_->currentItem()->data(Qt::UserRole).toInt();
    playAnimationById(id);
    updateTimelineHighlight();
}
void PresentationViewPanel::playanimation() {
    if (controller_) controller_->play();
}
void PresentationViewPanel::pauseanimation() {
    if (controller_) controller_->pause();
}
void PresentationViewPanel::nextanimation() { jumpRelative(+1); }

/* ---------------- presets ---------------- */
void PresentationViewPanel::onPresetClicked() {
    if (sender() == btnRotate_) addRotatePreset();
}
void PresentationViewPanel::addRotatePreset() {
    if (!controller_ || !camera_) return;
    auto& a = controller_->getAnimation();
    Seconds t0 = controller_->getCurrentTime(), t1 = t0 + Seconds{2};
    a.addKeyframe(camera_, t0);
    rotateCameraBy(glm::radians(90.f));
    a.addKeyframe(camera_, t1);
    controller_->play();
}
void PresentationViewPanel::rotateCameraBy(float r) {
    if (!camera_) return;
    auto f = camera_->getLookFrom();
    glm::mat4 m = glm::rotate(glm::mat4(1.f), r, glm::vec3(0, 1, 0));
    camera_->setLookFrom(glm::vec3(m * glm::vec4(f, 1)));
}

/* ---------------- toolbar helpers ---------------- */
void PresentationViewPanel::onToolbarClicked() {}
void PresentationViewPanel::toggleFullscreen() {
    window()->isFullScreen() ? window()->showNormal() : window()->showFullScreen();
    if (!window()->isFullScreen()) window()->resize(1280, 720);
}

/* ------------------------------------------------------------------------- */
void PresentationViewPanel::setController(AnimationController* c) { controller_ = c; }
void PresentationViewPanel::setCamera(CameraProperty* cam) { camera_ = cam; }

}  // namespace animation
}  // namespace inviwo
