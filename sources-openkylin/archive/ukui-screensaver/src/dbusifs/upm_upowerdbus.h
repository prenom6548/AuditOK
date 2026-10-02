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
    bool getAcOnlineStateFromSysfile();

private:
    UPowerDBus *m_upowerDBus;

    QVector<UpowerBatteryDBus *> m_batteryDBusVector;

    QVector<UpowerLinePowerDBus *> m_linePowerDBusVector;

    QString m_batteryIconName;

    void initializeBatteryObjects();

    void clearBatteryObjects();

    void rebuildBatteryObjects();

Q_SIGNALS:
    void lidStateChanged(bool);

    void batteryOnlineStateChanged(bool);

    void batteryInfoChanged(int, QStringList);

    void acOnlineStateChanged(int, bool);

    void deviceChanged();

private:
    void dealLidStateChanged(bool);

    void dealBatteryOnlineStateChanged(bool);

    void dealBatteryInfoChanged(int, QStringList);

    void dealAcOnlineStateChanged(int, bool);

public Q_SLOTS:
    bool getLidClosedState();

    bool getBatteryOnlineState();

    bool hasBattery();

    bool batteryIsPresent(int index = 0);

    int getBatteryState(int index = 0);

    int getBatteryNum();

    int getLinePowerNum();

    double getBatteryPercentage();

    qlonglong getTimeToFull(int index = 0);

    qlonglong getTimeToEmpty(int index = 0);

    QString getBatteryIconName(int index = 0);

    bool getAcOnlineState();

    void onDeviceAdded(QDBusObjectPath);

    void onDeviceRemoved(QDBusObjectPath);
};

#endif // UPM_UPOWERDBUS_H
