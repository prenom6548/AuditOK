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
#include "upttbatterydate.h"

#include <QDebug>
UpttBatteryDate::UpttBatteryDate(qint64 percentage, double energyNow,
                                 QDateTime lastTime, double lastEnergy,
                                 QObject *parent) : QObject(parent)
{
    m_recordDateTime = QDateTime::currentDateTime();
    m_percentage = percentage;
    m_energyNow = energyNow;

    if (0 != lastEnergy) {
        m_elapsedTimeSecs = lastTime.secsTo(m_recordDateTime);
        double diffTimeHour = (double)m_elapsedTimeSecs / 3600;
        m_energyRate = (lastEnergy - m_energyNow) / diffTimeHour;
    }
}

UpttBatteryDate::~UpttBatteryDate() {}

QDateTime UpttBatteryDate::getRecordDateTime()
{
    return m_recordDateTime;
}

qint64 UpttBatteryDate::getBatteryPercentage()
{
    return m_percentage;
}

qint64 UpttBatteryDate::getElapsedTime()
{
    return m_elapsedTimeSecs;
}

double UpttBatteryDate::getBatteryEnergy()
{
    return m_energyNow;
}

double UpttBatteryDate::getBatteryEnergyRate()
{
    return m_energyRate;
}



