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
#ifndef UPTTBATTERYDATE_H
#define UPTTBATTERYDATE_H

#include <QObject>
#include <QDateTime>
#include <QString>

class UpttBatteryDate : public QObject
{
    Q_OBJECT
public:
    explicit UpttBatteryDate(qint64 percentage, double energyNow,
                             QDateTime lastTime, double lastEnergy,
                             QObject *parent = nullptr);
    ~UpttBatteryDate();

    QDateTime getRecordDateTime();
    qint64 getBatteryPercentage();
    qint64 getElapsedTime();
    double getBatteryEnergy();
    double getBatteryEnergyRate();

private:
    QDateTime m_recordDateTime;
    qint64 m_elapsedTimeSecs;
    qint64 m_percentage;
    double m_energyNow;
    double m_energyRate;
};

#endif // UPTTBATTERYDATE_H
