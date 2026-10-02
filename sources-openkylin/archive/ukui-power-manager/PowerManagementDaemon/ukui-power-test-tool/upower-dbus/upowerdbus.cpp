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

#include "upowerdbus.h"

UPowerDBus::UPowerDBus(QObject *parent) : QObject(parent)
{
    m_DBusInterface = new QDBusInterface("org.freedesktop.UPower",
                                         "/org/freedesktop/UPower",
                                         "org.freedesktop.DBus.Properties",
                                         QDBusConnection::systemBus());

    QDBusConnection::systemBus().connect("org.freedesktop.UPower",
                                         "/org/freedesktop/UPower",
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this,
                                         SLOT(dealUPowerDBusMessage(QDBusMessage)));

    QDBusInterface dBusInterface("org.freedesktop.UPower",
                                 "/org/freedesktop/UPower",
                                 "org.freedesktop.UPower",
                                 QDBusConnection::systemBus());

    QDBusReply<QList<QDBusObjectPath>> reply = dBusInterface.call("EnumerateDevices");
    if (dBusInterface.isValid()) {
        for (QDBusObjectPath dBusObjectPath : reply.value()) {
            int upDeviceType = getDeviceType(dBusObjectPath.path());
            if (up_device_type_line_power == upDeviceType) {
                m_linePowerPathList << dBusObjectPath.path();
            } else if (up_device_type_battery == upDeviceType) {
                m_batteryPathList << dBusObjectPath.path();
            }
        }
    }

    initDeviceInfo();
}

UPowerDBus::~UPowerDBus()
{
    delete m_DBusInterface;
}

int UPowerDBus::getDeviceType(const QString &dBusPath)
{
    QDBusInterface dBusInterface("org.freedesktop.UPower",
                                 dBusPath,
                                 "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = dBusInterface.call("Get", "org.freedesktop.UPower.Device", "Type");
    if (reply.isValid()) {
        return reply.value().toInt();
    } else {
        return up_device_type_unknown;
    }
}

void UPowerDBus::initDeviceInfo()
{
    QDBusMessage msg = m_DBusInterface->call("GetAll", "org.freedesktop.UPower");

    if (msg.type() == QDBusMessage::ReplyMessage) {
        const QDBusArgument &dbusArgs = msg.arguments().at(0).value<QDBusArgument>();
        QMap<QString, QVariant> map;
        dbusArgs >> map;

        m_hasLid = map.value(QString("LidIsPresent")).toBool();

        m_lidIsClosed = map.value(QString("LidIsClosed")).toBool();

        m_batteryOnline = map.value(QString("OnBattery")).toBool();

        m_upowerDaemonVersion = map.value(QString("DaemonVersion")).toString();
    }
}

void UPowerDBus::dealLidClosedMessage(bool value)
{
    if (m_lidIsClosed != value) {
        m_lidIsClosed = value;
        Q_EMIT lidStateChanged(m_lidIsClosed);
    }
}

void UPowerDBus::dealBatteryOnlineMessage(bool value)
{
    if (m_batteryOnline != value) {
        m_batteryOnline = value;
        emit batteryOnlineStateChanged(m_batteryOnline);
    }
}

void UPowerDBus::dealUPowerDBusMessage(QDBusMessage msg)
{
    const QDBusArgument &dbusArgs = msg.arguments().at(1).value<QDBusArgument>();
    QMap<QString,QVariant> map;
    dbusArgs >> map;

    if (map.contains("LidIsClosed")) {
        dealLidClosedMessage(map.value(QString("LidIsClosed")).toBool());
    }
    if (map.contains("OnBattery")) {
        dealBatteryOnlineMessage(map.value(QString("OnBattery")).toBool());
    }
}

bool UPowerDBus::hasLid()
{
    return m_hasLid;
}

bool UPowerDBus::getLidClosedState()
{
    return m_lidIsClosed;
}

bool UPowerDBus::getBatteryOnlineState()
{
   return m_batteryOnline;
}

QString UPowerDBus::getUpowerDaemonVersion()
{
    return m_upowerDaemonVersion;
}

QStringList UPowerDBus::getBatteryPathList()
{
    return m_batteryPathList;
}

QStringList UPowerDBus::getLinePowerPathList()
{
    return m_linePowerPathList;
}
