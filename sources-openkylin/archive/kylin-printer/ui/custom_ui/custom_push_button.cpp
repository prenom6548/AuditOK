/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#include "custom_push_button.h"
#include <QDebug>
#include "common_ui.h"

CustomPushButton::CustomPushButton(QWidget *parent)
    : kdk::KPushButton(parent)
{

}

CustomPushButton::CustomPushButton(QString &text, QWidget *parent)
    : kdk::KPushButton(parent), m_fullText(text)
{

}

void CustomPushButton::setText(const QString &text)
{
    setFullText(text);
}

void CustomPushButton::setFullText(const QString &text)
{
    m_fullText = text;
    update();
}

void CustomPushButton::setTextLimitShrink(const QString &text, int width)
{
    this->setMinimumWidth(qMin(this->fontMetrics().width(text), width));
    setFullText(text);
}

void CustomPushButton::setTextLimitExpand(const QString &text)
{
    int textWidth = this->fontMetrics().width(text);
    this->setMaximumWidth(textWidth);
    setFullText(text);
}

QString CustomPushButton::fullText() const
{
    return m_fullText;
}

void CustomPushButton::paintEvent(QPaintEvent *event)
{
    kdk::KPushButton::paintEvent(event);
    elideText();
}

void CustomPushButton::elideText()
{
    int margin = 32;
    QFontMetrics fm = this->fontMetrics();
    int dif = fm.width(m_fullText) + margin - this->width() ;
    if (dif > 0) {
        QString showText = fm.elidedText(m_fullText, Qt::ElideRight, this->width() - margin);
        kdk::KPushButton::setText(showText);
        if (showText != m_fullText) {
            QString str = dealMessage(m_fullText);
            this->setToolTip(str);
        } else {
            this->setToolTip("");
        }
    } else {
        kdk::KPushButton::setText(m_fullText);
        this->setToolTip("");
    }
}
