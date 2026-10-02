/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
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

#include "myframe.h"

MyFrame::MyFrame(QWidget *parent) : QFrame(parent)
{
    this->setObjectName("kylin-burner_MyFrame");
    setFrameShape(QFrame::Box);
    exframe = new kdk::KHLineFrame();
    exframe->setAccessibleName("kylin-burner_KHLineFrame_MyFrame_exframe");
}

void MyFrame::paintEvent(QPaintEvent *event)
{

    QColor color1 = exframe->palette().color(QPalette::Window);

    // 设置画笔颜色为主题颜色
    QPainter painter(this);
    painter.setPen(QPen(color1, 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));

    // 绘制虚线框
    QRect rect = this->rect();
    rect.setX(3);
    rect.setY(3);
    rect.setWidth(rect.width() - 6);
    rect.setHeight(rect.height() - 6);
    painter.drawRoundedRect(rect, 6, 6);
}
