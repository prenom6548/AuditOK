/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "mytabwidget.h"
#include <QStyleOption>
#include <QPainter>
#include <QBitmap>

MyTabWidget::MyTabWidget(QWidget *parent) : QTabWidget(parent)
{
    //隐藏标题栏
  //  setWindowFlags(Qt::FramelessWindowHint);//无边框 置顶

    //设置窗口背景透明
//    setAttribute(Qt::WA_TranslucentBackground,false);

    //设置样式

//    QWidget *w = new QWidget(this);
//    QWidget *k = new QWidget(this);
//    this->addTab(w,"111");
//    this->addTab(k,"222");


}

void MyTabWidget::paintEvent(QPaintEvent *p1)
{
    //绘制样式
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);//绘制样式

    QBitmap bmp(this->size());
    bmp.fill();
    QPainter painter(&bmp);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawRoundedRect(bmp.rect(), 12, 12);
    setMask(bmp);
}
