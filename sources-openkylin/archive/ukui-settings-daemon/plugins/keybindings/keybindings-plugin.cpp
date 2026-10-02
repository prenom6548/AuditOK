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
#include "keybindings-plugin.h"
#include "clib-syslog.h"
#include "usd_base_class.h"

PluginInterface    *KeybindingsPlugin::mInstance=nullptr;

KeybindingsPlugin::KeybindingsPlugin()
{
    USD_LOG(LOG_DEBUG,"KeybindingsPlugin initializing");
    if (true) {
        //wayland
        m_waylandKeyManager = KeybindingsWaylandManager::KeybindingsWaylandManagerNew();
    } else {
        m_xKeyManager = KeybindingsManager::KeybindingsManagerNew();
    }
}

KeybindingsPlugin::~KeybindingsPlugin()
{
    USD_LOG(LOG_DEBUG,"KeybindingsPlugin free");
    if(m_xKeyManager){
        delete m_xKeyManager;
        m_xKeyManager = nullptr;
    }

    if (m_waylandKeyManager) {
        delete m_waylandKeyManager;
        m_waylandKeyManager = nullptr;
    }
}

void KeybindingsPlugin::activate()
{
    bool res;
    USD_LOG (LOG_DEBUG, "Activating %s plugin compilation time:[%s] [%s]",MODULE_NAME,__DATE__,__TIME__);
    if (m_xKeyManager) {
        res = m_xKeyManager->start();
    }
    if (m_waylandKeyManager) {
        m_waylandKeyManager->start();
    }
    if(!res)
        USD_LOG(LOG_ERR,"Unable to start Keybindings manager");
}

PluginInterface *KeybindingsPlugin::getInstance()
{
    if(nullptr == mInstance)
        mInstance = new KeybindingsPlugin();
    return mInstance;
}

void KeybindingsPlugin::deactivate()
{
    if (m_xKeyManager) {
        m_xKeyManager->stop();
    }
    if (m_waylandKeyManager) {
        m_waylandKeyManager->stop();
    }
    USD_LOG(LOG_DEBUG,"Dectivating Keybindings Plugin");
}

PluginInterface *createSettingsPlugin()
{
    return KeybindingsPlugin::getInstance();
}
