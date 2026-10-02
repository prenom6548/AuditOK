/*
* Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301, USA.
**/

#include "systemupgradecheck.h"
#include <QDebug>
#include <QDBusMessage>

#define DBUS_NAME        "com.kylin.systemupgrade"
#define DBUS_PATH        "/com/kylin/systemupgrade"
#define DBUS_INTERFACE   "com.kylin.systemupgrade.interface"

systemUpgradeCheck::systemUpgradeCheck()
{

    interface = new QDBusInterface(DBUS_NAME, DBUS_PATH, DBUS_INTERFACE, QDBusConnection::systemBus());

    if (!interface->isValid()) {
        qDebug() << "systemUpgrade interface not valid";
        isDbusActive = false;
        isSystemUpgrade =false;
    } else {
        isDbusActive = true;
    }
}

bool systemUpgradeCheck::checkSystemUpgrade(){
    if (!isDbusActive) return false;

    QDBusMessage response = interface->call("CheckInstallRequired");
    if (response.type() == QDBusMessage::ReplyMessage) {
        int res = response.arguments()[0].toInt();
        qDebug() << "CheckInstallRequired return " << res;
        if (res == 0)
            isSystemUpgrade = false;
        else
            isSystemUpgrade = true;
    } else {
        qDebug () << "call CheckInstallRequired failed;";
        isSystemUpgrade = false;
    }

    return isSystemUpgrade;
}

bool systemUpgradeCheck::doUpgradeThenRboot(){
    if (!isDbusActive) return false;

    bool  mask = false;
    QDBusMessage response = interface->call("TriggerInstallOnShutdown","reboot");
    if (response.type() == QDBusMessage::ReplyMessage) {
        int res = response.arguments()[0].toInt();
        qDebug() << "TriggerInstallOnShutdown reboot return " << res;
        if (res == 0)
            mask = true;
        else
            mask = false;
    } else {
        qDebug () << "call TriggerInstallOnShutdown reboot failed;";
        mask = false;
    }

    return mask;
}

bool systemUpgradeCheck::doUpgradeThenShutdown(){
    if (!isDbusActive) return false;

    bool  mask = false;
    QDBusMessage response = interface->call("TriggerInstallOnShutdown","shutdown");
    if (response.type() == QDBusMessage::ReplyMessage) {
        int res = response.arguments()[0].toInt();
        qDebug() << "TriggerInstallOnShutdown shutdown return " << res;
        if (res == 0)
            mask = true;
        else
            mask = false;
    } else {
        qDebug () << "call TriggerInstallOnShutdown shutdown failed;";
        mask = false;
    }

    return mask;
}
