/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

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
#include "clicklabel.h"
#include <QColor>
#include "../globalsignals.h"
#include "../globalbackupinfo.h"
#include "../gsettingswrapper.h"

ClickLabel::ClickLabel(QWidget *parent) :
    QLabel(parent)
{
}

ClickLabel::~ClickLabel()
{
}

void ClickLabel::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        event->accept();
        emit clicked();
    } else {
        QLabel::mousePressEvent(event);
    }
}

// 光标悬浮时是否变换颜色
void ClickLabel::setChangeHover()
{
    // 设置hover背景色
    QString hoverColor = pluginBtnHoverColor(true);
    this->setStyleSheet(QString("ClickLabel:hover{background-color:%1;border-radius: 6px;}").arg(hoverColor));

    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::styleNameChanged, this, [=]() {
        // 深浅主题切换时，因为调色板已经更换，高亮等颜色已经改变，所以要重新加载图标。
        QString hoverColor = pluginBtnHoverColor(true);
        this->setStyleSheet(QString("ClickLabel:hover{background-color:%1;border-radius: 6px;}").arg(hoverColor));
    });
}

QString ClickLabel::pluginBtnHoverColor(bool hoverFlag)
{
    QColor color1, color2;
    if (this->parent()) {
        QWidget * parent = qobject_cast<QWidget *>(this->parent());
        color1 = parent->palette().color(QPalette::Active, QPalette::Button);
        color2 = parent->palette().color(QPalette::Active, QPalette::BrightText);
    } else {
        color1 = palette().color(QPalette::Active, QPalette::Button);
        color2 = palette().color(QPalette::Active, QPalette::BrightText);
    }
    QColor color;
    qreal r,g,b,a;
    QString hoverColor;
    if ((g_GSettingWrapper.isDarkTheme() && hoverFlag) ||
        (!g_GSettingWrapper.isDarkTheme() && !hoverFlag)) {
        r = color1.redF() * 0.8 + color2.redF() * 0.2;
        g = color1.greenF() * 0.8 + color2.greenF() * 0.2;
        b = color1.blueF() * 0.8 + color2.blueF() * 0.2;
        a = color1.alphaF() * 0.8 + color2.alphaF() * 0.2;
    } else {
        r = color1.redF() * 0.95 + color2.redF() * 0.05;
        g = color1.greenF() * 0.95 + color2.greenF() * 0.05;
        b = color1.blueF() * 0.95 + color2.blueF() * 0.05;
        a = color1.alphaF() * 0.95 + color2.alphaF() * 0.05;
    }
    color = QColor::fromRgbF(r, g, b, a);
    hoverColor = QString("rgba(%1, %2, %3, %4)").arg(color.red())
                                                .arg(color.green())
                                                .arg(color.blue())
                                                .arg(color.alpha());
    return hoverColor;
}
