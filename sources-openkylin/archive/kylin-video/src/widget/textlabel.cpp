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
#include "textlabel.h"

TextLabel::TextLabel(QWidget *parent) :
    QLabel(parent)
{
}

void TextLabel::updateTextShow()
{
    QFontMetrics fontM(font());
    int rows = (height()+2) / fontM.height();

    QString show_text = "";
    int index = 0;
    while (true) {
        QString tmp_text = "";
        if (rows == 1 || index == m_text.length()) {
            tmp_text = m_text.right(m_text.length() - index);
            show_text += fontM.elidedText(tmp_text, Qt::ElideRight, width());
            break;
        }
        while (true) {
            tmp_text.append(m_text.at(index));
            index++;
            if (fontM.width(tmp_text) > width()) {
                tmp_text.chop(1);
                index--;
                break;
            }
            if (index == m_text.length())
                break;
        }
        show_text += tmp_text;
        if (show_text != m_text)
            show_text.append("\n");
        rows--;
    }
    QLabel::setText(show_text);
}

void TextLabel::resizeEvent(QResizeEvent *e)
{
    QLabel::resizeEvent(e);
    updateTextShow();
}
