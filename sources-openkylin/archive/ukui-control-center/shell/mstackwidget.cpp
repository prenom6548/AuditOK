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
#include "mstackwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QPainterPath>

MStackWidget::MStackWidget(QWidget *parent)
    : QStackedWidget(parent)
{

}

MStackWidget::~MStackWidget()
{

}

void MStackWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    QPainterPath path;
    path.addRoundedRect(this->rect(),6,6);
    path.setFillRule(Qt::WindingFill); // 多块区域组合填充模式
    path.addRect(width() - 6,height() -6 ,6,6);

    status ? painter.setBrush(this->palette().base()) : painter.setBrush(this->palette().window());
    painter.setPen(Qt::transparent);
    painter.drawPath(path);
    QWidget::paintEvent(event);
}
