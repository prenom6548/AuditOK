/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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

#include "textlabel.h"
#include <QResizeEvent>
#include <QApplication>

TextLabel::TextLabel(QWidget *parent, Qt::WindowFlags f)
    :QLabel(parent, f)
{
}

TextLabel::TextLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    :QLabel(text, parent, f)
{
}

QString TextLabel::realText()
{
    return m_realText.isEmpty() ? text() : m_realText;
}

void TextLabel::setRealText(const QString &text)
{
    m_realText = text;
}

void TextLabel::resizeEvent(QResizeEvent *)
{
    updateElidedText();
}

QSize TextLabel::sizeHint() const
{
    //! \note We change text if can not show all text, but after we changed it,
    //! sizeHint will be change, because the sizeHint is up to size of text,
    //! we make text width shorter, at the same time make sizeHint smaller.
    //! If sizeHint smaller, you will find the width of text didn't widen
    //! as we expected as the widget widened
    QFontMetrics metrics(qApp->font());
    int width = metrics.width(m_realText);
    int height = QLabel::sizeHint().height();

    return QSize(width, height);
}

void TextLabel::updateElidedText()
{
    QString str = realText();

    //! \bug if use font() not qApp->font(), the width of str will error
    //! when you change system font size, it's a bug of Qt?
    QFontMetrics metrics(qApp->font());
    if (metrics.width(str) > width()) {
        str = metrics.elidedText(str, Qt::ElideRight, width() + 1);
    }
    setText(str);

    updateToolTip();
}

void TextLabel::updateToolTip()
{
    if (realText() != text())
        setToolTip(realText());
    else
        setToolTip(QString());
}
