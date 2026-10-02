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
#include "clickfixlabel.h"
#include "shortcutline.h"
#include <QDebug>
#include <QApplication>
#include <QGSettings>

ClickFixLabel::ClickFixLabel(QWidget *parent):
    QLabel(parent)
{
    this->setContentsMargins(8,0,0,0);
    // 不进 Tab：系统/窗口快捷键改为常驻编辑框（与添加对话框一致）
    setFocusPolicy(Qt::NoFocus);
    setFont(qApp->font());
    QGSettings *fontSetting = new QGSettings(QByteArray("org.ukui.style"), QByteArray(), this);
    connect(fontSetting, &QGSettings::changed, this, [=](const QString &key) {
        updateShortcutFont(this, key, true);
    });
}

ClickFixLabel::~ClickFixLabel()
{

}

void ClickFixLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    Q_EMIT doubleClicked();
    return;
}

void ClickFixLabel::paintEvent(QPaintEvent *event)
{
    QFontMetrics fontMetrics(this->font());
    int fontSize = fontMetrics.width(mStr);
    int m_width = this->width() - 8;
    if (fontSize > m_width) {
        this->setText(fontMetrics.elidedText(mStr, Qt::ElideRight, m_width), false);
        this->setToolTip(mStr);
    } else {
        this->setText(mStr, false);
        this->setToolTip("");
    }
    QLabel::paintEvent(event);
}

void ClickFixLabel::setText(const QString & text, bool saveTextFlag)
{
    if (saveTextFlag)
        mStr = text;
    QLabel::setText(text);
}

bool ClickFixLabel::moveFocus(bool next)
{
    return focusNextPrevChild(next);
}
