/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "KAButton.h"


KAButton::KAButton(QWidget *parent)
    : QPushButton(parent)
{
    m_strText = "";
}

KAButton::KAButton(QString strText, QWidget *parent)
    : QPushButton(strText, parent)
{
    m_strText = strText;
}

void KAButton::setText(const QString &strText)
{
    m_strText = strText;
    QPushButton::setText(strText);
}

void KAButton::paintEvent(QPaintEvent *event)
{
    QString strEText = getElidedText(font(),  width() - 28,m_strText);
    if (strEText != m_strText) {
        QPushButton::setText(strEText);
        setToolTip(m_strText);
    } else {
        QPushButton::setText(m_strText);
        setToolTip("");
    }
    QPushButton::paintEvent(event);
}

QString KAButton::getElidedText(QFont font,int width,QString strInfo)
{
    QFontMetrics fontMetrics(font);
    //如果当前字体下，字符串长度大于指定宽度
    if(fontMetrics.width(strInfo) > width)
    {
        strInfo= QFontMetrics(font).elidedText(strInfo, Qt::ElideRight, width);
    }
    return strInfo;
}
