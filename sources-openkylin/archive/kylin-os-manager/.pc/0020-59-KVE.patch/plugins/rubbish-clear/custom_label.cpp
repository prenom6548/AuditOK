/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "custom_label.h"
#include <QDebug>
#include "common_ui.h"

CustomLabel::CustomLabel(QWidget *parent, Qt::WindowFlags f) : QLabel(parent, f)
{
    // 这个让使用者自己设置
    // this->setMinimumWidth(0);
    // setTextFormat(Qt::PlainText);
}

CustomLabel::CustomLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f), m_fullText(text)
{
    // this->setMinimumWidth(0);
    // setTextFormat(Qt::PlainText);
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
    QFontMetrics fm = this->fontMetrics();
    int dif = fm.width(m_fullText) - this->width();
    // 新版主题框架在相等的时候也会有显示不全的问题
    if (dif >= 0) {
        QString showText = fm.elidedText(m_fullText, Qt::ElideRight, this->width());
        QLabel::setText(showText);
        if (showText != m_fullText) {
            QString str = dealMessage(m_fullText);
            this->setToolTip(str);
        } else {
            this->setToolTip("");
        }
    } else {
        QLabel::setText(m_fullText);
        this->setToolTip("");
    }
}
