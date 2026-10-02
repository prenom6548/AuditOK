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

#ifndef UPOWERBATTERYDBUS_H
#define UPOWERBATTERYDBUS_H

#include <QObject>
#include <QtDBus>
#include <QDBusInterface>

typedef enum {
    battery_state_unknown = 0,
    battery_state_charging,
    battery_state_discharging,
    battery_state_empty,
    battery_state_fully,
    battery_state_pending_charge,
    battery_state_pending_discharge,
    battery_state_last
} BatteryState;

class UpowerBatteryDBus : public QObject
{
    Q_OBJECT
public:
    explicit UpowerBatteryDBus(int batteryId, const QString &batteryDBusPath, QObject *parent = nullptr);
    ~UpowerBatteryDBus();

private:
    void initBatteryInfo();

    int m_batteryId;

    QString m_batteryDBusPath;

    QDBusInterface *m_DBusInterface;

    bool m_batteryPresent;
    int m_batteryState;
    double m_percentage;
    qlonglong m_timeToFull;
    qlonglong m_timeToEmpty;
    QVector<qlonglong> m_timeToEmptyRecords;// 存储100%电量的预估续航时间，记录3次
    const int maxSize = 3;

Q_SIGNALS:
    void batteryInfoChanged(int, QStringList);

private Q_SLOTS:
    void dealBatteryPropertiesMessage(QDBusMessage);
    void recordTimeToEmpty(qlonglong time);
    qlonglong averageTimeToEmpty() const;

public:
    bool batteryIsPresent();
    int getBatteryState();
    double getBatteryPercentage();
    qlonglong getTimeToFull();
    qlonglong getTimeToEmpty();
    void refreshBatteryInfo();
};

#endif // UPOWERBATTERYDBUS_H
