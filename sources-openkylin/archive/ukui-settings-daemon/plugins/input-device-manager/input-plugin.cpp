/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2022 KylinSoft Co., Ltd.
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
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */
#include "input-plugin.h"
extern "C" {
#include "clib-syslog.h"
}


InputPlugin::InputPlugin()
{
    if (!m_inputDeviceManager) {
        m_inputDeviceManager = new InputDeviceManager;
    }
}
InputPlugin::~InputPlugin()
{
    if (m_inputDeviceManager) {
        delete m_inputDeviceManager;
        m_inputDeviceManager = nullptr;
    }
}

PluginInterface *InputPlugin::getInstance()
{
    static InputPlugin inputPlugin;
    return &inputPlugin;
}

void InputPlugin::activate()
{
   if (m_inputDeviceManager) {
       USD_LOG(LOG_DEBUG,"input device manager is start.");
       m_inputDeviceManager->start();
   }
}

void InputPlugin::deactivate()
{
    if (m_inputDeviceManager) {
        USD_LOG(LOG_DEBUG,"input device manager is stop.");
        m_inputDeviceManager->stop();
    }
}

PluginInterface* createSettingsPlugin()
{
    return InputPlugin::getInstance();
}
