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
#include "fixbutton.h"
#include <QFontMetrics>
#include <QDebug>

FixButton::FixButton(QWidget *parent):
    QPushButton(parent)
{

}

FixButton::~FixButton()
{

}

void FixButton::paintEvent(QPaintEvent *event)
{
    QFontMetrics fontMetrics(this->font());
    int fontSize = fontMetrics.width(mStr);
    int showTextSize = this->width() - 32;
    if (fontSize > showTextSize) {
        this->setText(fontMetrics.elidedText(mStr, Qt::ElideRight, showTextSize), false);
        this->setToolTip(mStr);
    } else {
        this->setText(mStr, false);
        this->setToolTip("");
    }
    QPushButton::paintEvent(event);
}

void FixButton::setText(const QString & text, bool saveTextFlag)
{
    if (saveTextFlag)
        mStr = text;
    QPushButton::setText(text);
}
