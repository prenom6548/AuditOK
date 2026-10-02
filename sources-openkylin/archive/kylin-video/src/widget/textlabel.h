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
#ifndef TEXTLABEL_H
#define TEXTLABEL_H

#include <QLabel>

class TextLabel : public QLabel
{
public:
    TextLabel(QWidget *parent = nullptr);

    void setText(const QString &text) {
        m_text = text;
    }

    void updateTextShow();

private:
    QString m_text;

    void resizeEvent(QResizeEvent *e);
};

#endif // TEXTLABEL_H
