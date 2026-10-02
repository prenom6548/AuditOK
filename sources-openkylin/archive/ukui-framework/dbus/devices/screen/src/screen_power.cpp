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

#include "screen_power.h"
#include "common.h"
#include "usd-common.h"

ScreenBrightNode::ScreenBrightNode(QObject *parent)
{
    qDebug()<<__FUNCTION__ << "init upm brightness";
    m_pPowerGsettingsManager = new GsettingsManager(this,
                                                    UKUI_POWER_MANAGER_SCHEMA,
                                                    PowerChangedHandler());
}

ScreenBrightNode::~ScreenBrightNode()
{

}

AbstractResult ScreenBrightNode::setScreenBrightness(QString name, uint screenBrightness)
{
    AbstractResult ret;
    return ret;
}

AbstractResult ScreenBrightNode::getScreenBrightness(QString name)
{
    AbstractResult ret;
    return ret;
}

AbstractResult ScreenBrightNode::setScreensBrightness(OutputGammaInfoList outputsInfo)
{
    AbstractResult ret;
     return ret;
}

AbstractResult ScreenBrightNode::setAllScreenSameBrightness(uint brighenss)
{
    AbstractResult ret;
    return ret;
}


OutputGammaInfoList ScreenBrightNode::getAllScreenBrightness()
{
    OutputGammaInfoList ret;
    return ret;
}

uint ScreenBrightNode::getPrimaryScreenBrightness()
{
    return m_pPowerGsettingsManager->get(UKUI_POWER_MANAGER_SCHEMA_BRIGHT).toInt();
}
#define _UKUI_FRAMEWORK_DEBUG
QHash<QString, OnKeyChanged> ScreenBrightNode::PowerChangedHandler()
{
     m_powerSettingsChangedHandlers[UKUI_POWER_MANAGER_SCHEMA_BRIGHT] = [this]{
#ifdef  _UKUI_FRAMEWORK_DEBUG
        qDebug()<<__FUNCTION__<<__LINE__<<UKUI_POWER_MANAGER_SCHEMA_BRIGHT<<getPrimaryScreenBrightness();
#endif
        QDBusMessage notifySignal =
                QDBusMessage::createSignal(DEVICES_SCREEN_PATH, ORG_UKUI_FRAMEWORK_SCREEN_INTERFACE, "PrimaryScreenBrightnessChanged");
        notifySignal.setArguments({QVariant::fromValue((uint)getPrimaryScreenBrightness())});
        QDBusConnection::sessionBus().send(notifySignal);
    };

    return m_powerSettingsChangedHandlers;
}
