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
#include "languagerestartbar.h"

#include <QApplication>
#include <QPainter>

// 构造语言重启提示条
LanguageRestartBar::LanguageRestartBar(QWidget *parent) :
    QWidget(parent)
{
    setAutoFillBackground(false);
}

// 析构语言重启提示条
LanguageRestartBar::~LanguageRestartBar()
{
}

// 绘制语言重启提示条背景
void LanguageRestartBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (qApp->property("kcontain-secondary-alpha-normal").canConvert<QBrush>()) {
        QBrush containColor = qApp->property("kcontain-secondary-alpha-normal").value<QBrush>();
        painter.setBrush(containColor);
    }

    painter.setPen(Qt::NoPen);

    const int border = qApp->property("kradius-normal").toInt();
    const QRect rect = this->rect();
    painter.drawRoundedRect(rect.x(), rect.y(), rect.width(), rect.height(), border, border);
    painter.restore();

    QWidget::paintEvent(event);
}
