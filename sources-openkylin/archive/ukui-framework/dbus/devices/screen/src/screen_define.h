/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#ifndef __SCREEN_DEFINE_H__
#define __SCREEN_DEFINE_H__
#include <QObject>
#include <QDBusArgument>
#include "gsettings_manager.h"

#define UKUI_SETTINGS_DAEMON_COLOR_SCHEMA   "org.ukui.SettingsDaemon.plugins.color"
#define UKUI_SETTINGS_DAEMON_COLOR_MAIN_SWITCH  "main-switch"
#define UKUI_SETTINGS_DAEMON_COLOR_EYS_CARE "eye-care"
#define UKUI_SETTINGS_DAEMON_COLOR_NIGHT_MODE   "night-light-enabled"

#define UKUI_SETTINGS_DAEMON_AUTO_BRIGHTNESS           "org.ukui.SettingsDaemon.plugins.auto-brightness"
#define UKUI_SETTINGS_AUTO_BRIGHTNESS_HAD_SENSOR    "had-sensor"
#define UKUI_SETTINGS_AUTO_BRIGHTNESS_ENABLE        "auto-brightness"

#define MY_NAME  "ukui-framework-screen"
#define UKUI_POWER_MANAGER_DBUS_NAME         "org.ukui.powermanagement"
#define UKUI_POWER_MANAGER_DBUS_PATH         "/"
#define UKUI_POWER_MANAGER_DBUS_INTERFACE    "org.ukui.powermanagement.interface"

#define UKUI_POWER_MANAGER_SCHEMA           "org.ukui.power-manager"
#define UKUI_POWER_MANAGER_SCHEMA_BRIGHT    "brightness-ac"

struct OutputGammaInfo{
    QString outputName;
    uint    gamma;
    uint    brignthess;
    uint    temperature;
};

typedef QList<OutputGammaInfo> OutputGammaInfoList;

Q_DECLARE_METATYPE(OutputGammaInfo)
Q_DECLARE_METATYPE(OutputGammaInfoList)

inline QDBusArgument &operator << (QDBusArgument &argument, const OutputGammaInfo &outputInfo)
{
    argument.beginStructure();
    argument << outputInfo.outputName;
    argument << outputInfo.gamma;
    argument << outputInfo.brignthess;
    argument << outputInfo.temperature;
    argument.endStructure();

    return argument;
}

inline const QDBusArgument &operator >> (const QDBusArgument &argument, OutputGammaInfo &outputInfo) {
    argument.beginStructure();
    argument >> outputInfo.outputName;
    argument >> outputInfo.gamma;
    argument >> outputInfo.brignthess;
    argument >> outputInfo.temperature;
    argument.endStructure();
    return argument;
}

#endif
