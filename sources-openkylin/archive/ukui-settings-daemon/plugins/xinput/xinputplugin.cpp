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

#include "xinputplugin.h"

#include <QMutex>
#include <syslog.h>
#include "usd_base_class.h"

XinputPlugin* XinputPlugin::_instance = nullptr;

XinputPlugin* XinputPlugin::instance()
{
    QMutex mutex;
    mutex.lock();
    if(nullptr == _instance)
        _instance = new XinputPlugin;
    mutex.unlock();
    return _instance;
}

XinputPlugin::XinputPlugin()
{
    if (UsdBaseClass::isWayland()) {
        USD_LOG(LOG_DEBUG,"is wayland exit...");
        return;
    }
    if (!m_pXinputManager) {
        m_pXinputManager = new XinputManager(nullptr);
    }

    USD_LOG(LOG_ERR, "Loading Xinput plugins");
}

XinputPlugin::~XinputPlugin()
{

}

void XinputPlugin::activate()
{
    if (UsdBaseClass::isWayland()) {
        USD_LOG(LOG_DEBUG,"is wayland exit...");
        return;
    }
    USD_LOG(LOG_ERR,"activating Xinput plugins");
    m_pXinputManager->start();
}

void XinputPlugin::deactivate()
{
    if (UsdBaseClass::isWayland()) {
        USD_LOG(LOG_DEBUG,"is wayland exit...");
        return;
    }
    m_pXinputManager->stop();
}

PluginInterface *createSettingsPlugin()
{
    return dynamic_cast<PluginInterface*>(XinputPlugin::instance());
}

