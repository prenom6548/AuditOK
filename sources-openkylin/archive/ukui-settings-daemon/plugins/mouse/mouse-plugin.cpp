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
#include "mouse-plugin.h"
#include "clib-syslog.h"
#include "usd_base_class.h"

PluginInterface * MousePlugin::mInstance = nullptr;

MousePlugin::MousePlugin()
{
    if(UsdBaseClass::isWayland()){
        //wayland
        m_waylandMouseManager = MouseWaylandManager::MouseWaylandManagerNew();
    } else {
        UsdMouseManager = MouseManager::MouseManagerNew();
    }
    USD_LOG(LOG_DEBUG,"MousePlugin initializing!");
}

MousePlugin::~MousePlugin()
{
    if (UsdMouseManager){
        delete UsdMouseManager;
        UsdMouseManager = nullptr;
    }
    if (m_waylandMouseManager) {
        delete m_waylandMouseManager;
        m_waylandMouseManager = nullptr;
    }
}

void MousePlugin::activate()
{
    bool res;
    USD_LOG (LOG_DEBUG, "Activating %s plugin compilation time:[%s] [%s]",MODULE_NAME,__DATE__,__TIME__);

    if (m_waylandMouseManager) {
        //wayland
        res = m_waylandMouseManager->start();
    }
    if (UsdMouseManager) {
        res = UsdMouseManager->start();
    }
    if(!res){
        USD_LOG(LOG_ERR,"Unable to start Mouse manager!");
    }

}

PluginInterface * MousePlugin::getInstance()
{
    if (nullptr == mInstance){
        mInstance = new MousePlugin();
    }
    return mInstance;
}

void MousePlugin::deactivate()
{
    if (m_waylandMouseManager) {
        //wayland
        m_waylandMouseManager->stop();
    }
    if (UsdMouseManager) {
        UsdMouseManager->stop();
    }
    USD_LOG(LOG_DEBUG,"Deactivating Mouse Plugin");
}

PluginInterface *createSettingsPlugin()
{
    return MousePlugin::getInstance();
}





