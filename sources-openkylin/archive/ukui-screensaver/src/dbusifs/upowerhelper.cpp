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
#include <QCoreApplication>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include "freedesktophelper.h"
#include "upm_upowerdbus.h"
#include "dmi_chassis_type.h"

const static QString upowerService = QStringLiteral("org.freedesktop.UPower");
const static QString upowerPath = QStringLiteral("/org/freedesktop/UPower");
const static QString upowerInterface = QStringLiteral("org.freedesktop.UPower");
const static QString propertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
const static QString upowerDisplayPath = QStringLiteral("/org/freedesktop/UPower/devices/DisplayDevice");
const static QString upowerDeviceInterface = QStringLiteral("org.freedesktop.UPower.Device");

UpowerHelper::UpowerHelper(QObject *parent) : QObject(parent)
{
    if (!FreedesktopHelper::getInstance(false)->NameHasOwner(upowerService)) {
        connect(
            FreedesktopHelper::getInstance(false),
            &FreedesktopHelper::serviceStatusChanged,
            this,
            &UpowerHelper::onServiceStatusChanged);
        FreedesktopHelper::getInstance(false)->asyncStartService(upowerService);
    } else {
        m_serviceIsRuning = true;
    }
}

UpowerHelper::~UpowerHelper() {}

void UpowerHelper::init()
{
    if (m_isInited || !m_serviceIsRuning) {
        return;
    }
    m_isBattery = (UpmUpowerDBus::self()->hasBattery() || UpmUpowerDBus::self()->getAcOnlineState()) && (!isDesktop());

    connect(UpmUpowerDBus::self(), &UpmUpowerDBus::acOnlineStateChanged, this, &UpowerHelper::dealAcOnlineStateChanged);
    connect(UpmUpowerDBus::self(), &UpmUpowerDBus::batteryInfoChanged, this, &UpowerHelper::dealBatteryInfoChanged);
    connect(UpmUpowerDBus::self(), &UpmUpowerDBus::deviceChanged, this, &UpowerHelper::dealDeviceChanged);
    connect(UpmUpowerDBus::self(), &UpmUpowerDBus::lidStateChanged, this, &UpowerHelper::onLidStateChanged);
    m_isInited = true;
    qDebug() << "upowerServiceReady:" << m_isBattery;
    Q_EMIT upowerServiceReady();
}

void UpowerHelper::onServiceStatusChanged(QString strName, bool isActive)
{
    if (strName != upowerService) {
        return;
    }
    if (isActive) {
        m_serviceIsRuning = true;
        init();
    }
}

QString UpowerHelper::getBatteryIconName()
{
    if (m_isInited && UpmUpowerDBus::self()) {
        return UpmUpowerDBus::self()->getBatteryIconName();
    }
    return QString();
}

QStringList UpowerHelper::getBatteryArgs()
{
    QStringList batteryArgs;
    if (!m_isInited) {
        return batteryArgs;
    }
    int batteryState = UpmUpowerDBus::self()->getBatteryState();
    double batteryPercentage = UpmUpowerDBus::self()->getBatteryPercentage();
    qint64 timeToEmpty = UpmUpowerDBus::self()->getTimeToEmpty();
    qint64 timeToFull = UpmUpowerDBus::self()->getTimeToFull();
    bool hasBattery = UpmUpowerDBus::self()->hasBattery();
    bool acOnlineState = UpmUpowerDBus::self()->getAcOnlineState();
    batteryArgs.append(QString("%1").arg(batteryPercentage));
    batteryArgs.append(QString("%1").arg(batteryState));
    batteryArgs.append(QString("%1").arg(timeToEmpty));
    batteryArgs.append(QString("%1").arg(hasBattery));
    batteryArgs.append(QString("%1").arg(acOnlineState));
    batteryArgs.append(QString("%1").arg(timeToFull));
    return batteryArgs;
}

void UpowerHelper::dealAcOnlineStateChanged(int, bool)
{
    onUpdateBatteryInfo();
}

void UpowerHelper::dealBatteryInfoChanged(int, QStringList)
{
    onUpdateBatteryInfo();
}

void UpowerHelper::dealDeviceChanged()
{
    onUpdateBatteryInfo();
}

void UpowerHelper::onUpdateBatteryInfo()
{
    Q_EMIT batteryChanged(getBatteryArgs());
    Q_EMIT batteryStatusChanged(getBatteryIconName());
}

void UpowerHelper::onLidStateChanged(bool value)
{
    Q_EMIT lidStateChanged(value);
    qDebug() << "lid state:" << value;
}

bool UpowerHelper::isDesktop()
{
    QFile file("/sys/class/dmi/id/chassis_type");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open chassis_type file";
        return false;
    }

    QString chassisTypeStr = file.readAll().trimmed();
    file.close();

    bool ok = false;
    int chassisTypeInt = chassisTypeStr.toInt(&ok);
    if (!ok) {
        qWarning() << "Invalid chassis_type format:" << chassisTypeStr;
        return false;
    }

    if (static_cast<DmiChassisType>(chassisTypeInt) == DmiChassisType::Desktop) {
        return true; // 台式机
    }
    return false; // 非台式机
}
