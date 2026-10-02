/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "navgationbutton.h"

#include <QPainter>

NavgationButton::NavgationButton(FringePosition position, QWidget *parent)
    : QPushButton(parent)
    , m_position(position)
{
    this->setFocusPolicy(Qt::NoFocus);
    this->setCheckable(true);
}

void NavgationButton::paintEvent(QPaintEvent *e)
{
    QPushButton::paintEvent(e);

    if (!isChecked()) {
        return;
    }

    QPainter painter(this);
    QRect rect = this->rect();
    if (m_position == Left) {
        rect.setRight(rect.left() + 5);
    }
    else if (m_position == Right) {
        rect.setLeft(rect.right() - 5);
    }
    else if (m_position == Bottom) {
        rect.setTop(rect.bottom() - 2);
    }
    else {
        rect.setBottom(rect.top() + 2);
    }
    painter.fillRect(rect, QColor("#3253bc"));
}
