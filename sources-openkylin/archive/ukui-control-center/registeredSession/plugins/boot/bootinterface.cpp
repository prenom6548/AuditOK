/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "bootinterface.h"
#include <QGSettings/QGSettings>
#include <kysdk/kysdk-system/libkysysinfo.h>

BootInterface::BootInterface():UKUI::ServiceObject(nullptr), QDBusContext()
{
    QDBusInterface m_pUkccSystemDbus("com.control.center.qt.systemdbus",
                                      "/",
                                      "com.control.center.interface",
                                      QDBusConnection::systemBus(), this);

    int grubSystems = gotGrubSystems();
    QDBusReply<int> actualSystems = m_pUkccSystemDbus.call("getActualSystem");

    qDebug() << "grubSystems = " << grubSystems << "actualSystems = " << actualSystems;

    if (grubSystems != actualSystems) {
        QGSettings ukcc(UKCC_SCHAME);
        // 获取到当前系统不一致，设置gsetting提供给前端取消勾选
        ukcc.set(SYSTEMS_SAME, false);

        QString lang = qgetenv("LANG");
        m_pUkccSystemDbus.call("updateGrub", lang);
    }
}

BootInterface::~BootInterface()
{
}

int BootInterface::gotGrubSystems()
{
    // 1.获取当前/boot/grub/grub.cfg文件中启动项
    QString grubMenus(kdk_system_get_grub_menu());
    grubMenus.remove("\"");
    grubMenus.remove("{");
    grubMenus.remove("}");

    qDebug() << "get boot list = " << grubMenus;
    QStringList grubMenuItem =  grubMenus.split(",");
    int gotSystems = 0;
    // 2.windows 去掉列表中windows
    for (int i = 0; i < grubMenuItem.count(); i++) {
        QString tmp = grubMenuItem[i];
        QStringList item = tmp.split(":");
        if (item.size() >= 2) {
            qDebug() << "grub menu type :" << item.at(1);
            if (item.at(1) == "menuentry") {
                qDebug() << "grub menu :" << item.at(0);
                if (!QString(item.at(0)).contains("windows", Qt::CaseInsensitive)) {
                    gotSystems++;
                }
            }
        }
    }

    return gotSystems;
}
