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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef TIMESLIDER_H
#define TIMESLIDER_H

#include <QSlider>
#include "sliderbar.h"

class QTimer;

//class TimeSlider : public QSlider
class TimeSlider : public SliderBar
{
    Q_OBJECT
    
    friend class unitConstruct;  // 可能存在这样的声明
public:
    TimeSlider(QWidget *parent = nullptr);

signals:
    void sigShowPreview(int);
    void sigHidePreview();

private:
    bool m_isPressed,
         m_isPreviewShow;
    int m_mousePos;
    QTimer* m_timerPreview;

protected:
#if 0
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void enterEvent(QEvent *e) override;
#endif
};

#endif // TIMESLIDER_H
