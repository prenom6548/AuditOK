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
#include <QDBusConnection>
#include <QDBusInterface>

#include "gamma-manager-dbus.h"
//#include "gamma-manager.h"
#include "x-manager.h"
GmDbus::GmDbus(QObject* parent): QObject(parent)
{
    QDBusConnection::sessionBus().registerObject("0", this, QDBusConnection::ExportAllSlots);
}

GmDbus::~GmDbus()
{

}

int GmDbus::setScreenBrightness(QString appName, QString screenName, uint screenBrightness)
{
    if (screenBrightness > 100 || screenBrightness < 0) {
        USD_LOG(LOG_DEBUG, "app %s set bad value(%d) to %s", appName.toLatin1().data(), screenBrightness, screenName.toLatin1().data());
        return -1;
    }

    xManager *pGmManager = static_cast<xManager*>(this->parent());
    pGmManager->doScreenBrightnessChanged(screenName, screenBrightness);
    return screenBrightness;
}

int GmDbus::setAllScreenBrightness(QString appName, int screenBrightness)
{
    if (screenBrightness > 100 || screenBrightness < 0) {
        return -1;
    }

    xManager *pGmManager = static_cast<xManager*>(this->parent());
    pGmManager->doScreenBrightnessChanged("all", screenBrightness);

    return screenBrightness;
}

int GmDbus::setPrimaryBrightness(QString appName, int screenBrightness)
{
    if (screenBrightness > 100 || screenBrightness < 0) {
        return -1;
    }

    xManager *pGmManager = static_cast<xManager*>(this->parent());
    pGmManager->setPrimaryBrightness(screenBrightness);

    return screenBrightness;
}

int GmDbus::getPrimaryBrightness()
{
    xManager *pGmManager =  static_cast<xManager*>(this->parent());
    return pGmManager->getPrimaryBrightness();
}

OutputGammaInfo GmDbus::getScreensGamma(QString appName)
{
    xManager *pGmManager =  static_cast<xManager*>(this->parent());
    return pGmManager->getScreensInfo();
}

OutputGammaInfoList GmDbus::getScreensGammaList(QString appName)
{
    xManager *pGmManager = static_cast<xManager*>(this->parent());
    return pGmManager->getScreensInfoList();
}


QHash<QString, QVariant> GmDbus::getScreensGammaInfo(QString appName)
{
    return QHash<QString, QVariant> {

           { QStringLiteral("ActiveEnabled"), true},


           { QStringLiteral("ModeEnabled"), true}
    };
}

int GmDbus::setColorTemperature(QString appName, int colorTemp)
{
    if (colorTemp > 8000 || colorTemp < 1100) {
        USD_LOG(LOG_DEBUG, "app %s set bad value(%d)", appName.toLatin1().data(), colorTemp);
        return -1;
    }
    xManager *pGmManager =  static_cast<xManager*>(this->parent());
    return pGmManager->setTemperature(colorTemp);
}

int GmDbus::enablePrimarySignal(int state)
{
    xManager *pGmManager = static_cast<xManager*>(this->parent());
    return pGmManager->enablePrimarySignal(state);
}

void  GmDbus::restoreColorTemperature()
{
    xManager *pGmManager = static_cast<xManager*>(this->parent());
    return pGmManager->gammaReckeck("");
}
