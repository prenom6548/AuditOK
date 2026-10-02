/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#ifndef DEVICE_MONITOR_H
#define DEVICE_MONITOR_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QSocketNotifier>

#include "device_information.h"




class DeviceMonitorBackend : public QObject
{
    Q_OBJECT

private:
    DeviceMonitorBackend();
public:
    static DeviceMonitorBackend *getInstance();
    ~DeviceMonitorBackend();
    bool doDeviceAdd      (const QString &);
    bool doDeviceRemove   (const QString &);

private:
    QMap<QString, DeviceInformation> m_mpDeviceInformationMap;
    void detectStart ();
    bool usbDeviceIdentify (const QString &);

    QSocketNotifier *notifier = nullptr;
    int hotplug_sock = -1;
    void readNetlinkMessage();

signals:
    void start();
    bool usbDeviceAdd      (const QString &);
    bool usbDeviceRemove   (const QString &);
    void findUsbConnect    (DeviceInformation);
    void findUsbDisconnect (QString);
};

#endif // DEVICE_MONITOR_H
