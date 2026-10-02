/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "freqlabel.h"
#include <QGSettings>
#include <QPainter>
#include <QVariant>

#define FREQLABLE_HIGHT 18
#define FREQLABLE_MARGINS 4,0,4,0
#define RADIUS 6.0

FreqLabel::FreqLabel(QWidget *parent) : QLabel(parent)
{
    const QByteArray id("org.ukui.style");
    QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
    if(QGSettings::isSchemaInstalled(id)){
        connect(fontSetting, &QGSettings::changed,[=](QString key) {
            if ("systemFontSize" ==key) {
                changedFontSlot();
            }
        });
    }
    changedFontSlot();
    setText("...");
    setFixedHeight(FREQLABLE_HIGHT);
    setContentsMargins(FREQLABLE_MARGINS);
}

void FreqLabel::changedFontSlot()
{
    const QByteArray id("org.ukui.style");
    if(QGSettings::isSchemaInstalled(id)){
        QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
        QVariant fontVariant =  fontSetting->get("systemFontSize");
        QFont font;
        font.setPointSize(fontVariant.toInt()*0.85);
        this->setFont(font);
    }
}

void FreqLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); //抗锯齿效果
    auto rect = this->rect();
    painter.drawRoundedRect(rect, RADIUS, RADIUS);
    QLabel::paintEvent(event);
}
