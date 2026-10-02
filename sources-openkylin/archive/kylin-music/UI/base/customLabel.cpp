/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "customLabel.h"
#include <QtWidgets>

CustomLabel::CustomLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    this->setMinimumWidth(minSize);
    setTextFormat(Qt::PlainText);
}

CustomLabel::CustomLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f), m_fullText(text)
{
    this->setMinimumWidth(minSize);
    setTextFormat(Qt::PlainText);
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
//    this->setMinimumSize(200);
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
    QFontMetrics fm = this->fontMetrics();
    int dif = fm.width(m_fullText) - this->width();
    if (dif > 0) {
        QString showText = fm.elidedText(m_fullText, Qt::ElideRight, this->width());
        QLabel::setText(showText);
        if (showText != m_fullText) {
//            QString str = dealMessage(m_fullText);
            this->setToolTip(m_fullText);
        } else {
            this->setToolTip("");
        }
    } else {
        QLabel::setText(m_fullText);
        this->setToolTip("");
    }
}

//QString MyLabel::dealMessage(QString msg)
//{
//    if(msg.size() > fontSize)
//    {
//        QString str;
//        int time = msg.size()/fontSize;
//        for(int i = 0; i <= time-1; i++)
//        {
//            str = QString(str + msg.mid(i*fontSize,fontSize)+"\r\n");
//        }
//        str = QString(str+msg.mid(time*fontSize,fontSize));
//        return str;
//    }
//    else
//        return msg;
//}

