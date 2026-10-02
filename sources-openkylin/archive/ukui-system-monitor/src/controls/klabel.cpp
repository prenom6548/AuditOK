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
#include "klabel.h"

#include "../util.h"

KLabel::KLabel(QWidget *parent)
    : QLabel(parent)
{
    m_strText = "";
}

KLabel::KLabel(QString strText, QWidget *parent)
    : QLabel(strText, parent)
{
    m_strText = strText;
}

void KLabel::setText(const QString &strText)
{
    m_strText = strText;
    QLabel::setText(strText);
}

void KLabel::paintEvent(QPaintEvent *event)
{
    QString strEText = getElidedText(font(), m_strText, width());
    if (strEText != m_strText) {
        QLabel::setText(strEText);
        setToolTip(m_strText);
    } else {
        QLabel::setText(m_strText);
        setToolTip("");
    }
    QLabel::paintEvent(event);
}
