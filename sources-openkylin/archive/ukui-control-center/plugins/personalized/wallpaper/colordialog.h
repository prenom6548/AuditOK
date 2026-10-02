/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QRadioButton>
#include <QResizeEvent>
#include <QShowEvent>
#include <QSpinBox>
#include <QVBoxLayout>
#include <kdialog.h>
#include <klabel.h>
#include <kpushbutton.h>

#include "colorpreview.h"
#include "colorsquare.h"
#include "gradientslider.h"

using namespace kdk;

class ColorDialog : public KDialog
{
    Q_OBJECT

public:
    explicit ColorDialog(QWidget *parent = nullptr);
    ~ColorDialog();

    QColor color() const;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void initUi();
    void initSetup();
    void signalsBind();
    void updateInputLayout();
    void setupDialogTabOrder();
    void drawSlider();
    void SetVerticalSlider();
    void SetColor(QColor color);

    static const int DIALOG_WIDTH = 420;

    ColorSquare *colorSquare = nullptr;
    GradientSlider *gradientSlider = nullptr;
    ColorPreview *colorPreview = nullptr;

    enum CheckedColor
    {
        H, S, V, R, G, B
    } checkedColor;

    QSpinBox *spinAlpha = nullptr;
    GradientSlider *sliderAlpha = nullptr;

    QRadioButton *radioHue = nullptr;
    QSpinBox *spinHue = nullptr;
    GradientSlider *sliderHue = nullptr;

    QRadioButton *radioSat = nullptr;
    QSpinBox *spinSat = nullptr;
    GradientSlider *sliderSat = nullptr;

    QRadioButton *radioVal = nullptr;
    QSpinBox *spinVal = nullptr;
    GradientSlider *sliderVal = nullptr;

    QRadioButton *radioRed = nullptr;
    QSpinBox *spinRed = nullptr;
    GradientSlider *sliderRed = nullptr;

    QRadioButton *radioGreen = nullptr;
    QSpinBox *spinGreen = nullptr;
    GradientSlider *sliderGreen = nullptr;

    QRadioButton *radioBlue = nullptr;
    QSpinBox *spinBlue = nullptr;
    GradientSlider *sliderBlue = nullptr;

    QHBoxLayout *m_colorPickerHBoxLayout = nullptr;
    QHBoxLayout *m_inputHBoxLayout = nullptr;
    QHBoxLayout *m_hexHBoxLayout = nullptr;
    QHBoxLayout *m_rgbHBoxLayout = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;

    KLabel *m_hexLabel = nullptr;
    QLineEdit *m_colorLineEdit = nullptr;
    KLabel *m_rgbLabel = nullptr;
    QSpinBox *m_spinBoxR = nullptr;
    QSpinBox *m_spinBoxG = nullptr;
    QSpinBox *m_spinBoxB = nullptr;

    KPushButton *m_cancelBtn = nullptr;
    KPushButton *m_okBtn = nullptr;

private slots:
    void updateWidgetsSlot();
    void setRgbSlot();
    void SetHsvSlot();
    void okSlot();

signals:
    void colorChanged(QColor);
    void checkedChanged(char);
    void colorSelected(QColor);
};

#endif // COLORDIALOG_H
