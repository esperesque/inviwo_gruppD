/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/
#pragma once

#include <modules/animationqt/animationqtmoduledefine.h>  // for IVW_MODULE_ANIMATIONQT_API
#include <modules/animationqt/factories/trackwidgetqtfactory.h>
#include <modules/animation/animationcontrollerobserver.h>    // for AnimationControllerObserver
#include <modules/animation/datastructures/animationstate.h>  // for AnimationState
#include <modules/qtwidgets/inviwodockwidget.h>               // for InviwoDockWidget
#include <modules/animation/animationcontroller.h>
#include <memory>  // for unique_ptr
#include <string>  // for string
#include <inviwo/core/datastructures/camera/perspectivecamera.h> // Inviwo Camera class
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/property.h>

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class QAction;
class QComboBox;
class QMainWindow;
//class QWidget;
//class AnimationController;



namespace inviwo {
namespace animation {

class PresentationViewPanel : public QWidget {
public:
    // PresentationViewPanel( AnimationController* controller, QWidget* parent = nullptr);
    PresentationViewPanel(AnimationController* controller, QWidget* parent = nullptr);
    PresentationViewPanel();
    void setCamera(CameraProperty* camera);
    CameraProperty* getCamera() const { return camera_; }

private:
    void setupUI();
    void updatedisplay();
    void playanimation();
    void pauseanimation();
    void nextanimation();
    void toggleCameraRotation();  // Slot to toggle camera rotation
    void rotateCamera();          // Function to handle camera rotation

    bool rotatingCamera_;    // Whether the camera is rotating
    QTimer* rotationTimer_;  // Timer to update the camera's rotation
    // inviwo::PerspectiveCamera* camera_;   // Pointer to the PerspectiveCamera
    CameraProperty* camera_;  // Pointer to hold the camera reference
    AnimationController* controller_;
    QLabel* animationDisplay_;
    QPushButton* playButton_;
    QPushButton* pauseButton_;
    QPushButton* nextButton_;
    QPushButton* rotateButton_;  // Button to toggle camera rotation
};
}  // namespace animation
}
  // namespace animation

  // namespace inviwo
