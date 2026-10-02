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
#include "custom_check_box.h"
#include <QDebug>
#include "common_ui.h"
#include <QApplication>
#include <QStyle>

CustomCheckBox::CustomCheckBox(QWidget *parent)
    : QCheckBox(parent)
{
    // 这个让使用者自己设置
    // this->setMinimumWidth(0);
    // setTextFormat(Qt::PlainText);
}

CustomCheckBox::CustomCheckBox(const QString &text, QWidget *parent)
    : QCheckBox(text, parent), m_fullText(text)
{
    // this->setMinimumWidth(0);
    // setTextFormat(Qt::PlainText);
}

void CustomCheckBox::setText(const QString &text)
{
    setFullText(text);
}

void CustomCheckBox::setFullText(const QString &text)
{
    m_fullText = text;
    update();
}

void CustomCheckBox::setTextLimitShrink(const QString &text, int width)
{
    this->setMinimumWidth(qMin(this->fontMetrics().width(text), width));
    setFullText(text);
}

void CustomCheckBox::setTextLimitExpand(const QString &text)
{
    int textWidth = this->fontMetrics().width(text);
    this->setMaximumWidth(textWidth);
    setFullText(text);
}

QString CustomCheckBox::fullText() const
{
    return m_fullText;
}

void CustomCheckBox::paintEvent(QPaintEvent *event)
{
    QCheckBox::paintEvent(event);
    elideText();
}

void CustomCheckBox::elideText()
{
    QFontMetrics fm = this->fontMetrics();
    int offset = iconSize().width() + qApp->style()->pixelMetric(QStyle::PM_RadioButtonLabelSpacing);
    int dif = fm.width(m_fullText) - this->width() + offset;
    // 新版主题框架在相等的时候也会有显示不全的问题
    if (dif >= 0) {
        QString showText = fm.elidedText(m_fullText, Qt::ElideRight, this->width() - offset);
        QCheckBox::setText(showText);
        if (showText != m_fullText) {
            QString str = dealMessage(m_fullText);
            this->setToolTip(str);
        } else {
            this->setToolTip("");
        }
    } else {
        QCheckBox::setText(m_fullText);
        this->setToolTip("");
    }
}
