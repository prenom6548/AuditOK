/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "ukui_login_sound_user_config.h"

UkuiLoginSoundUserConfig::UkuiLoginSoundUserConfig()
{
    initSettings();
    initJsonMap();
}

void UkuiLoginSoundUserConfig::initSettings()
{
    m_pAutoLoginSetting = std::make_unique<QSettings>(LIGHTDM_CONF_FILE, QSettings::IniFormat);
    m_pLastLoginSetting = std::make_unique<QSettings>(UKUI_GREETER_CONF_FILE, QSettings::IniFormat);
}

void UkuiLoginSoundUserConfig::initJsonMap()
{
    for (const auto& [k, v] : m_keys) {
        std::shared_ptr<IJson> json = nullptr;

        switch (k) {
        case JsonType::JSON_TYPE_SOUNDEFFECT: {
            json = std::make_shared<SoundEffectJson>(v());
            json->init();
            break;
        }
        case JsonType::JSON_TYPE_USERINFO: {
            json = std::make_shared<UserInfoJson>(v());
            break;
        }
        default:
            break;
        }

        if (json) {
            m_jsonMap.emplace(k, json);
        }
    }
}

const QString UkuiLoginSoundUserConfig::getAutoLoginUser() const
{
    return m_pAutoLoginSetting->value(LIGHTDM_CONF_KEY).toString();
}

const QString UkuiLoginSoundUserConfig::getLastLoginUser() const
{
    return m_pLastLoginSetting->value(UKUI_GREETER_CONF_KEY).toString();
}

std::unordered_map<JsonType, std::shared_ptr<IJson>> UkuiLoginSoundUserConfig::getJsonMap() const
{
    return m_jsonMap;
}

QString UkuiLoginSoundUserConfig::getSoundEffectFullPath()
{
    return SOUND_EFFECT_JSON;
}

QString UkuiLoginSoundUserConfig::getUserInfoFullPath()
{
    QString path = DEFAULT_PATH;
    QString autoLoginUser = getAutoLoginUser();
    QString lastLoginUser = getLastLoginUser();

    if ("" == autoLoginUser && "" == lastLoginUser) {
        path = INVAILD_PATH;
    }
    else {
        autoLoginUser == "" ? path.append(lastLoginUser) : path.append(autoLoginUser);
    }

    path == INVAILD_PATH ? path : path.append(AUDIO_JSON);
    syslog(LOG_DEBUG, "usr info file path: %s.", path.toLatin1().data());
    return  path;
}

UkuiLoginSoundUserConfig::~UkuiLoginSoundUserConfig()
{
}

