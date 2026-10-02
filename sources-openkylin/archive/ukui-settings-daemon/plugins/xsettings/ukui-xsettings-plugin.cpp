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
#include <QDebug>
#include "ukui-xsettings-plugin.h"
#include "clib-syslog.h"
#include "usd_base_class.h"

PluginInterface *XSettingsPlugin::mInstance =nullptr;
AbstractManager *XSettingsPlugin::m_pukuiSettingManager = nullptr;

XSettingsPlugin::XSettingsPlugin()
{
    if(nullptr == m_pukuiSettingManager) {
        m_pukuiSettingManager = new ukuiXSettingsManager();
        p_waylandXSettings = new ukuiWaylandSettingsManager();
    }
}

XSettingsPlugin::~XSettingsPlugin()
{
    if (m_pukuiSettingManager) {
        delete m_pukuiSettingManager;
        m_pukuiSettingManager = nullptr;
    }

    if (p_waylandXSettings) {
        delete p_waylandXSettings;
        p_waylandXSettings = nullptr;
    }
}

void XSettingsPlugin::activate()
{
    bool res;
    if (m_pukuiSettingManager) {
        res = m_pukuiSettingManager->start();
        p_waylandXSettings->start();
        if (!res) {
            USD_LOG (LOG_DEBUG, "Unable to start XSettingsPlugin manager");
        }
        USD_LOG (LOG_DEBUG, "Activating %s plugin compilation time:[%s] [%s]",MODULE_NAME,__DATE__,__TIME__);
    } else {
        USD_LOG (LOG_DEBUG, "Unactivating %s plugin compilation time:[%s] [%s]",MODULE_NAME,__DATE__,__TIME__);
    }
}

void XSettingsPlugin::deactivate()
{
    if (m_pukuiSettingManager) {
        m_pukuiSettingManager->stop();
        delete m_pukuiSettingManager;
        m_pukuiSettingManager = nullptr;
    }
}

PluginInterface *XSettingsPlugin::getInstance()
{
    if(nullptr == mInstance)
        mInstance = new XSettingsPlugin();
    return mInstance;
}

PluginInterface* createSettingsPlugin() {
    return XSettingsPlugin::getInstance();
}
