#include "modules/animationqt/presentationviewpanel.h"
#include <modules/animationqt/factories/trackwidgetqtfactory.h>
#include <modules/animation/animationcontroller.h>

//PresentationViewPanel::PresentationViewPanel(/**/ AnimationController* controller, QWidget parent)
//    : QWidget(parent), controller_(controller) {
//    setupUI();
//}
PresentationViewPanel::PresentationViewPanel()
     {
    setupUI();
    resize(1244, 700);
}

void PresentationViewPanel::setupUI() {
    animationDisplay_ = new QLabel("Animation Display", this);
    playButton_ = new QPushButton("Play", this);
    pauseButton_ = new QPushButton("Pause", this);
    nextButton_ = new QPushButton("Next", this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(animationDisplay_);
    layout->addWidget(playButton_);
    layout->addWidget(pauseButton_);
    layout->addWidget(nextButton_);

    setLayout(layout);

    connect(playButton_, &QPushButton::clicked, this, &PresentationViewPanel::playanimation);
    connect(pauseButton_, &QPushButton::clicked, this, &PresentationViewPanel::pauseanimation);
    connect(nextButton_, &QPushButton::clicked, this, &PresentationViewPanel::nextanimation);
}

void PresentationViewPanel::playanimation() {
    //controller_.resume();
    updatedisplay();
}

void PresentationViewPanel::pauseanimation() {
    //controller_.pause();
    updatedisplay();
}

void PresentationViewPanel::nextanimation() {
    //controller_.loadNext();
    updatedisplay();
}

void PresentationViewPanel::updatedisplay() {
    animationDisplay_->setText(
        "Current Time: ");  //+
                               //QString::number(controller_.getCurrentTime().count()));
}
