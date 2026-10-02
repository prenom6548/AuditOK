/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef UKUI_QUICK_WINDOW_TYPE_H
#define UKUI_QUICK_WINDOW_TYPE_H

#include <QObject>

namespace UkuiQuick {

/**
 * usage:
 * windowType: WindowType.Tooltip
 */
class WindowType
{
    Q_GADGET
public:
    // copy from [KWindowSystem: netwm_def.h, KWayland: plasmashell.h]
    // 仅保留常用的层级
    enum Type {
        Normal = 0,
        Desktop,
        Dock,
        Panel,
        SystemWindow,
        Notification,
        CriticalNotification,
        ScreenLockNotification,
        OnScreenDisplay,
        Dialog,
        ToolTip,
        Menu,
        PopupMenu,
        AppletPopup,
        Switcher,
        ScreenLock,
        WaterMark,
        InputPanel,
        Logout,
        Authentication
    };
    Q_ENUM(Type)
};

/**
 * @class Decoration
 * Window Decoration Components, only available on Wayland.
 * @see WindowProxy::setDecorationComponents
 */
class Decoration
{
    Q_GADGET
public:
    enum Component
    {
        RoundCorner = 0x1,
        Border = 0x2,
        Shadow = 0x4,
    };
    Q_DECLARE_FLAGS(Components, Component)
    Q_FLAG(Components)
};

class WindowState
{
    Q_GADGET

public:
    enum State
    {
        Minimizable = 0x1,
        Maximizable = 0x2,
        Closeable = 0x4,
        Fullscreenable = 0x8,
        Moveable = 0x10,
        Resizable = 0x20,
        Focusable = 0x40,
        Activatable = 0x80,
        KeepAbove = 0x100,
        KeepBelow = 0x200,
    };
    Q_DECLARE_FLAGS(States, State)
    Q_FLAG(State)
};



}
Q_DECLARE_METATYPE(UkuiQuick::Decoration::Components)
Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiQuick::Decoration::Components)
Q_DECLARE_METATYPE(UkuiQuick::WindowState::States)
Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiQuick::WindowState::States)
#endif //UKUI_QUICK_WINDOW_TYPE_H
