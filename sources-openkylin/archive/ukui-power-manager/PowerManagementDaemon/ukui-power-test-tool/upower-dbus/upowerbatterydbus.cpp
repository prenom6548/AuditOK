/*
 * Copyright 2023 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "upowerbatterydbus.h"

UpowerBatteryDBus::UpowerBatteryDBus(int batteryId, const QString &batteryDBusPath, QObject *parent)
    : QObject(parent), m_batteryId(batteryId), m_batteryDBusPath(batteryDBusPath)
{
    m_DBusInterface = new QDBusInterface("org.freedesktop.UPower",
                                         m_batteryDBusPath,
                                         "org.freedesktop.DBus.Properties",
                                         QDBusConnection::systemBus(),
                                         this);

    QDBusConnection::systemBus().connect("org.freedesktop.UPower",
                                         m_batteryDBusPath,
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this,
                                         SLOT(dealBatteryPropertiesMessage(QDBusMessage)));

    initBatteryInfo();
}

UpowerBatteryDBus::~UpowerBatteryDBus() {}

void UpowerBatteryDBus::dealBatteryPropertiesMessage(QDBusMessage msg)
{
    const QDBusArgument &dbusArgs = msg.arguments().at(1).value<QDBusArgument>();
    QMap<QString, QVariant> map;
    dbusArgs >> map;

    QStringList batteryInfoChangedList;

    if (map.contains("IsPresent")) {
        m_batteryPresent = map.value(QString("IsPresent")).toBool();
        batteryInfoChangedList.append("IsPresent");
    }

    if (map.contains("Percentage")) {
        m_percentage = map.value(QString("Percentage")).toDouble();
        batteryInfoChangedList.append("Percentage");
    }

    if (map.contains("Energy")) {
        m_energyNow = map.value(QString("Energy")).toDouble();
        batteryInfoChangedList.append("Energy");
    }

    if (0 != batteryInfoChangedList.size()) {
        emit batteryInfoChanged(m_batteryId, batteryInfoChangedList);
    }
}

void UpowerBatteryDBus::initBatteryInfo()
{
    QDBusMessage msg = m_DBusInterface->call("GetAll", "org.freedesktop.UPower.Device");

    if (msg.type() == QDBusMessage::ReplyMessage) {
        const QDBusArgument &dbusArgs = msg.arguments().at(0).value<QDBusArgument>();
        QMap<QString, QVariant> map;
        dbusArgs >> map;

        m_batteryPresent = map.value(QString("IsPresent")).toBool();
        m_percentage = map.value(QString("Percentage")).toDouble();
        m_energyNow = map.value(QString("Energy")).toDouble();
        m_energyFull = map.value(QString("EnergyFull")).toDouble();
        m_energyFullDesign = map.value(QString("EnergyFullDesign")).toDouble();
    }
}

bool UpowerBatteryDBus::batteryIsPresent()
{
    return m_batteryPresent;
}

double UpowerBatteryDBus::getBatteryPercentage()
{
    return m_percentage;
}

double UpowerBatteryDBus::getEnergyNow()
{
    return m_energyNow;
}

double UpowerBatteryDBus::getEnergyFull()
{
    return m_energyFull;
}

double UpowerBatteryDBus::getEnergyFullDesign()
{
    return m_energyFullDesign;
}

void UpowerBatteryDBus::refreshBatteryInfo()
{
    QDBusInterface dBusInterface("org.freedesktop.UPower",
                                 m_batteryDBusPath,
                                 "org.freedesktop.UPower.Device",
                                 QDBusConnection::systemBus());
    dBusInterface.call("Refresh");
}
