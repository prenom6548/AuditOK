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
#ifndef UPM_UPOWERDBUS_H
#define UPM_UPOWERDBUS_H

#include <QObject>

#include "upowerdbus.h"
#include "upowerbatterydbus.h"
#include "upowerlinepowerdbus.h"

class UpmUpowerDBus : public QObject
{
    Q_OBJECT

public:
    explicit UpmUpowerDBus(QObject *parent = nullptr);
    ~UpmUpowerDBus();
    static UpmUpowerDBus* self();

    void refreshBatteryInfo();

private:
    UPowerDBus *m_upowerDBus;

    QVector<UpowerBatteryDBus *> m_batteryDBusVector;

    QVector<UpowerLinePowerDBus *> m_linePowerDBusVector;

    QString m_batteryIconName;

signals:
    void lidStateChanged(bool);

    void batteryOnlineStateChanged(bool);

    void batteryInfoChanged(int, QStringList);

    void acOnlineStateChanged(int, bool);

private:
    void dealLidStateChanged(bool);

    void dealBatteryOnlineStateChanged(bool);

    void dealBatteryInfoChanged(int, QStringList);

    void dealAcOnlineStateChanged(int, bool);

public:
    bool getLidClosedState();

    bool getBatteryOnlineState();

    bool hasBattery();

    bool batteryIsPresent(int index = 0);

    int getBatteryNum();

    int getLinePowerNum();

    double getBatteryPercentage();

    double getEnergyNow(int index = 0);

    double getEnergyFull(int index = 0);

    double getEnergyFullDesign(int index = 0);

    bool getAcOnlineState();
};

#endif // UPM_UPOWERDBUS_H
