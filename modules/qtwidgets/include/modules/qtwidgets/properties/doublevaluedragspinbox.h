/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <QObject>  // for Q_OBJECT, signals, slots
#include <QString>  // for QString
#include <QWidget>  // for QWidget

namespace inviwo {

class NumberLineEdit;
class ValueDragger;

class IVW_MODULE_QTWIDGETS_API DoubleValueDragSpinBox : public QWidget {
    Q_OBJECT
public:
    explicit DoubleValueDragSpinBox(QWidget* parent = nullptr);
    virtual ~DoubleValueDragSpinBox() override = default;

    void setReadOnly(bool r);
    bool isReadOnly() const;

    bool isValid() const;

    void setSpecialValueText(const QString& txt);
    QString specialValueText() const;

    void setWrapping(bool w);
    bool wrapping() const;

    QString text() const;

    QString cleanText() const;
    int decimals() const;
    double maximum() const;
    double minimum() const;
    QString prefix() const;
    void setDecimals(int prec);
    void setMaximum(double max);
    void setMinimum(double min);
    void setPrefix(const QString& prefix);
    void setRange(double minimum, double maximum);
    /**
     * Sets the increment of a single step to @p inc.
     * If @p inc is zero, the spinbox buttons will be hidden.
     * @see QDoubleSpinBox::setSingleStep
     */
    void setSingleStep(double val);
    void setSuffix(const QString& suffix);
    double singleStep() const;
    QString suffix() const;
    double value() const;

signals:
    void valueChanged(double d);
    void valueChanged(const QString& text);
    void editingFinished();

public slots:
    void setValue(double value);
    void selectAll();
    void stepDown();
    void stepUp();
    void setInvalid(bool invalid = true);

private:
    void updateState();

    NumberLineEdit* spinBox_;
    ValueDragger* valueDragger_;

    bool invalid_;
};

}  // namespace inviwo
