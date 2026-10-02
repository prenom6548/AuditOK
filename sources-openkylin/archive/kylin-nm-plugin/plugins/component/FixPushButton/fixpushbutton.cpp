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
#include "fixpushbutton.h"
#include <QGSettings>

FixPushButton::FixPushButton(QWidget *parent) :
    QPushButton(parent)
{
    const QByteArray id("org.ukui.style");
    QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
    if(QGSettings::isSchemaInstalled(id)){
        connect(fontSetting, &QGSettings::changed,[=](QString key) {
            if ("systemFont" == key || "systemFontSize" ==key) {
                changedLabelSlot();
            }
        });
    }
}


void FixPushButton::setButtonText(QString text) {

    mStr = text;
    changedLabelSlot();
}

QString FixPushButton::getText(){
    return mStr;
}

void FixPushButton::changedLabelSlot() {
    QFontMetrics  fontMetrics(this->font());
    int fontSize = fontMetrics.width(mStr);
    if (fontSize > 65) {
        setText(fontMetrics.elidedText(mStr, Qt::ElideRight, 65));
        setToolTip(mStr);
    } else {
        setText(mStr);
        setToolTip("");
    }
}
