/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef MEDIATYPE_H
#define MEDIATYPE_H
#include <QObject>
#include "usd_global_define.h"

#define MEDIAKEY_SCHEMA_FILTER         "org.ukui.SettingsDaemon.plugins.media-keys."
#define BINDING_SCHEMA "org.ukui.SettingsDaemon.plugins.media-keys"
#define UKUI_DAEMON_NAME    "ukui-settings-daemon"
#include "usd_global_define.h"
//action type :快捷键对应功能
typedef enum{
    ACTION_ENUM_TYPE
} ActionType;

class UsdEnumClass : public QObject {
    Q_OBJECT

public:
    enum eActionType{
        ACTION_ENUM_TYPE
    };

    Q_ENUM(eActionType)
};


typedef enum {
    POWER_SUSPEND = 1,
    POWER_SHUTDOWN,
    POWER_HIBERNATE,
    POWER_INTER_ACTIVE,
}PowerType;

#endif // MEDIATYPE_H
