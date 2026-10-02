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
 */

//
// Created by hxf on 22-11-11.
//

#ifndef UKUI_SIDEBAR_WINDOWTYPE_H
#define UKUI_SIDEBAR_WINDOWTYPE_H

#include <qobjectdefs.h>

namespace Sidebar {

/**
 * 预定义的全部窗口类型，可以增加
 */
class SidebarWindowType
{
    Q_GADGET
public:
    enum Type {
        Normal,
        Sidebar,
        NotificationCenter,
        StatusBar,
        RightHandGesture
    };
    Q_ENUM(Type)
};

}

#endif //UKUI_SIDEBAR_WINDOWTYPE_H
