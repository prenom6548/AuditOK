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
#ifndef NETWORK_DEVICE_MONITOR_H
#define NETWORK_DEVICE_MONITOR_H

#include <QObject>
#include "device_information.h"


class NetworkDeviceMonitor : public QObject
{
    Q_OBJECT
public:
    static NetworkDeviceMonitor *getInstance();
    ~NetworkDeviceMonitor();

    static QList<DeviceInformation> getDeviceFromIP(const QString &ip, const int port = 9100, ProtocolType protocolType = ProtocolType::SOCKET);
private:
    NetworkDeviceMonitor(/* args */);
signals:
    void getDeviceFromIP_async(const QString &ip, const int port = 9100, ProtocolType protocolType = ProtocolType::SOCKET);
    void getDeviceFromIP_cb(DeviceInformation &devInfo);
};

#endif // NETWORK_DEVICE_MONITOR_H