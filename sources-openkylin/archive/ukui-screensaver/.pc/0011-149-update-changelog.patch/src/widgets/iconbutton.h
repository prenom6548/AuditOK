/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QGSettings>

enum ICONBUTTONTYPE {
    EDITBUTTON,
    NORMALBUTTON,
};

class IconButton:public QPushButton
{
    Q_OBJECT
public:
    IconButton(int buttonType, QWidget* parent = nullptr);

    ~IconButton();

protected:
    void paintEvent(QPaintEvent * event);

private:
    static QColor mixColor(const QColor &c1, const QColor &c2, qreal bias = 0.5);

private:
    int m_buttonType = NORMALBUTTON;
};

#endif // ICONBUTTON_H
