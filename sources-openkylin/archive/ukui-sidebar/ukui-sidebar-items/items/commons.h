/*
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
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

//
// Created by hxf on 22-11-9.
//

#ifndef UKUI_SIDEBAR_COMMONS_H
#define UKUI_SIDEBAR_COMMONS_H

#define DEFAULT_COLOR "#40262626"

#include <QObject>

class BackgroundType
{
    Q_GADGET
public:
    enum Type {
        Null,
        DesktopPic,
        ScreensaverPic
    };
    Q_ENUM(Type)
};

#endif //UKUI_SIDEBAR_COMMONS_H
