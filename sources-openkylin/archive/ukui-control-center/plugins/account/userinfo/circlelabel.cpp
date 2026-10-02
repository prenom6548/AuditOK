/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "circlelabel.h"

#include <QPainter>
#include <QMouseEvent>


CircleLabel::CircleLabel(QWidget *parent)
    : QLabel(parent)
{
}

CircleLabel::CircleLabel(QPixmap pixmap, QWidget *parent)
    : QLabel(parent)
{
    setPixmap(pixmap);
}

CircleLabel::~CircleLabel()
{
}

void CircleLabel::setPixmap(QPixmap pixmap)
{
    mPixmap = pixmap;
    update();
}

void CircleLabel::paintEvent(QPaintEvent *event)
{
   QPainter palinter(this);

   palinter.setRenderHint(QPainter::Antialiasing, true);
   palinter.setRenderHint(QPainter::HighQualityAntialiasing);
   palinter.setRenderHint(QPainter::SmoothPixmapTransform);

   int radius = qMin(width(), height()) / 2;
   QPoint center(width() / 2, height() / 2);

   palinter.setPen(QPen(Qt::transparent, -1, Qt::SolidLine));
   palinter.setBrush(Qt::white);
   palinter.drawEllipse(center, radius, radius);

   QPixmap pixmapa(mPixmap);

   palinter.drawPixmap(0, 0, radius * 2, radius * 2, pixmapa);
}

void CircleLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
    }
}

void CircleLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPushButton *button = qobject_cast<QPushButton *>(this->parent());
        if (button != nullptr) {
            button->setChecked(true);
            Q_EMIT button->clicked();
        }
    }

}
