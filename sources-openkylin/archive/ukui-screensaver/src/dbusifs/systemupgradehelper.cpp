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

SystemUpgradeHelper::SystemUpgradeHelper(QObject *parent) : QObject(parent) {}

SystemUpgradeHelper::~SystemUpgradeHelper() {}

void SystemUpgradeHelper::init()
{
    if (!m_sysUpgradeInterface) {
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
}

QPair<bool, QString> SystemUpgradeHelper::checkSystemUpgrade()
{
    init();

    QString upgradeTime = QStringLiteral("NoUpdate");
    isSystemUpgrade = false;

    if (!isDbusActive)
        return qMakePair(isSystemUpgrade, upgradeTime);

    QDBusMessage response = m_sysUpgradeInterface->call("CheckInstallRequired");
    if (response.type() == QDBusMessage::ReplyMessage) {
        if (response.arguments().size() == 1 || response.arguments().size() == 2) {
            int res = response.arguments()[0].toInt();
            qDebug() << "CheckInstallRequired return int" << res;
            if (res == 1 || res == 2) {
                isSystemUpgrade = true;
                if (response.arguments().size() == 2) {
                    const QVariant upgradeVariant = response.arguments()[1];
                    qDebug() << "upgradeTime" << upgradeVariant;
                    bool ok = false;
                    const int intValue = upgradeVariant.toInt(&ok);
                    if (ok && intValue > 0) {
                        upgradeTime = upgradeVariant.toString();
                    } else {
                        upgradeTime = QStringLiteral("NoUpdate");
                        qDebug() << "Failed to get the upgrade time";
                    }
                } else {
                    upgradeTime = QStringLiteral("NoUpdate");
                    qDebug() << "Failed to get the upgrade time";
                }
            } else {
                upgradeTime = QStringLiteral("NoUpdate");
                isSystemUpgrade = false;
                qDebug() << "Do not need to update";
            }
        } else {
            upgradeTime = QStringLiteral("NoUpdate");
            isSystemUpgrade = false;
            qDebug() << "CheckInstallRequired response arguments is null or too much arguments";
        }
    } else {
        upgradeTime = QStringLiteral("NoUpdate");
        isSystemUpgrade = false;
        qDebug() << "call CheckInstallRequired failed";
    }

    qDebug() << "isSystemUpgrade" << isSystemUpgrade << "upgradeTime" << upgradeTime;
    return qMakePair(isSystemUpgrade, upgradeTime);
}

bool SystemUpgradeHelper::doUpgradeThenRboot()
{
    init();
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
    init();
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

bool SystemUpgradeHelper::checkHideUpdate()
{
    if (!isDbusActive)
        return false;

    bool mask = false;
    QDBusMessage response = m_sysUpgradeInterface->call("CheckHideUpdate");
    if (response.type() == QDBusMessage::ReplyMessage) {
        bool res = response.arguments()[0].toBool();
        qDebug() << "CheckHideUpdate return " << res;
        if (res)
            mask = true;
        else
            mask = false;
    } else {
        qDebug() << "call CheckHideUpdate failed;";
        if (response.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "Dbus error name:" << response.errorName() << "-- Message:" << response.errorMessage();
        }
        mask = false;
    }

    return mask;
}
