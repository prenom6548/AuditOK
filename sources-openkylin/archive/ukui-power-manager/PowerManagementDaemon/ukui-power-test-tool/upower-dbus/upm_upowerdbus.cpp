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

    int index = 0;
    QStringList batteryPathList = m_upowerDBus->getBatteryPathList();
    for (index = 0; index < batteryPathList.size(); ++index) {
        UpowerBatteryDBus *upowerBatteryDBus = new UpowerBatteryDBus(index, batteryPathList.at(index), this);
        connect(upowerBatteryDBus, &UpowerBatteryDBus::batteryInfoChanged, this, &UpmUpowerDBus::dealBatteryInfoChanged);
        m_batteryDBusVector << upowerBatteryDBus;
    }

    QStringList linePowerPathList = m_upowerDBus->getLinePowerPathList();
    for (index = 0; index < linePowerPathList.size(); ++index) {
        UpowerLinePowerDBus *upowerLinePowerDBus = new UpowerLinePowerDBus(index, linePowerPathList.at(index), this);
        connect(upowerLinePowerDBus, &UpowerLinePowerDBus::acOnlineStateChanged, this, &UpmUpowerDBus::dealAcOnlineStateChanged);
        m_linePowerDBusVector << upowerLinePowerDBus;
    }
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
    emit lidStateChanged(value);
}

void UpmUpowerDBus::dealBatteryOnlineStateChanged(bool value)
{
    emit batteryOnlineStateChanged(value);
}

void UpmUpowerDBus::dealBatteryInfoChanged(int index, QStringList batteryInfoChangedList)
{
    emit batteryInfoChanged(index, batteryInfoChangedList);
}

void UpmUpowerDBus::dealAcOnlineStateChanged(int index, bool value)
{
    emit acOnlineStateChanged(index, value);
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
            return true;
        }
    }
    return false;
}

bool UpmUpowerDBus::batteryIsPresent(int index)
{
    if (0 == m_batteryDBusVector.size()) {
        return false;
    }
    return m_batteryDBusVector.at(index)->batteryIsPresent();
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

double UpmUpowerDBus::getEnergyNow(int index)
{
    if (0 == m_batteryDBusVector.size()) {
        return 0;
    }

    return m_batteryDBusVector.at(index)->getEnergyNow();
}

double UpmUpowerDBus::getEnergyFull(int index)
{
    if (0 == m_batteryDBusVector.size()) {
        return 0;
    }

    return m_batteryDBusVector.at(index)->getEnergyFull();
}

double UpmUpowerDBus::getEnergyFullDesign(int index)
{
    if (0 == m_batteryDBusVector.size()) {
        return 0;
    }

    return m_batteryDBusVector.at(index)->getEnergyFullDesign();
}

bool UpmUpowerDBus::getAcOnlineState()
{
    if (0 == m_batteryDBusVector.size() || 0 == m_linePowerDBusVector.size()) {
        return true;
    }
    for (int index = 0; index < m_linePowerDBusVector.size(); ++index) {
        if (true == m_linePowerDBusVector.at(index)->getAcOnlineState()) {
            return true;
        }
    }
    return false;
}
