#include "modules/animationqt/presentationviewpanel.h"
#include <modules/animationqt/factories/trackwidgetqtfactory.h>
#include <modules/animation/animationcontroller.h>
#include <inviwo/core/datastructures/camera/perspectivecamera.h>  // For PerspectiveCamera
#include <QTimer>                                                 // For timer functionality
#include <glm/gtc/constants.hpp>                                  // For glm::pi()
#include <modules/animation/datastructures/propertytrack.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animation/animationcontroller.h>
namespace inviwo {
namespace animation {
PresentationViewPanel::PresentationViewPanel(AnimationController* controller, QWidget* parent)
    : QWidget(parent)
    , controller_(controller)
    , rotatingCamera_(false)
    , rotationTimer_(nullptr)
    , camera_(nullptr) {
    setupUI();
    resize(1244, 700);
}

PresentationViewPanel::PresentationViewPanel()
    : rotatingCamera_(false), rotationTimer_(nullptr), camera_(nullptr) {
    setupUI();
    resize(1244, 700);
}

void PresentationViewPanel::setupUI() {
    animationDisplay_ = new QLabel("Animation Display", this);
    playButton_ = new QPushButton("Play", this);
    pauseButton_ = new QPushButton("Pause", this);
    nextButton_ = new QPushButton("Next", this);
    rotateButton_ = new QPushButton("Toggle Camera Rotation", this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(animationDisplay_);
    layout->addWidget(playButton_);
    layout->addWidget(pauseButton_);
    layout->addWidget(nextButton_);
    layout->addWidget(rotateButton_);

    setLayout(layout);

    connect(playButton_, &QPushButton::clicked, this, &PresentationViewPanel::playanimation);
    connect(pauseButton_, &QPushButton::clicked, this, &PresentationViewPanel::pauseanimation);
    connect(nextButton_, &QPushButton::clicked, this, &PresentationViewPanel::nextanimation);
    connect(rotateButton_, &QPushButton::clicked, this,
            &PresentationViewPanel::toggleCameraRotation);
}

void PresentationViewPanel::playanimation() {
    if (controller_) controller_->play();
    updatedisplay();
}

void PresentationViewPanel::pauseanimation() {
    if (controller_) controller_->pause();
    updatedisplay();
}

void PresentationViewPanel::nextanimation() {
    // controller_.loadNext();
    updatedisplay();
}

void PresentationViewPanel::updatedisplay() {
    if (controller_) {
        double currentTimeSec = controller_->getCurrentTime().count() / 1000.0;
        animationDisplay_->setText(QString("Current Time: %1 s").arg(currentTimeSec, 0, 'f', 2));
    } else {
        animationDisplay_->setText("Controller not connected.");
    }
}

void PresentationViewPanel::setCamera(CameraProperty* camera) {
    camera_ = camera;  // Set the camera reference
}

void PresentationViewPanel::toggleCameraRotation() {
    if (rotatingCamera_) {
        // Stop rotation
        if (rotationTimer_) {
            rotationTimer_->stop();
        }
        rotatingCamera_ = false;
    } else {
        // Start rotation
        rotatingCamera_ = true;
        if (!rotationTimer_) {
            rotationTimer_ = new QTimer(this);
            connect(rotationTimer_, &QTimer::timeout, this, &PresentationViewPanel::rotateCamera);
        }
        rotationTimer_->start(
            100);  // Adjust the speed of the rotation by changing the timeout (100ms)
    }
}

void PresentationViewPanel::rotateCamera() {
    if (!camera_) return;

    // Get the current camera position and target from the CameraProperty
    glm::vec3 lookFrom = camera_->getLookFrom();
    glm::vec3 lookTo = camera_->getLookTo();

    // Define the rotation angle
    float angle = glm::radians(1.0f);  // 1 degree
    glm::vec3 direction = glm::normalize(lookFrom - lookTo);

    // Create a rotation matrix around the Y-axis (up axis)
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 newLookFrom = rotationMatrix * glm::vec4(lookFrom, 1.0f);

    // Clone the current camera
    CameraProperty* cameraClone = camera_->clone();  // Clone the CameraProperty

    // Apply the new position after the rotation to the cloned camera
    cameraClone->setLookFrom(glm::vec3(newLookFrom));  // Update the look-from after rotation

    // Set the cloned camera (updated with new position) back to camera_
    camera_->set(cameraClone);  // Pass the CameraProperty pointer to set()

    // Optionally, add a keyframe if the controller exists
    if (controller_) {
        auto& animation = controller_->getAnimation();
        auto t = animation.getLastTime() + animation::Seconds(0.1);
        animation.addKeyframe(camera_, t);  // Add keyframe to the animation
    }
    
    // Update the display after the camera change
    updatedisplay();
}






}  // namespace animation
}  // namespace inviwo
