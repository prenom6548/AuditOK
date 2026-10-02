/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "systemupgradehelper.h"
#include <QDebug>
#include <QDBusMessage>

const static QString sysUpgradeService = QStringLiteral("com.kylin.systemupgrade");
const static QString sysUpgradePath = QStringLiteral("/com/kylin/systemupgrade");
const static QString sysUpgradeInterface = QStringLiteral("com.kylin.systemupgrade.interface");

SystemUpgradeHelper::SystemUpgradeHelper(QObject *parent) : QObject(parent)
{
    m_sysUpgradeInterface
        = new QDBusInterface(sysUpgradeService, sysUpgradePath, sysUpgradeInterface, QDBusConnection::systemBus());

    if (!m_sysUpgradeInterface->isValid()) {
        qDebug() << "systemUpgrade interface not valid";
        isDbusActive = false;
        isSystemUpgrade = false;
    } else {
        isDbusActive = true;
    }
}

SystemUpgradeHelper::~SystemUpgradeHelper() {}

bool SystemUpgradeHelper::checkSystemUpgrade()
{
    if (!isDbusActive)
        return false;

    QDBusMessage response = m_sysUpgradeInterface->call("CheckInstallRequired");
    if (response.type() == QDBusMessage::ReplyMessage) {
        int res = response.arguments()[0].toInt();
        qDebug() << "CheckInstallRequired return " << res;
        if (res == 0)
            isSystemUpgrade = false;
        else
            isSystemUpgrade = true;
    } else {
        qDebug() << "call CheckInstallRequired failed;";
        isSystemUpgrade = false;
    }
    return isSystemUpgrade;
}

bool SystemUpgradeHelper::doUpgradeThenRboot()
{
    if (!isDbusActive)
        return false;

    bool mask = false;
    QDBusMessage response = m_sysUpgradeInterface->call("TriggerInstallOnShutdown", "reboot");
    if (response.type() == QDBusMessage::ReplyMessage) {
        int res = response.arguments()[0].toInt();
        qDebug() << "TriggerInstallOnShutdown reboot return " << res;
        if (res == 0)
            mask = true;
        else
            mask = false;
    } else {
        qDebug() << "call TriggerInstallOnShutdown reboot failed;";
        mask = false;
    }

    return mask;
}

bool SystemUpgradeHelper::doUpgradeThenShutdown()
{
    if (!isDbusActive)
        return false;

    bool mask = false;
    QDBusMessage response = m_sysUpgradeInterface->call("TriggerInstallOnShutdown", "shutdown");
    if (response.type() == QDBusMessage::ReplyMessage) {
        int res = response.arguments()[0].toInt();
        qDebug() << "TriggerInstallOnShutdown shutdown return " << res;
        if (res == 0)
            mask = true;
        else
            mask = false;
    } else {
        qDebug() << "call TriggerInstallOnShutdown shutdown failed;";
        mask = false;
    }

    return mask;
}
