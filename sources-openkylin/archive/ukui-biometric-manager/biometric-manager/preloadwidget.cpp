/*
 * Copyright (C) 2025 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "preloadwidget.h"
#include <QPainter>
#include <QApplication>
#include <QStyle>

PreloadWidget::PreloadWidget(QWidget *parent) : QWidget{ parent } {}

void PreloadWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(this->rect(), this->palette().base().color());
    QRect iconRect(0, 0, 96, 96);
    iconRect.moveCenter(this->rect().center());
    qApp->style()->drawItemPixmap(&p, iconRect, Qt::AlignCenter, QIcon::fromTheme("biometric-manager").pixmap(96, 96));
}
