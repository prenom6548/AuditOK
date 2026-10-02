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
#include "media-key-settings.h"


#define MEDIAKEY_SCHEMA         "org.ukui.SettingsDaemon.plugins.media-keys"

#define GSETTINGS_INIT_RESULT "gsettings-init-result"   //gsettings初始化结果

MediaKeySettings::MediaKeySettings(QString schema, QObject *parent):
    MediaKeyAbstractSettings(schema, parent)
{
    setGsettings(schema);
    initSettings();
}

MediaKeySettings::~MediaKeySettings()
{
    clearMapData();
}

MediaKeySettings *MediaKeySettings::instance(QString schema)
{
    static MediaKeySettings MediaKeySettings(schema);
    return &MediaKeySettings;
}

void MediaKeySettings::initSettings()
{
    clearMapData();

    for (const QString& key : getGsettings()->keys()) {
        insertKeysData(key, getGsettings()->get(key));
    }

    connect(getGsettings().data(), SIGNAL(changed(const QString&)), this, SLOT(onKeyChanged(const QString&)), Qt::DirectConnection);

}
