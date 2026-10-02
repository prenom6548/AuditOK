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
#ifndef NETWORK_DEVICE_DETECTOR_H
#define NETWORK_DEVICE_DETECTOR_H

#include <QObject>

#include "device_information.h"

enum class DetectResult : int
{
    DETECT_OK = 0,
    DETECT_ERROR,
};

enum class SetResult : int
{
    SET_OK = 0,
    SET_ERROR,
};

class NetworkDeviceDetector: public QObject
{
    Q_OBJECT
public:
    static NetworkDeviceDetector *getInstance();
    ~NetworkDeviceDetector();
    static QList<DeviceInformation> getDeviceInfoFromHost(const QString &host, const int port, ProtocolType protocolType);
private:
    NetworkDeviceDetector();
    SetResult setUri                  (DeviceInformation &,
                                       const ProtocolType &);

    bool probe(DeviceInformation &deviceInfo, const QString host, const int port, ProtocolType protocolType);
    bool probeIpp(QList<DeviceInformation> &deviceInfoList, const QString host, const int port);

public:
    DetectResult detectNetWorkNode    (DeviceInformation &);
    DetectResult detectVendorAndModel (DeviceInformation &);
    DetectResult detectUUID           (DeviceInformation &);
    DetectResult detectSerial         (DeviceInformation &);

};

#endif // NETWORK_DEVICE_DETECTOR_H