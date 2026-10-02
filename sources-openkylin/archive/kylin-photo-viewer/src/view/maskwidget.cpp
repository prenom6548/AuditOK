/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#include "maskwidget.h"

#include <QPainter>

MaskWidget::MaskWidget(QWidget *parent) :
    QWidget(parent)
{
    this->setObjectName("Kylin-photo-viewer_QWidget_mask");
    this->setAccessibleName("Kylin-photo-viewer_QWidget_mask");
    this->setAccessibleDescription("mask widget");

//    setAttribute(Qt::WA_TranslucentBackground);
    pWidth = parent->width();
    pHeigh = parent->height();
    pRadius = 6;
    pColor = QString("#ffffff");
    pBorder = 2;
}

MaskWidget::~MaskWidget()
{
}


void MaskWidget::paintEvent(QPaintEvent *event){
    Q_UNUSED(event)
    QPainter painter(this);

    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿

    painter.setPen(QPen(QColor(palette().color(QPalette::Base)), pBorder));

    painter.drawRect(0, 0, pWidth, pHeigh);
    painter.drawRoundedRect(0, 0, pWidth, pHeigh, pRadius, pRadius);
}
