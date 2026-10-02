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

#include "upowerbatterydbus.h"

UpowerBatteryDBus::UpowerBatteryDBus(int batteryId, const QString &batteryDBusPath, QObject *parent)
    : QObject(parent), m_batteryId(batteryId), m_batteryDBusPath(batteryDBusPath)
{
    m_DBusInterface = new QDBusInterface("org.freedesktop.UPower",
                                         m_batteryDBusPath,
                                         "org.freedesktop.DBus.Properties",
                                         QDBusConnection::systemBus(),
                                         this);

    QDBusConnection::systemBus().connect("org.freedesktop.UPower",
                                         m_batteryDBusPath,
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this,
                                         SLOT(dealBatteryPropertiesMessage(QDBusMessage)));

    initBatteryInfo();
}

UpowerBatteryDBus::~UpowerBatteryDBus() {}

void UpowerBatteryDBus::dealBatteryPropertiesMessage(QDBusMessage msg)
{
    const QDBusArgument &dbusArgs = msg.arguments().at(1).value<QDBusArgument>();
    QMap<QString, QVariant> map;
    dbusArgs >> map;

    QStringList batteryInfoChangedList;
    static double prePercentage = 0.0;

    if (map.contains("IsPresent")) {
        m_batteryPresent = map.value(QString("IsPresent")).toBool();
        batteryInfoChangedList.append("IsPresent");
    }
    if (map.contains("State")) {
        m_batteryState = map.value(QString("State")).toInt();
        batteryInfoChangedList.append("State");
        if (m_batteryState != battery_state_discharging) {
            m_timeToEmptyRecords.clear();
            qInfo() << "非电池供电状态，清空可用时间记录";
        }
    }
    if (map.contains("Percentage")) {
        m_percentage = map.value(QString("Percentage")).toDouble();
        batteryInfoChangedList.append("Percentage");

        /**只有在电量变化时才更新剩余可用时间;这里如果电量变化做一个主动地refresh，为了更新TimeToEmpty，这里不去更改静态变量prePercentage**/
        if(fabs(prePercentage - m_percentage) < 0.5) {
            qInfo() << "battery percentage not change, not refresh";
        } else {
            qInfo() << "battery percentage change, refresh upower";
            m_DBusInterface->call(QLatin1String("Refresh"));
        }
        /****/
    }
    if (map.contains("TimeToFull")) {
        m_timeToFull = map.value(QString("TimeToFull")).toLongLong();
        batteryInfoChangedList.append("TimeToFull");
    }
    if (map.contains("TimeToEmpty")) {
        /**只有在电量变化时才更新剩余可用时间**/
        qlonglong timeToEmpty = map.value(QString("TimeToEmpty")).toLongLong();
        if (timeToEmpty != 0) {
            recordTimeToEmpty(timeToEmpty);
        }

        if(fabs(prePercentage - m_percentage) < 0.5) {
            qInfo() << "battery percentage not change, not update TimeToEmpty";
        } else {
            qInfo() << "battery percentage change,update TimeToEmpty";
            prePercentage = m_percentage;
            /****/

            //m_timeToEmpty = map.value(QString("TimeToEmpty")).toLongLong();
            m_timeToEmpty = averageTimeToEmpty();
            qInfo() << "averageTimeToEmpty is  " << m_timeToEmpty;
            if (m_timeToEmpty == 0) {
                m_timeToEmpty = map.value(QString("TimeToEmpty")).toLongLong();
            }
            batteryInfoChangedList.append("TimeToEmpty");
        }
    }

    if (0 != batteryInfoChangedList.size()) {
        Q_EMIT batteryInfoChanged(m_batteryId, batteryInfoChangedList);
    }
}

void UpowerBatteryDBus::initBatteryInfo()
{
    QDBusMessage msg = m_DBusInterface->call("GetAll", "org.freedesktop.UPower.Device");

    if (msg.type() == QDBusMessage::ReplyMessage) {
        const QDBusArgument &dbusArgs = msg.arguments().at(0).value<QDBusArgument>();
        QMap<QString, QVariant> map;
        dbusArgs >> map;

        m_batteryPresent = map.value(QString("IsPresent")).toBool();
        m_batteryState = map.value(QString("State")).toInt();
        m_percentage = map.value(QString("Percentage")).toDouble();
        m_timeToFull = map.value(QString("TimeToFull")).toLongLong();
        m_timeToEmpty = map.value(QString("TimeToEmpty")).toLongLong();
    }
}

bool UpowerBatteryDBus::batteryIsPresent()
{
    return m_batteryPresent;
}

int UpowerBatteryDBus::getBatteryState()
{
    return m_batteryState;
}

double UpowerBatteryDBus::getBatteryPercentage()
{
    return m_percentage;
}

qlonglong UpowerBatteryDBus::getTimeToFull()
{
    return m_timeToFull;
}

qlonglong UpowerBatteryDBus::getTimeToEmpty()
{
    return m_timeToEmpty;
}

void UpowerBatteryDBus::refreshBatteryInfo()
{
    QDBusInterface dBusInterface("org.freedesktop.UPower",
                                 m_batteryDBusPath,
                                 "org.freedesktop.UPower.Device",
                                 QDBusConnection::systemBus());
    dBusInterface.call("Refresh");
}

void UpowerBatteryDBus::recordTimeToEmpty(qlonglong time) {
    if (m_percentage < 1.0) {
        qWarning() << "battery percentage too small(<= 1),return";
        return;
    }

    if (m_percentage <= 10.0) {
        qWarning() << "battery percentage too small,just save one";
        m_timeToEmptyRecords.clear();
        m_timeToEmptyRecords.prepend(time * 100 / m_percentage);
        return;
    }

    if(m_timeToEmptyRecords.size() == maxSize) {
        m_timeToEmptyRecords.removeLast();
    }

    qInfo() << "record time " << time << " for percentage " << m_percentage;
    m_timeToEmptyRecords.prepend(time * 100 / m_percentage);// 存储电量百分百的时候预估的时间
}

qlonglong UpowerBatteryDBus::averageTimeToEmpty() const {
    if(m_timeToEmptyRecords.empty()) {
        return 0;
    }

    qlonglong sum = 0;
    for(int val : m_timeToEmptyRecords) {
        sum += val;
    }
    return sum * m_percentage / (m_timeToEmptyRecords.size() * 100);// 当前电量的预估的时间
}
