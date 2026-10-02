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

#include <QGSettings>
#include "customtabbar.h"
#include <QHoverEvent>
#include <QApplication>
#include <QPainterPath>
#include <QStyleOptionTab>
#include <QStylePainter>
#include <QDebug>

CustomTabBar::CustomTabBar(QWidget *parent) : QTabBar(parent)
{
}


void CustomTabBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    int radius = 12;
    for (int i = 0; i < count(); ++i) {
        QStyleOptionTab opt;
        initStyleOption(&opt, i);
        QRect r = this->tabRect(i);

        QPainterPath path;
        if (count() == 1) {
            // 只有一个tab，左上和右上都画圆角
            path.moveTo(r.left() + radius, r.top());
            path.lineTo(r.right() - radius, r.top());
            path.arcTo(r.right() - radius * 2, r.top(), radius * 2, radius * 2, 90, -90); // 右上圆角
            path.lineTo(r.right(), r.bottom());
            path.lineTo(r.left(), r.bottom());
            path.lineTo(r.left(), r.top() + radius);
            path.arcTo(r.left(), r.top(), radius * 2, radius * 2, 180, -90); // 左上圆角
            path.closeSubpath();
        } else if (i == 0) {
            // 左侧tab：左上圆角，其他直角
            path.moveTo(r.left() + radius, r.top());
            path.lineTo(r.right(), r.top());
            path.lineTo(r.right(), r.bottom());
            path.lineTo(r.left(), r.bottom());
            path.lineTo(r.left(), r.top() + radius);
            path.arcTo(r.left(), r.top(), radius * 2, radius * 2, 180, -90);
            path.closeSubpath();
        } else if (i == count() - 1) {
            // 右侧tab：右上圆角，其他直角
            path.moveTo(r.left(), r.top());
            path.lineTo(r.right() - radius, r.top());
            path.arcTo(r.right() - radius * 2, r.top(), radius * 2, radius * 2, 90, -90); // 右上圆角
            path.lineTo(r.right(), r.bottom());
            path.lineTo(r.left(), r.bottom());
            path.closeSubpath();
        } else {
            // 其他tab：全直角
            path.addRect(r);
        }
        painter.save();
        painter.setClipPath(path);
        style()->drawControl(QStyle::CE_TabBarTab, &opt, &painter, this);
        painter.restore();
    }
}
