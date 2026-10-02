/*
* Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef WINDOW_HELPER_COMMON_H
#define WINDOW_HELPER_COMMON_H
#include <QMap>
#include "window-type.h"
#include "window-helper.h"
namespace UkuiQuick {
static QMap<WindowType::Type, uint> ukui_surface_roleMap = {
    {WindowType::Normal, 0},
    {WindowType::Desktop, 1},
    {WindowType::Dock, 2},
    {WindowType::Panel, 2},
    {WindowType::OnScreenDisplay, 3},
    {WindowType::Notification, 4},
    {WindowType::ToolTip, 5},
    {WindowType::CriticalNotification, 6},
    {WindowType::AppletPopup, 7},
    {WindowType::ScreenLock, 8},
    {WindowType::WaterMark, 9},
    {WindowType::SystemWindow,10},
    {WindowType::InputPanel,11},
    {WindowType::Logout,12},
    {WindowType::ScreenLockNotification,13},
    {WindowType::Switcher,14},
    {WindowType::Authentication, 15}
};

static QMap<WindowProxy::SlideFromEdge, int> ukui_surface_slideEdge = {
    {WindowProxy::NoEdge, -1},
    {WindowProxy::TopEdge, 1},
    {WindowProxy::BottomEdge, 3},
    {WindowProxy::LeftEdge, 0},
    {WindowProxy::RightEdge, 2}
};
}
#endif //WINDOW_HELPER_COMMON_H
