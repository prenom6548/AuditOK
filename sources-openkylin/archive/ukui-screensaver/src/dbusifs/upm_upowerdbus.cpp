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
#include <QDebug>

#include "upm_upowerdbus.h"

Q_GLOBAL_STATIC(UpmUpowerDBus, s_upowerDBus)

UpmUpowerDBus::UpmUpowerDBus(QObject *parent) : QObject(parent)
{
    m_upowerDBus = new UPowerDBus(this);
    connect(m_upowerDBus, &UPowerDBus::lidStateChanged, this, &UpmUpowerDBus::dealLidStateChanged);
    connect(m_upowerDBus, &UPowerDBus::batteryOnlineStateChanged, this, &UpmUpowerDBus::dealBatteryOnlineStateChanged);

    initializeBatteryObjects();

    QStringList linePowerPathList = m_upowerDBus->getLinePowerPathList();
    for (int index = 0; index < linePowerPathList.size(); ++index) {
        UpowerLinePowerDBus *upowerLinePowerDBus = new UpowerLinePowerDBus(index, linePowerPathList.at(index), this);
        connect(upowerLinePowerDBus, &UpowerLinePowerDBus::acOnlineStateChanged, this, &UpmUpowerDBus::dealAcOnlineStateChanged);
        m_linePowerDBusVector << upowerLinePowerDBus;
    }

    qDebug() << "Battery Num:" << m_batteryDBusVector.size();
    qDebug() << "line power Num:" << m_linePowerDBusVector.size();

    QDBusConnection::systemBus().connect("org.freedesktop.UPower",
                                          "/org/freedesktop/UPower",
                                          "org.freedesktop.UPower",
                                          "DeviceAdded",
                                          this,
                                          SLOT(onDeviceAdded(QDBusObjectPath)));

    QDBusConnection::systemBus().connect("org.freedesktop.UPower",
                                          "/org/freedesktop/UPower",
                                          "org.freedesktop.UPower",
                                          "DeviceRemoved",
                                          this,
                                          SLOT(onDeviceRemoved(QDBusObjectPath)));

}

UpmUpowerDBus::~UpmUpowerDBus() {}

UpmUpowerDBus *UpmUpowerDBus::self()
{
    return s_upowerDBus;
}

void UpmUpowerDBus::refreshBatteryInfo()
{
    for (int batteryNum = 0; batteryNum < m_batteryDBusVector.size(); ++batteryNum) {
        m_batteryDBusVector.at(batteryNum)->refreshBatteryInfo();
    }
}

void UpmUpowerDBus::dealLidStateChanged(bool value)
{
    qDebug() << "lid state changed:" << value;
    Q_EMIT lidStateChanged(value);
}

void UpmUpowerDBus::dealBatteryOnlineStateChanged(bool value)
{
    Q_EMIT batteryOnlineStateChanged(value);
}

void UpmUpowerDBus::dealBatteryInfoChanged(int index, QStringList batteryInfoChangedList)
{
    qDebug() << "battery info changed";
    Q_EMIT batteryInfoChanged(index, batteryInfoChangedList);
}

void UpmUpowerDBus::dealAcOnlineStateChanged(int index, bool value)
{
    qDebug() << "ac online changed:" << value;
    Q_EMIT acOnlineStateChanged(index, value);
}

bool UpmUpowerDBus::getLidClosedState()
{
    return m_upowerDBus->getLidClosedState();
}

bool UpmUpowerDBus::getBatteryOnlineState()
{
    return m_upowerDBus->getBatteryOnlineState();
}

bool UpmUpowerDBus::hasBattery()
{
    if (0 == m_batteryDBusVector.size()) {
        return false;
    }

    for (int batteryNum = 0; batteryNum < m_batteryDBusVector.size(); ++batteryNum) {
        if (true == batteryIsPresent(batteryNum)) {
            qInfo() << "Equipment has battery";
            return true;
        }
    }
    qInfo() << "Equipment no battery";
    return false;
}

bool UpmUpowerDBus::batteryIsPresent(int index)
{
    return m_batteryDBusVector.at(index)->batteryIsPresent();
}

int UpmUpowerDBus::getBatteryState(int index)
{
    if (0 == m_batteryDBusVector.size()) {
        return 0;
    }
    return m_batteryDBusVector.at(index)->getBatteryState();
}

int UpmUpowerDBus::getBatteryNum()
{
    return m_batteryDBusVector.size();
}

int UpmUpowerDBus::getLinePowerNum()
{
    return m_linePowerDBusVector.size();
}

double UpmUpowerDBus::getBatteryPercentage()
{
    if (0 == m_batteryDBusVector.size()) {
        return 0;
    }

    double percentage = 0.0;
    for (int index = 0; index < m_batteryDBusVector.size(); ++index) {
        percentage += m_batteryDBusVector.at(index)->getBatteryPercentage();
    }

    return percentage / m_batteryDBusVector.size();
}

qlonglong UpmUpowerDBus::getTimeToFull(int index)
{
    if (0 == m_batteryDBusVector.size()) {
        return 0;
    }
    return m_batteryDBusVector.at(index)->getTimeToFull();
}

qlonglong UpmUpowerDBus::getTimeToEmpty(int index)
{
    if (0 == m_batteryDBusVector.size()) {
        return 0;
    }
    return m_batteryDBusVector.at(index)->getTimeToEmpty();
}

QString UpmUpowerDBus::getBatteryIconName(int index)
{
    //处理没有电池的情况
    if(!hasBattery()) {
        if(m_linePowerDBusVector.size() > 0) {
            if(true == getAcOnlineState()) {
                qDebug() << "Laptop with no battery detected, showing special battery icon";
                return "battery-error-charging-symbolic";
            }
        }
    }

    if (0 == m_batteryDBusVector.size() || 0 == m_linePowerDBusVector.size()) {
        return "battery-level-0-charging-symbolic";
    }

    double percentage = m_batteryDBusVector.at(index)->getBatteryPercentage();
    if (true == getAcOnlineState()) {
        m_batteryIconName = QString("battery-level-%1-charging-symbolic").arg(qRound(percentage / 10.0) * 10);
    } else {
        m_batteryIconName = QString("battery-level-%1-symbolic").arg(qRound(percentage / 10.0) * 10);
    }

    return m_batteryIconName;
}

bool UpmUpowerDBus::getAcOnlineState()
{
    if (0 == m_linePowerDBusVector.size()) {
        return false;
    }

    for (int index = 0; index < m_linePowerDBusVector.size(); ++index) {
        if (true == m_linePowerDBusVector.at(index)->getAcOnlineState()
                && true == m_linePowerDBusVector.at(index)->getOnLineStateFromSysFile()) {
            return true;
        }
    }
    return false;
}

void UpmUpowerDBus::onDeviceAdded(QDBusObjectPath devicePath)
{
    const QString path = devicePath.path();
    qInfo() << "Device added:" << path;
    if(m_upowerDBus->getDeviceType(path) == up_device_type_battery) {
        rebuildBatteryObjects();
        Q_EMIT deviceChanged();
    }
}

void UpmUpowerDBus::onDeviceRemoved(QDBusObjectPath devicePath)
{
    const QString path = devicePath.path();
    qInfo() << "Device removed:" << path;
    if(path.contains("BAT")) {
        rebuildBatteryObjects();
        Q_EMIT deviceChanged();
    }
}

void UpmUpowerDBus::clearBatteryObjects()
{
    for (auto* battery : m_batteryDBusVector) {
        delete battery;
    }
    m_batteryDBusVector.clear();
}

void UpmUpowerDBus::initializeBatteryObjects()
{
    QStringList batteryPathList = m_upowerDBus->getBatteryPathList();
    for (int index = 0; index < batteryPathList.size(); ++index) {
        UpowerBatteryDBus *upowerBatteryDBus = new UpowerBatteryDBus(index, batteryPathList.at(index), this);
        connect(upowerBatteryDBus, &UpowerBatteryDBus::batteryInfoChanged,
               this, &UpmUpowerDBus::dealBatteryInfoChanged);
        m_batteryDBusVector << upowerBatteryDBus;
    }
    qDebug() << "Initialized" << m_batteryDBusVector.size() << "battery objects";
}

void UpmUpowerDBus::rebuildBatteryObjects()
{
    clearBatteryObjects();
    m_upowerDBus->refreshDevicePathLists();
    initializeBatteryObjects();
}


bool UpmUpowerDBus::getAcOnlineStateFromSysfile()
{
    if (0 == m_linePowerDBusVector.size()) {
        return false;
    }

    for (int index = 0; index < m_linePowerDBusVector.size(); ++index) {
        if (true == m_linePowerDBusVector.at(index)->getOnLineStateFromSysFile()) {
            return true;
        }
    }
    return false;
}
