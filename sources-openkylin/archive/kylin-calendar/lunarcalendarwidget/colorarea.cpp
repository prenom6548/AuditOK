/*
* Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include "colorarea.h"
#include <QColor>
#include <QPainter>
#include "calendarcolor.h"
#include <QGSettings>
ColorArea::ColorArea(QWidget *parent) : QWidget(parent)
{
    // 构造函数实现，如果需要的话，可以在这里添加初始化代码

    color1 = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED);

}
ColorArea::~ColorArea()
{

}
void ColorArea::setcolor(const QColor &color)
{
    this_Color = color;
    update(); // 调用update()来标记窗口部件需要重绘
}

void ColorArea::paintEvent(QPaintEvent *event)
{

    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), color1); // 填充窗口部件的颜色
}
