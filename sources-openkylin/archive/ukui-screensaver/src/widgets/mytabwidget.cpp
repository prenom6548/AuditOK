/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include <QApplication>
#include "customtabbar.h"

MyTabWidget::MyTabWidget(QWidget *parent) : QTabWidget(parent) {
    CustomTabBar *tabBar = new CustomTabBar(this);
    setTabBar(tabBar);
}

void MyTabWidget::paintEvent(QPaintEvent *event)
{
    QPalette pal = qApp->palette();
    QColor colorPal = pal.color(QPalette::Base);

    //设置窗体为圆角
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    painter.setBrush(colorPal);
    painter.setPen(Qt::transparent);
    auto rect = this->rect();
    painter.drawRoundedRect(rect, 12, 12);      //窗口圆角

    return QWidget::paintEvent(event);
}
