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
    if (map.contains("State")) {
        m_batteryState = map.value(QString("State")).toInt();
        batteryInfoChangedList.append("State");
    }
    if (map.contains("Percentage")) {
        m_percentage = map.value(QString("Percentage")).toDouble();
        batteryInfoChangedList.append("Percentage");
    }
    if (map.contains("TimeToFull")) {
        m_timeToFull = map.value(QString("TimeToFull")).toLongLong();
        batteryInfoChangedList.append("TimeToFull");
    }
    if (map.contains("TimeToEmpty")) {
        m_timeToEmpty = map.value(QString("TimeToEmpty")).toLongLong();
        batteryInfoChangedList.append("TimeToEmpty");
    }

    if (0 != batteryInfoChangedList.size()) {
        Q_EMIT batteryInfoChanged(m_batteryId, batteryInfoChangedList);
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
        m_batteryState = map.value(QString("State")).toInt();
        m_percentage = map.value(QString("Percentage")).toDouble();
        m_timeToFull = map.value(QString("TimeToFull")).toLongLong();
        m_timeToEmpty = map.value(QString("TimeToEmpty")).toLongLong();
    }
}

bool UpowerBatteryDBus::batteryIsPresent()
{
    return m_batteryPresent;
}

int UpowerBatteryDBus::getBatteryState()
{
    return m_batteryState;
}

double UpowerBatteryDBus::getBatteryPercentage()
{
    return m_percentage;
}

qlonglong UpowerBatteryDBus::getTimeToFull()
{
    return m_timeToFull;
}

qlonglong UpowerBatteryDBus::getTimeToEmpty()
{
    return m_timeToEmpty;
}

void UpowerBatteryDBus::refreshBatteryInfo()
{
    QDBusInterface dBusInterface("org.freedesktop.UPower",
                                 m_batteryDBusPath,
                                 "org.freedesktop.UPower.Device",
                                 QDBusConnection::systemBus());
    dBusInterface.call("Refresh");
}
