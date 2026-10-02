/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SERVICES_ASSISTANT_CTRL_H
#define SERVICES_ASSISTANT_CTRL_H

#include <string>

// 音视频session dbus
#define SESSION_DBUS_NAME_STR_FIELD "org.ukui.SettingsDaemon"
#define SESSION_DBUS_PLAYER_PATH "/mpris/controller"
#define SESSION_DBUS_PLAYER_INTERFACE "org.ukui.mpris.controller"

// 亮度session dbus
#define SESSION_DBUS_NAME_BRIGHTNESS "org.ukui.SettingsDaemon"
#define SESSION_DBUS_BRIGHTNESS_PATH "/GlobalBrightness"
#define SESSION_DBUS_BRIGHTNESS_INTERFACE "org.ukui.SettingsDaemon.Brightness"

// 日历session dbus
#define SESSION_DBUS_NAME_CALENDAR "org.ukui.calendar.Notifications"
#define SESSION_DBUS_PATH_CALENDAR "/org/ukui/calendar/Notifications"
#define SESSION_DBUS_INTERFACE_CALENDAR "org.ukui.calendar.Notifications"

const std::string SYSTEM_DBUS_BLE_SERVICE = "com.ukui.bluetooth";
const std::string SYSTEM_DBUS_BLE_PATH = "/com/ukui/bluetooth";
const std::string SYSTEM_DBUS_BLE_INTERFACE = "com.ukui.bluetooth";

class Ctrl {
public:
    Ctrl();
    bool controlMprisPlayerDbus(const std::string& funcName);
    void setBrightness(bool isUp);
    void setDefaultAdapterSwitchStatus(bool power);

private:
};

#endif