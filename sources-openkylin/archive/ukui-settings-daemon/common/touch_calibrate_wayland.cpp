/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2025 Tianjin KYLIN Information Technology Co., Ltd.
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

#include <KF5/KScreen/kscreen/config.h>
#include <KF5/KScreen/kscreen/log.h>
#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/edid.h>
#include <KF5/KScreen/kscreen/configmonitor.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/setconfigoperation.h>

#include "touch_calibrate_wayland.h"


TouchCalibrateWayland::TouchCalibrateWayland()
{

}

void TouchCalibrateWayland::getScreenList()
{
    auto *op = new KScreen::GetConfigOperation();
    if (!op->exec()) {
        SYS_LOG(LOG_DEBUG,"get error:%s",qPrintable(op->errorString()));
        return;
    }

    for (auto kscreen: op->config()->outputs()) {
        ScreenInfoPtr screen(new ScreenInfo);
        screen->name = kscreen->name();
        screen->width = kscreen->sizeMm().width();
        screen->height = kscreen->sizeMm().height();
        m_screenInfoMap.insert(screen->name, screen);
        SYS_LOG(LOG_DEBUG, "%s  width : %d height : %d",
                screen->name.toLatin1().data(), screen->width, screen->height);

    }
    USD_LOG_SHOW_PARAM1(m_screenInfoMap.count());
}

void TouchCalibrateWayland::getTouchDeviceList()
{
    QDBusInterface deviceInterface(QStringLiteral(KDE_KWIN_SERVICE),
                                                              QStringLiteral(KDE_KWIN_OBJECT),
                                                              QStringLiteral(KDE_KWIN_DEVICE_MANAGER),
                                                              QDBusConnection::sessionBus());

    if (deviceInterface.isValid()) {
        QStringList deviceList = deviceInterface.property("devicesSysNames").toStringList();
        for (QString device : deviceList) {
            addTouchDeviceToList(device);
        }
    }

}

void TouchCalibrateWayland::addTouchDeviceToList(QString device)
{
    QDBusInterface *interface = new QDBusInterface(KDE_KWIN_SERVICE,
                                     KDE_KWIN_OBJECT_PATH + device,
                                     KDE_KWIN_INTERFACE,
                                     QDBusConnection::sessionBus());

    if (!interface->isValid()) {
        delete interface;
        interface = nullptr;
        return;
    }
    //touch:触摸屏、touchpad：触摸板，tabletTool：触摸笔
    if (interface->property("touch").toBool() ||
            interface->property("tabletTool").toBool()){
        TouchDevicePtr dev = TouchDevicePtr(new TouchDevice);
        dev->name = interface->property("name").toString();
        dev->calibrateName = dev->name;
        QString devicePath = QString("/dev/input/%1").arg(interface->property("sysName").toString());
        getTouchSize(qPrintable(devicePath)  , dev->width, dev->height);

        if (interface->property("tabletTool").toBool()) {
            m_tabletList.append(dev);
        } else {
            m_touchScreenList.append(dev);
        }
    }
}

void TouchCalibrateWayland::calibrateTouchScreen()
{
    //首先配置文件映射
    Q_FOREACH (const TouchConfigPtr& touchConfig, m_touchConfigList) {//配置文件
        Q_FOREACH (const TouchDevicePtr& touch , m_touchScreenList) {//所有设备
            if (touch->name == touchConfig->sTouchName) {
                const ScreenInfoPtr& screen = m_screenInfoMap.value(touchConfig->sMonitorName);
                if (screen.data()) {
                    calibrateDevice(touch->name, screen->name);
                    touch->isMapped = true;
                    screen->isMapped = true;
                }
            }
        }
    }
    autoMaticMapping(m_touchScreenList, m_screenInfoMap);
}


void TouchCalibrateWayland::calibrateDevice(QVariant deviceName, const QString &output)
{
    QDBusInterface wlcomInputInterface(C_WLCOM_SERVICE,
                                       C_WLCOM_INPUT_PATH,
                                       C_WLCOM_INPUT_INTERFACE,
                                       QDBusConnection::sessionBus());

    QList<QVariant> args;
    args << deviceName.toString();
    args << output;
    wlcomInputInterface.asyncCallWithArgumentList("MapToOutput", args);
    USD_LOG(LOG_DEBUG,"map :%s to %s",qPrintable(deviceName.toString()), qPrintable(output));
}

