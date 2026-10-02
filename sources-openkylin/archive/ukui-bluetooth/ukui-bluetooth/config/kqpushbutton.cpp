/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "kqpushbutton.h"

KQPushButton::KQPushButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{

}

KQPushButton::KQPushButton(QWidget *parent)
    : QPushButton(parent)
{

}

bool KQPushButton::event(QEvent *e) {
    if (e->type() == QEvent::HoverEnter) {
        emit hovered(true);
    }
    if (e->type() == QEvent::HoverLeave) {
        emit hovered(false);
    }
    return QPushButton::event(e);
}
