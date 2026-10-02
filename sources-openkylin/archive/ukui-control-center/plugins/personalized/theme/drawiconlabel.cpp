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
#include "drawiconlabel.h"
#include <QPainter>
#define TOP_LEFT_CORNER 1
#define TOP_RIGHT_CORNER 2
#define BOTTOM_RIGHT_CORNER 4
#define BOTTOM_LEFT_CORNER 8

DrawIconLabel::DrawIconLabel(int radius, QWidget *parent):
    mRadius(radius),
    QLabel(parent)
{
    setScaledContents(true);
}

DrawIconLabel::DrawIconLabel(const QPixmap &pixmap, int radius, QWidget *parent):
    mPixmap(pixmap),
    mRadius(radius),
    QLabel(parent)
{
    setScaledContents(true);
}

QPainterPath DrawIconLabel::makeRoundedRect(const QRect &rect, int radius, int corner)
{
    QPainterPath path;
    if(corner & TOP_LEFT_CORNER) {
        path.moveTo(rect.x() + radius,rect.y());
        path.arcTo(rect.x(), rect.y(), radius*2, radius*2, 90.f, 90.f);//画左上圆角
    } else {
        path.moveTo(rect.x(), rect.y());
    }

    if(corner & BOTTOM_LEFT_CORNER) {
        path.lineTo(rect.x(), rect.y() + rect.height() -radius);
        path.arcTo(rect.x(), rect.y() + rect.height() - radius *2, radius*2, radius*2, 180.0f, 90.f);
    } else {
        path.lineTo(rect.x(), rect.y() + rect.width());
    }

    if(corner & BOTTOM_RIGHT_CORNER) {
        path.lineTo(rect.x() + rect.width() - radius,rect.y() + rect.height());
        path.arcTo(rect.x() + rect.width() - 2*radius, rect.y() + rect.height() - 2*radius,radius*2, radius*2, 270.0f, 90.f);
    } else {
        path.lineTo(rect.x()+ rect.width(), rect.y() + rect.height());
    }

    if(corner & TOP_RIGHT_CORNER) {
        path.lineTo(rect.x() + rect.width(), rect.y() + radius);
        path.arcTo(rect.x() + rect.width() - 2*radius, rect.y(), radius*2,radius*2, 0.f, 90.f);
    } else {
        path.lineTo(rect.x()+ rect.width(), rect.y());
    }
    return path;
}

void DrawIconLabel::setIcon(const QPixmap &pixmap)
{
    mPixmap = pixmap;
}

void DrawIconLabel::setRadius(int radius)
{
    mRadius = radius - 2;
}

void DrawIconLabel::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath picPath = makeRoundedRect(this->rect(), mRadius, TOP_RIGHT_CORNER | BOTTOM_LEFT_CORNER | TOP_LEFT_CORNER | BOTTOM_RIGHT_CORNER);
    p.setClipPath(picPath);
    p.drawPixmap(this->rect(), mPixmap);

    return QLabel::paintEvent(event);
}
