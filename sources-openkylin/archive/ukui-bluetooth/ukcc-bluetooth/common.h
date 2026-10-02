/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef COMMON_H
#define COMMON_H

#include <QObject>

#define STR_MOUSE       "MOUSE"
#define STR_TOUCHPAD    "TOUCHPAD"
#define STR_KEYBOARD    "KEYBOARD"
#define STR_TRACKPOINT  "TRACKPOINT"

class Common : public QObject
{
    Q_OBJECT
public:
    explicit Common(QObject *parent = nullptr);
    static bool isWayland();
    static int getSystemCurrentMouseAndTouchPadDevCount();
    static int getSystemCurrentKeyBoardDevCount();
signals:

};

#endif // COMMON_H
