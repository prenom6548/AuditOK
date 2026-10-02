/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "customlabel.h"

CustomLabel::CustomLabel(QWidget *parent)
        : QLabel(parent){

}

CustomLabel::CustomLabel(QString &text, QWidget *parent)
        : QLabel(parent), m_fullText(text)
{

}

void CustomLabel::setText(const QString &text)
{
    setFullText(text);
}

void CustomLabel::setFullText(const QString &text)
{
    m_fullText = text;
    update();
}

void CustomLabel::setTextLimitShrink(const QString &text, int width)
{
    this->setMinimumWidth(qMin(this->fontMetrics().width(text), width));
    setFullText(text);
}

void CustomLabel::setTextLimitExpand(const QString &text)
{
    int textWidth = this->fontMetrics().width(text);
    this->setMaximumWidth(textWidth);
    setFullText(text);
}

QString CustomLabel::fullText() const
{
    return m_fullText;
}

void CustomLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    elideText();
}

void CustomLabel::elideText()
{
    int margin = 32;
    QFontMetrics fm = this->fontMetrics();
    int dif = fm.width(m_fullText) + margin - this->width() ;
    if (dif > 0) {
        QString showText = fm.elidedText(m_fullText, Qt::ElideRight, this->width() - margin);
        QLabel::setText(showText);
        if (showText != m_fullText) {
            QString str = m_fullText;
            this->setToolTip(str);
        } else {
            this->setToolTip("");
        }
    } else {
        QLabel::setText(m_fullText);
        this->setToolTip("");
    }
}
