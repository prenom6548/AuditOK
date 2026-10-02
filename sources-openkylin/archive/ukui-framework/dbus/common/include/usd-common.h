/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef USDCOMMON_H
#define USDCOMMON_H

#define USD_SYSBUS_SERVER               "com.kylin.ukui.SettingsDaemon"
#define USD_SYSBUS_PATH                 "/devicemanager"
#define USD_SYSBUS_INTERFACE            "com.kylin.ukui.SettingsDaemon.interface"
#define USD_SIGNAL_DEVICENUMCHANGED     "DeviceNumChanged"

#define USD_SESSION_SERVER                  "org.ukui.SettingsDaemon"
#define USD_SESSION_PATH                    "/org/ukui/SettingsDaemon/InputDeviceManager"
#define USD_SESSION_INTERFACE               "org.ukui.SettingsDaemon.InputDeviceManager"
#define USD_METHOD_GETDEVCOUNTBYTYPE        "GetDevCountByType"
#define USD_SIGNAL_MOUSECOUNTCHANGED        "MouseCountChanged"
#define USD_SIGNAL_KBDCOUNTCHANGED          "KbdCountChanged"
#define USD_SIGNAL_TOUCHPADCOUNTCHANGED     "TouchpadCountChanged"

#define ORG_UKUI_FRAMEWORK_SCREEN_INTERFACE "org.ukui.Framework.Devices.Screen"

#define DBUS_XRANDR_NAME                    "org.ukui.SettingsDaemon"
#define DBUS_GC_BRIGHTNESS_PATH             "/GlobalBrightness"
#define DBUS_GC_BRIGHTNESS_INTERFACE        "org.ukui.SettingsDaemon.Brightness"

#define DBUS_GM_NAME                    "org.ukui.SettingsDaemon"
#define DBUS_GM_PATH                    "/org/ukui/SettingsDaemon/GammaManager"
#define DBUS_GM_INTERFACE               "org.ukui.SettingsDaemon.GammaManager"
#define DBUS_GM_SCREENCHANGED           "screenBrightnessChanged"

namespace  UKUI{
#define DEFINE_PROPERTY(type, name) Q_PROPERTY(type name READ Get##name WRITE Set##name NOTIFY name##Changed)
};

#endif // USDCOMMON_H

