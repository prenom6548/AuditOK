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
#include "messagebar.h"

#include <QPainter>
#include <QApplication>
MessageBar::MessageBar(QWidget *parent) : QWidget(parent)
{

}

MessageBar::~MessageBar()
{

}

void MessageBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (qApp->property("kcontain-secondary-alpha-normal").canConvert<QBrush>()) {
        QBrush containColor = qApp->property("kcontain-secondary-alpha-normal").value<QBrush>();
        painter.setBrush(containColor);
    }

    if (qApp->property("kline-normal").canConvert<QBrush>()) {
        QBrush borderColor = qApp->property("kline-normal").value<QBrush>();
        painter.setPen(QPen(borderColor,1));
    }

    int border = qApp->property("kradius-normal").toInt();

    QRect rect = this->rect();
    painter.drawRoundedRect(rect.x(), rect.y(), rect.width(), rect.height(), border, border);

    QWidget::paintEvent(event);
}
