/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
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

#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QHBoxLayout>
#include <QSlider>
#include <QEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QFocusEvent>
#include <QX11Info>
#include <QGraphicsDropShadowEffect>

class SliderWidget : public QDialog
{
    Q_OBJECT

public:
    explicit SliderWidget(QWidget *parent = nullptr);

    QSlider *vSlider = nullptr;

    void changeVolumePos(int posX, int posY, int width, int height);
    void initColor();
    void initUi();
    void setColor(QColor c){m_color = c;}
    void setRadius(int radius){m_radius = radius;repaint();}

protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;   //鼠标滑块点击  事件过滤器
#ifndef WHITE_TEST
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) override;
#endif
private:
    int volunmPosX;
    int volunmPosY;
    int volunmPosWidth;
    int volunmPosHeight;

    QColor m_color;
    int m_radius;

    QHBoxLayout *HLayout = nullptr;

    QTimer* m_focusCheckTimer {nullptr};
};


#endif // SLIDERWIDGET_H
