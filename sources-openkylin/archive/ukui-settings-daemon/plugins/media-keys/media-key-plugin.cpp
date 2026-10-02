/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2020 KylinSoft Co., Ltd.
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
#include "media-key-plugin.h"


PluginInterface* MediakeyPlugin::mInstance = nullptr;

MediakeyPlugin::MediakeyPlugin()
{
    if (m_pMediakeyManager == nullptr) {
        m_pMediakeyManager = new MediaKeyManager();
    }
}

MediakeyPlugin::~MediakeyPlugin()
{
    USD_LOG(LOG_ERR,"MediakeyPlugin deconstructor!");
}

PluginInterface *MediakeyPlugin::getInstance()
{
    if (nullptr == mInstance) {
        mInstance = new MediakeyPlugin();
    }
    return mInstance;
}

void MediakeyPlugin::activate()
{
    if (m_pMediakeyManager->start()) {
        USD_LOG (LOG_DEBUG, "Activating %s plugin compilation time:[%s] [%s]",MODULE_NAME,__DATE__,__TIME__);
    } else {
        USD_LOG (LOG_ERR, "error: %s plugin start failed ! time:[%s] [%s]",MODULE_NAME,__DATE__,__TIME__);
    }
}

void MediakeyPlugin::deactivate()
{
    USD_LOG(LOG_ERR, "deactivating mediakey plugin ...");
    m_pMediakeyManager->stop();
}

PluginInterface* createSettingsPlugin()
{
    return MediakeyPlugin::getInstance();
}
