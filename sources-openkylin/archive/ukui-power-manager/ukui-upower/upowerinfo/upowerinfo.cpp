/*
 * Copyright 2021 KylinSoft Co., Ltd.
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

#include "upowerinfo.h"

UPowerInfo::UPowerInfo(QObject *parent) : QObject(parent)
{
    iface = new QDBusInterface(UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());

    m_hasBattery = false;
    mOnBattery = false;
    mLidIsClosed = false;
}

UPowerInfo::~UPowerInfo()
{
    delete iface;
}

void UPowerInfo::initUpowerInfo()
{
    getBatteryPath();
    if (true == m_hasBattery) {
        QDBusConnection::systemBus().connect(UPOWER_SERVICE,
                                             UPOWER_PATH,
                                             FREEDESKTOP_UPOWER,
                                             "PropertiesChanged",
                                             this,
                                             SLOT(dealMessage(QDBusMessage)));
        mOnBattery = OnBattery();
        mLidIsClosed = LidIsClosed();
    }
}

QString UPowerInfo::UPowerVersion()
{
    return "3.1";
}

QString UPowerInfo::CanHibernate()
{
    QDBusInterface interface(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        QDBusConnection::systemBus());
    if (!interface.isValid()) {
        qCritical() << QDBusConnection::sessionBus().lastError().message();
    }
    /*调用远程的 CanHibernate 方法，判断是否可以执行休眠的操作,返回值为yes为允许执行休眠，no为无法执行休眠
     * na为交换分区不足*/
    QDBusReply<QString> reply = interface.call("CanHibernate");
    if (reply.isValid()) {
        return reply.value();
    } else {
        return "Call Dbus method failed";
    }
}

QString UPowerInfo::MachineType()
{
    QDBusReply<QVariant> reply = iface->call("Get", UPOWER_SERVICE, "LidIsPresent");
    if (reply.isValid()) {
        if (true == reply.value().toBool()) {
            return "book";
        } else {
            return "pc";
        }
    } else {
        qDebug() << "Get lid is present failed";
        return "Get lid is present failed";
    }
}

bool UPowerInfo::CanAdjustBacklight()
{
    QDBusInterface controlCenterDBus("com.control.center.qt.systemdbus",
                                     "/", "com.control.center.interface",
                                     QDBusConnection::systemBus());
    QDBusReply<QString> reply = controlCenterDBus.call("getDmidecodeType");
    qDebug() << "dmidecode type:" << reply.value();
    if (0 == QString::compare("desktop", reply.value(), Qt::CaseInsensitive)) {
        return false;
    }

    QDir dir("/sys/class/backlight");
    QStringList filter;
    QStringList filterAll;
    filter << ".*";
    dir.setNameFilters(filter);
    QList<QFileInfo> fileInfo = dir.entryInfoList(filter);
    int num = fileInfo.count();
    fileInfo = dir.entryInfoList(filterAll);
    int numAll = fileInfo.count();
    if (0 < numAll - num) {
        return true;
    } else {
        return false;
    }
}

bool UPowerInfo::OnBattery()
{
    //    QDBusInterface iface(UPOWER_SERVICE,UPOWER_PATH,
    //                         FREEDESKTOP_UPOWER,
    //                         QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface->call("Get", UPOWER_SERVICE, "OnBattery");
    if (reply.isValid()) {
        return reply.value().toBool();
    } else {
        qDebug() << "Get on battery failed";
        return false;
    }
}

bool UPowerInfo::LidIsClosed()
{
    QDBusReply<QVariant> reply = iface->call("Get", UPOWER_SERVICE, "LidIsClosed");
    if (reply.isValid()) {
        return reply.value().toBool();
    } else {
        qDebug() << "Get lid is close failed";
        return false;
    }
}

QString UPowerInfo::DaemonVersion()
{
    //    QDBusInterface iface(UPOWER_SERVICE,UPOWER_PATH,
    //                         FREEDESKTOP_UPOWER,
    //                         QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface->call("Get", UPOWER_SERVICE, "DaemonVersion");
    if (reply.isValid()) {
        return reply.value().toString();
    } else {
        qDebug() << "Get version failed";
        return "Get version failed!";
    }
}

QString UPowerInfo::getBatteryPath()
{
    QString batteryPath = "";
    QDBusInterface dface(UPOWER_SERVICE, UPOWER_PATH, UPOWER_INTERFACE, QDBusConnection::systemBus());
    QDBusReply<QList<QDBusObjectPath>> reply = dface.call("EnumerateDevices");
    if (dface.isValid()) {
        for (QDBusObjectPath op : reply.value()) {
            int upDeviceType = getDeviceType(op.path());
            if (up_device_type_battery == upDeviceType) {
                batteryPath = op.path();
                m_hasBattery = true;
                break;
            }
        }
    } else {
        qDebug() << "Enumerate devices failed";
    }
    return batteryPath;
}

int UPowerInfo::getDeviceType(const QString &dBusPath)
{
    qDebug() << "device dBus object path:" << dBusPath;

    // upower dbus device path /org/freedesktop/UPower/devices/keyboard_hidpp_battery_0
    QString deviceName = dBusPath.split('/').last();
    QString deviceType = deviceName.split('_').first();

    if ("keyboard" == deviceType) {
        return up_device_type_keyboard;
    }

    if ("mouse" == deviceType) {
        return up_device_type_mouse;
    }

    QDBusInterface dBusInterface("org.freedesktop.UPower",
                                 dBusPath,
                                 "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = dBusInterface.call("Get", "org.freedesktop.UPower.Device", "Type");
    if (reply.isValid()) {
        return reply.value().toInt();
    } else {
        qDebug() << "Get device type failed";
        return up_device_type_unknown;
    }
}

void UPowerInfo::dealMessage(QDBusMessage msg)
{
    QString key;
    QVariant value;
    QList<QVariant> outArgs = msg.arguments();
    QVariant upowerDate = outArgs.at(1);
    const QDBusArgument &dbusArgs = upowerDate.value<QDBusArgument>();
    dbusArgs.beginMap();
    while (!dbusArgs.atEnd()) {
        dbusArgs.beginMapEntry();
        dbusArgs >> key >> value;
        dbusArgs.endMapEntry();
    }
    dbusArgs.endMap();
    if ("LidIsClosed" == key) {
        dealLidMessage(value.toBool());
    } else if ("OnBattery" == key) {
        dealPowerMessage(value.toBool());
    } else {
        qDebug() << "Upower message get failed";
    }
}

void UPowerInfo::dealPowerMessage(bool value)
{
    if (mOnBattery != value) {
        mOnBattery = value;
        emit acChanged(mOnBattery);
    }
}

void UPowerInfo::dealLidMessage(bool value)
{
    if (mLidIsClosed != value) {
        mLidIsClosed = value;
        emit lidChanged(mLidIsClosed);
    }
}
