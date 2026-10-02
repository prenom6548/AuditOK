/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#include "ktiplabel.h"
#include <QEvent>
#include <QDebug>
KTipLabel::KTipLabel(QWidget *parent)
    : KLabel(parent)
{
    m_strText = "";
}

KTipLabel::KTipLabel(QString strText, QWidget *parent)
{
    m_strText = strText;
}

void KTipLabel::setText(const QString &strText)
{
    m_strText = strText;
    KLabel::setText(strText);
}

void KTipLabel::paintEvent(QPaintEvent *event)
{
    QString strEText = getElidedText(font(),  width() ,m_strText);
    if (strEText == m_strText) {
        KLabel::setText(m_strText);
    } else if (strEText != m_strText ) {
        KLabel::setText(strEText);
    }
    QLabel::paintEvent(event);
}

QString KTipLabel::getElidedText(QFont font,int width,QString strInfo)
{
    QFontMetrics fontMetrics(font);
    //如果当前字体下，字符串长度大于指定宽度
    if(fontMetrics.width(strInfo) > width)
    {
        strInfo= QFontMetrics(font).elidedText(strInfo, Qt::ElideRight, width);
    }
    return strInfo;
}
