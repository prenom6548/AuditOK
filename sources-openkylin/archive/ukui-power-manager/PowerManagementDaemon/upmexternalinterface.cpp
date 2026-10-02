/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "upmexternalinterface.h"

UpmExternalInterface::UpmExternalInterface(QObject *parent) : QObject(parent)
{
    m_powersaveLevel = powersave_level_normal;
    m_batteryOnline = isOnBattery();
    m_powerGsettings = new QGSettings(POWER_MANAGER_SETTINGS, QByteArray(), this);
    m_currentPowerPolicy = m_powerGsettings->get(POWER_POLICY_CURRENT).toInt();
    m_percentageLowState = false;

    updatePowersaveLevel();

    QDBusConnection::systemBus().connect("org.freedesktop.UPower",
                                         "/org/freedesktop/UPower",
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this,
                                         SLOT(dealUPowerDBusMessage(QDBusMessage)));

    QDBusConnection::sessionBus().connect("org.ukui.upower",
                                          "/upower/BatteryInfo",
                                          "org.ukui.upower.battery",
                                          "LowBatteryState",
                                          this,
                                          SLOT(dealPercentageLowMessage(bool)));

    connect(m_powerGsettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == POWER_POLICY_CURRENT) {
            m_currentPowerPolicy = m_powerGsettings->get(POWER_POLICY_CURRENT).toInt();
            updatePowersaveLevel();
        }
    });
}

UpmExternalInterface::~UpmExternalInterface() {}

bool UpmExternalInterface::isOnBattery()
{
    QDBusInterface interface(UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = interface.call("Get", "org.freedesktop.UPower", "OnBattery");
    if (reply.isValid()) {
        return reply.value().toBool();
    }

    return false;
}

void UpmExternalInterface::dealUPowerDBusMessage(QDBusMessage msg)
{
    const QDBusArgument &dbusArgs = msg.arguments().at(1).value<QDBusArgument>();
    QMap<QString, QVariant> map;
    dbusArgs >> map;

    if (map.contains("OnBattery")) {
        m_batteryOnline = map.value(QString("OnBattery")).toBool();
        updatePowersaveLevel();
    }
}

void UpmExternalInterface::dealPercentageLowMessage(bool state)
{
    if (state != m_percentageLowState) {
        m_percentageLowState = state;
        updatePowersaveLevel();
    }
}

void UpmExternalInterface::updatePowersaveLevel()
{
    int powersaveLevel = powersave_level_normal;

    if (true == m_batteryOnline) {
        if (true == m_percentageLowState) {
            powersaveLevel = powersave_level_medium;
        } else {
            if (power_policy_performance != m_currentPowerPolicy) {
                powersaveLevel = powersave_level_low;
            }
        }
    }
    if (powersaveLevel != m_powersaveLevel) {
        m_powersaveLevel = powersaveLevel;
        emit powersaveLevelChanged(m_powersaveLevel);
    }
}

int UpmExternalInterface::getPowersaveLevel()
{
    return m_powersaveLevel;
}
