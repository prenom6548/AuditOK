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
#include "customscrollarea.h"
#include <QScrollBar>
#include <QDebug>
#include <QGSettings>
#define THEME_SCHAME "org.ukui.style"
#define COLOR_THEME "styleName"
CustomScrollArea::CustomScrollArea(QWidget *parent)
    : QScrollArea(parent)
{
}

void CustomScrollArea::wheelEvent(QWheelEvent *event)
{
    int delta = event->angleDelta().y();
    QScrollBar *vBar = verticalScrollBar();

    // 判断滚动条是否在最顶部或最底部
    bool atTop = (vBar->value() == vBar->minimum());
    bool atBottom = (vBar->value() == vBar->maximum());

    if ((delta > 0 && atTop) || (delta < 0 && atBottom)) {
        TopOrButtom = true;
        // 如果滚动到最顶部且继续向上滚动，或者滚动到最底部且继续向下滚动，则忽略事件
        event->ignore();
    } else {
        // 否则，正常处理事件
        QScrollArea::wheelEvent(event);
    }
}
