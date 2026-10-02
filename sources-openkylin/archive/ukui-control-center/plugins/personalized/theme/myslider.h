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
#ifndef MYSLIDER_H
#define MYSLIDER_H

#include <QObject>
#include <QSlider>

class MySlider : public QSlider
{
    Q_OBJECT
public:
    MySlider(QWidget *parent = nullptr);
    ~MySlider();
protected:
    void mousePressEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *ev);
    void leaveEvent(QEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

private:
    bool isMouseCliked = false;

Q_SIGNALS:
    void sliderClickedSignals();

};

#endif // MYSLIDER_H
