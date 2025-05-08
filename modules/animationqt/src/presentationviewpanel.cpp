#include "modules/animationqt/presentationviewpanel.h"

#include <QIcon>
#include <QSize>
#include <QGroupBox>
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

    tbBreak_ = makeTool("‖", &PresentationViewPanel::onToolbarClicked);     // reserverad
    tbAutoplay_ = makeTool("▶", &PresentationViewPanel::onToolbarClicked);  // reserverad
 //   tbExit_ = makeTool("⏏", &PresentationViewPanel::onToolbarClicked);      // reserverad

    tbFullscreen_ = makeTool("", &PresentationViewPanel::toggleFullscreen);
    tbFullscreen_->setIcon( QIcon(":/animation/icons/fullscreen.svg") );
    tbFullscreen_->setIconSize(QSize(24, 24));  // justera efter behov
    tbFullscreen_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tbFullscreen_->setToolTip("Toggle fullscreen");


    tbRestart_ = makeTool("↺", &PresentationViewPanel::restartPresentation);  // reset all
    tbDelete_ = makeTool("🗑", &PresentationViewPanel::deleteCurrentBox);     // delete sel
    tbClear_ = makeTool("🧹", &PresentationViewPanel::clearTimelineBoxes);    // clear TL
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

    ensureStartItem();

    /* ---------- Script / Presets / View-controls ---------- */
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
    main->addLayout(libraryLayout_);
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
    while (auto* c = libraryLayout_->takeAt(0)) delete c;

    for (int i = 0; i < static_cast<int>(workspaceAnimations_.size()); ++i) {
        auto* btn = new QPushButton(QString::fromStdString(workspaceAnimations_.get(i).getName()));
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
    if (id == StartId) {
        controller_->stop();
        return;
    }
    controller_->setAnimation(workspaceAnimations_.get(id));
    controller_->play();
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
        if (id != StartId && done.insert(id).second) {
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
void PresentationViewPanel::onPresetClicked() {
    if (sender() == btnRotate_) addRotatePreset();
}
void PresentationViewPanel::addRotatePreset() {
    if (!controller_ || !camera_) return;
    auto& anim = controller_->getAnimation();
    Seconds t0 = controller_->getCurrentTime(), t1 = t0 + Seconds{2};
    anim.addKeyframe(camera_, t0);
    rotateCameraBy(glm::radians(90.f));
    anim.addKeyframe(camera_, t1);
    controller_->play();
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
/* ------------------------------------------------------------------------- */
void PresentationViewPanel::setController(AnimationController* c) { controller_ = c; }
void PresentationViewPanel::setCamera(CameraProperty* cam) { camera_ = cam; }

}  // namespace animation
}  // namespace inviwo
