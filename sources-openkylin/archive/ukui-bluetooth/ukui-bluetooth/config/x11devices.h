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

#ifndef X11DEVICES_H
#define X11DEVICES_H

#include <QObject>

#define STR_MOUSE       "MOUSE"
#define STR_TOUCHPAD    "TOUCHPAD"
#define STR_KEYBOARD    "KEYBOARD"
#define STR_TRACKPOINT  "TRACKPOINT"

class x11Devices : public QObject
{
    Q_OBJECT
public:
    explicit x11Devices(QObject *parent = nullptr);
    static unsigned int getSystemCurrentMouseAndTouchPadDevCount();
    static unsigned int getSystemCurrentKeyBoardDevCount();

};

#endif // X11DEVICES_H
