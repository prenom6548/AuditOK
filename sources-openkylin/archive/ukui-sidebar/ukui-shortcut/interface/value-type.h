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

#ifndef UKUI_SIDEBAR_VALUE_TYPE_H
#define UKUI_SIDEBAR_VALUE_TYPE_H

#include <QObject>

namespace UkuiShortcut {

class ValueType
{
    Q_GADGET
public:
    enum Type {
        Disabled = 0,
        Name,
        Icon,
        ToolTip,
        Color,
        Value,
        MaxValue,
        Menu,
        //Menu item
        MenuItemIcon,
        MenuItemText,
        MenuItemStatusIcon,
        StatusName,
        WidgetId,
        IconClickable
    };
    Q_ENUM(Type)
};

class Color
{
    Q_GADGET
public:
    /**
     * 枚举插件想要在前端界面显示的颜色
     */
    enum ColorRole
    {
        // style color
        BaseColor = 0,
        HighLight = 1,
        // status
        Success = 2,     // Success = HighLight
        Warning = 3,
        Info = 4,        // Info = BaseColor
        Danger = 5,
        Disable = 6
    };
    Q_ENUM(ColorRole)
};

} // UkuiShortcut

#endif //UKUI_SIDEBAR_VALUE_TYPE_H
