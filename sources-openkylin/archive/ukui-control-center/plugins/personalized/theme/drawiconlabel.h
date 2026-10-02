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
#ifndef DrawIconLabel_H
#define DrawIconLabel_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPainterPath>

class DrawIconLabel : public QLabel
{
    Q_OBJECT
public:
    DrawIconLabel(int radius = 10, QWidget *parent = nullptr);
    DrawIconLabel(const QPixmap &pixmap, int radius = 10, QWidget *parent = nullptr);

    QPainterPath makeRoundedRect(const QRect& rect,int radius,int corner) ;
    void setIcon(const QPixmap &pixmap);
    void setRadius(int radius);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QPixmap mPixmap;
    int mRadius;
};

#endif // DrawIconLabel_H
