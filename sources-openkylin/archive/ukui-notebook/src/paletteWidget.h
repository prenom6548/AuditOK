/*
* Copyright (C) 2020 Tianjin KYLIN Information Technology Co., Ltd.
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

#ifndef PALETTEWIDGET_H
#define PALETTEWIDGET_H

#include <QWidget>
#include <QStyleOption>
#include <QPainter>
#include <QPainterPath>
#include <QBitmap>

class PaletteWidget
{

public:
    explicit PaletteWidget();
    ~PaletteWidget();

    static QString KY_BLUE;
    static QString KY_PURPLE;
    static QString KY_PINK;
    static QString KY_RED;
    static QString KY_ORANGE;
    static QString KY_YELLOW;
    static QString KY_GREEN;
    static QString KY_GREY;

};

#endif // PALETTEWIDGET_H
