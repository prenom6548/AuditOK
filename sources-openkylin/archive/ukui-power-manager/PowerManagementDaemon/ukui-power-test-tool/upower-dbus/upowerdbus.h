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

#ifndef UPOWERDBUS_H
#define UPOWERDBUS_H

#include <QObject>
#include <QtDBus>
#include <QDBusInterface>

enum up_device_type
{
    up_device_type_unknown = 0,
    up_device_type_line_power,
    up_device_type_battery,
    up_device_type_ups,
    up_device_type_monitor,
    up_device_type_mouse,
    up_device_type_keyboard,
    up_device_type_pda,
    up_device_type_phone,
    up_device_type_media_player,
    up_device_type_tablet,
    up_device_type_computer,
    up_device_type_max
};

class UPowerDBus : public QObject
{
    Q_OBJECT
public:
    explicit UPowerDBus(QObject *parent = nullptr);
    ~UPowerDBus();

private:
    int getDeviceType(const QString &);

    void initDeviceInfo();

    void dealLidClosedMessage(bool);

    void dealBatteryOnlineMessage(bool);

    QDBusInterface *m_DBusInterface;

    bool m_hasLid;

    bool m_lidIsClosed;

    bool m_batteryOnline;

    QString m_upowerDaemonVersion;

    QStringList m_batteryPathList;

    QStringList m_linePowerPathList;

private Q_SLOTS:
    void dealUPowerDBusMessage(QDBusMessage);

Q_SIGNALS:
    void lidStateChanged(bool);

    void batteryOnlineStateChanged(bool);

public:
    bool hasLid();

    bool getLidClosedState();

    bool getBatteryOnlineState();

    QString getUpowerDaemonVersion();

    QStringList getBatteryPathList();

    QStringList getLinePowerPathList();
};

#endif // UPOWERDBUS_H
