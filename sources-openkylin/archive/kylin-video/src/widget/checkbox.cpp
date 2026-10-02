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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "checkbox.h"

CheckBox::CheckBox(QWidget *parent) :
    QCheckBox(parent)
{

}

CheckBox::~CheckBox()
{

}

void CheckBox::setText(const QString &text)
{
    m_tip = text;
    resetText();
}

void CheckBox::setFont(const QFont &font)
{
    QCheckBox::setFont(font);
    resetText();
}

void CheckBox::resetText()
{
    QFontMetrics fontWidth(font());//得到每个字符的宽度
    QString show_name = fontWidth.elidedText(m_tip, Qt::ElideRight, width() - 30);
    QCheckBox::setText(show_name);
    if (show_name.endsWith("…"))
        setToolTip(m_tip);
    else
        setToolTip("");
}

void CheckBox::resizeEvent(QResizeEvent *e)
{
    resetText();
    QCheckBox::resizeEvent(e);
}
