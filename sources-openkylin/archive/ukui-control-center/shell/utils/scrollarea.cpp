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
#include "scrollarea.h"

#include <QApplication>

ScrollArea::ScrollArea(QWidget *parent) : QScrollArea(parent)
{
    // Tab/程序设焦到插件子控件时，把焦点控件滚入可视区，右侧滚动条随之变化。
    // 仅靠 focusNextPrevChild 不够：Tab 事件落在子控件上时不会进本类 override。
    connect(qApp, &QApplication::focusChanged, this, [this](QWidget *, QWidget *now) {
        if (!now || !widget()) {
            return;
        }
        if (now == widget() || widget()->isAncestorOf(now)) {
            ensureWidgetVisible(now);
        }
    });
}

ScrollArea::~ScrollArea()
{

}

bool ScrollArea::focusNextPrevChild(bool next)
{
    if (QWidget::focusNextPrevChild(next)) {
        if (QWidget *fw = focusWidget()) {
            ensureWidgetVisible(fw);
        }
        return true;
    }
    return false;
}
