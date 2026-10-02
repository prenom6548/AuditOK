/* Copyright (C) 2020, KylinSoft Co., Ltd.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QRegExp>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QPainterPath>
#include <QGSettings>
#include "colorsquare.h"
#include "gradientslider.h"
#include "view/sizedate.h"
#include "maskwidget.h"
#include "global/variable.h"

class ColorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ColorDialog(QWidget *parent = nullptr);
    ~ColorDialog();

    QLineEdit *m_colorLineEdit;
    QColor color() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    QVBoxLayout *m_Layout;

    ColorSquare *m_colorSquare = nullptr;
    GradientSlider *m_gradientSlider = nullptr;

    QHBoxLayout *m_topLayout;
    QFrame *m_frame1;
    QPushButton *m_closeBtn;
    QSpacerItem *m_TopSpacer;

    QHBoxLayout *m_midLayout;
    QFrame *m_frame2;
    QPushButton *m_colorPicker;

    QHBoxLayout *m_bottomLayout;
    QFrame *m_frame3;

    QVBoxLayout *m_hexLayout;
    QLabel *m_hexLabel;

    QVBoxLayout *m_redLayout;
    QSpinBox *m_spinBoxR;
    QLabel *m_labelR;

    QVBoxLayout *m_greenLayout;
    QSpinBox *m_spinBoxG;
    QLabel *m_labelG;

    QVBoxLayout *m_blueLayout;
    QSpinBox *m_spinBoxB;
    QLabel *m_labelB;

    enum CheckedColor
    {
        H,S,V,R,G,B
    }checkedColor;

    QSpinBox*           spinAlpha;
    GradientSlider*     sliderAlpha;

    QRadioButton*       radioHue;
    QSpinBox*           spinHue;
    GradientSlider*     sliderHue;

    QRadioButton*       radioSat;
    QSpinBox*           spinSat;
    GradientSlider*     sliderSat;

    QRadioButton*       radioVal;
    QSpinBox*           spinVal;
    GradientSlider*     sliderVal;

    QRadioButton*       radioRed;
    QSpinBox*           spinRed;
    GradientSlider*     sliderRed;

    QRadioButton*       radioGreen;
    QSpinBox*           spinGreen;
    GradientSlider*     sliderGreen;

    QRadioButton*       radioBlue;
    QSpinBox*           spinBlue;
    GradientSlider*     sliderBlue;

    QGSettings *m_pGsettingThemeData = nullptr;

    void initLayout();
    void setupInit();
    void signalsBind();
    void drawSlider();
    void setHorizontalSlider();
    void SetColor(QColor color);

private Q_SLOTS:
    void updateWidgetsSlot();
    void setRgbSlot();
    void SetHsvSlot();
    void colorSelectedSolt();

Q_SIGNALS:
    void colorChanged(QColor);
    void checkedChanged(char);
    void colorSelected(QColor);
};

#endif // COLORDIALOG_H
