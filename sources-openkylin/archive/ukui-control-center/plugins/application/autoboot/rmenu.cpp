/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#include "rmenu.h"
#include <QEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>

RMenu::RMenu(QWidget *parent):
    QMenu(parent)
{

}

void RMenu::showEvent(QShowEvent *event)
{
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int menuXPos = this->pos().x();
    int menuWidth = this->size().width() - 4;

    if (menuXPos < menuWidth || menuXPos >= (desk_x - this->size().width() - 4))
        return QMenu::showEvent(event);
    int buttonWidth = 36;
    QPoint pos = QPoint(menuXPos - menuWidth + buttonWidth,
                        this->pos().y());
    this->move(pos);
    QMenu::showEvent(event);
}

