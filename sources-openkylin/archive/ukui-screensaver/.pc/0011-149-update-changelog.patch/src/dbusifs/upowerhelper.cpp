/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "upowerhelper.h"
#include "device.h"
#include "enginedevice.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>

const static QString upowerService = QStringLiteral("org.freedesktop.UPower");
const static QString upowerPath = QStringLiteral("/org/freedesktop/UPower");
const static QString upowerInterface = QStringLiteral("org.freedesktop.UPower");
const static QString propertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
const static QString upowerDisplayPath = QStringLiteral("/org/freedesktop/UPower/devices/DisplayDevice");
const static QString upowerDeviceInterface = QStringLiteral("org.freedesktop.UPower.Device");

UpowerHelper::UpowerHelper(QObject *parent) : QObject(parent)
{
    m_upowerService
        = new QDBusInterface(upowerService, upowerPath, upowerInterface, QDBusConnection::systemBus(), this);

    QString batteryPath = "";
    if (m_upowerService->isValid()) {
        QDBusReply<QList<QDBusObjectPath>> reply = m_upowerService->call("EnumerateDevices");
        for (QDBusObjectPath op : reply.value()) {
            if (op.path().contains("battery_")) {
                batteryPath = op.path();
                //                qDebug() << "battery path is :" << batteryPath;
                break;
            }
        }
    } else {
        qDebug() << "Enumerate devices failed";
    }

    if (!batteryPath.isEmpty()) {
        QDBusConnection::systemBus().connect(
            upowerService, batteryPath, propertiesInterface, "PropertiesChanged", this, SLOT(dealMessage(QDBusMessage)));
        m_batInterface
            = new QDBusInterface(upowerService, batteryPath, propertiesInterface, QDBusConnection::systemBus(), this);
    }

    m_upowerInterface
        = new QDBusInterface(upowerService, upowerPath, propertiesInterface, QDBusConnection::systemBus(), this);

    if (m_upowerInterface->isValid()) {
        QDBusReply<QVariant> interfaceReply = m_upowerInterface->call("Get", "org.freedesktop.UPower", "LidIsPresent");
        if (interfaceReply.isValid()) {
            m_isBattery = interfaceReply.value().toBool();
        }
        // 监听屏幕关屏信号
        QDBusConnection::systemBus().connect(
            upowerService, upowerPath, propertiesInterface, "PropertiesChanged", this, SLOT(onLidWatcherMessage()));
    }

    if (m_upowerService->isValid()) {
        m_engineDevice = EngineDevice::getInstance();
        // onBatteryChanged(m_engineDevice->engine_get_state());

        int size;
        size = m_engineDevice->devices.size();
        for (int i = 0; i < size; i++) {
            DEVICE *dv;
            dv = m_engineDevice->devices.at(i);

            if (dv->m_dev.kind == UP_DEVICE_KIND_LINE_POWER) {
                continue;
            }

            connect(m_engineDevice, &EngineDevice::engine_signal_Battery_State, this, &UpowerHelper::onBatteryChanged);
            continue;
        }
    }
}

UpowerHelper::~UpowerHelper() {}

QString UpowerHelper::getBatteryIconName()
{
    if (m_upowerService->isValid() && m_upowerInterface->isValid() && m_batInterface) {
        bool batteryState = false;
        QDBusReply<QVariant> reply = m_upowerInterface->call("Get", upowerService, "OnBattery");
        if (reply.isValid()) {
            batteryState = reply.value().toBool();
        }
        double percentage = -1.0;
        QDBusReply<QVariant> percentage_reply = m_batInterface->call("Get", upowerDeviceInterface, "Percentage");
        if (percentage_reply.isValid()) {
            percentage = percentage_reply.value().toDouble();
        }

        if (batteryState) {
            return QString("battery-level-%1-symbolic").arg((int)percentage / 10 * 10);
        } else {
            return QString("battery-level-%1-charging-symbolic").arg((int)percentage / 10 * 10);
        }
    }
    return QString();
}

QStringList UpowerHelper::getBatteryArgs()
{
    if (!m_engineDevice) {
        return QStringList();
    }
    return m_engineDevice->engine_get_state();
}

void UpowerHelper::onBatteryChanged(QStringList args)
{
    Q_EMIT batteryChanged(args);
}

void UpowerHelper::dealMessage(QDBusMessage)
{
    Q_EMIT batteryStatusChanged(getBatteryIconName());
}

void UpowerHelper::onLidWatcherMessage()
{
    QDBusInterface iface(upowerService, upowerPath, propertiesInterface, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", upowerInterface, "LidIsClosed");
    if (reply.isValid()) {
        bool isLidClosed = reply.value().toBool();
        Q_EMIT lidStateChanged(isLidClosed);
        qDebug() << "lid state:" << isLidClosed;
    } else {
        qDebug() << "Failed to get lid closed event!";
    }
}
