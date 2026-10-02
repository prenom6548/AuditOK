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
#include <QThread>

#include "device_information.h"
#include "network_device_monitor.h"
#include "network_device_detector.h"
#include "information_collector.h"
#include "connect.h"
#include "common.h"
#include "smbclient_helper.h"

NetworkDeviceMonitor *NetworkDeviceMonitor::getInstance()
{
    static NetworkDeviceMonitor *instance = nullptr;
    static QThread *thread;
    if (nullptr == instance) {
        instance = new NetworkDeviceMonitor();
        thread = new QThread();
        instance->moveToThread(thread);
        
        connect(instance, &NetworkDeviceMonitor::getDeviceFromIP_async, instance, [=](const QString &ip, const int port, ProtocolType protocolType) {
            QList<DeviceInformation> devInfoList = instance->getDeviceFromIP(ip, port, protocolType);
            if (devInfoList.isEmpty()) {
                devInfoList.append(DeviceInformation());
            }
            
            for (auto devInfo : devInfoList) {
                emit instance->getDeviceFromIP_cb(devInfo);
            }
        });
        thread->start();
    }
    return instance;
}

NetworkDeviceMonitor::NetworkDeviceMonitor()
{

}

NetworkDeviceMonitor::~NetworkDeviceMonitor()
{

}

QStringList get_possible_queue_names()
{
    QStringList candidates;
    candidates << "PASSTHRU" << "ps" << "lp" << "PORT1" << "";
    for (int i = 0; i < 8; i++) {
        candidates.append(QString("LPT%1").arg(i));
        candidates.append(QString("LPT%1_PASSTHRU").arg(i));
        candidates.append(QString("COM%1").arg(i));
        candidates.append(QString("COM%1_PASSTHRU").arg(i));
    }

    for (int i = 0; i < 50; i++) {
        candidates.append(QString("pr%1").arg(i));
    }
    return candidates;
}

QList<DeviceInformation> NetworkDeviceMonitor::getDeviceFromIP(const QString &ip, const int port, ProtocolType protocolType)
{
    QList<DeviceInformation> res;
    if (protocolType == ProtocolType::SMB) {
        return res;
    }

    if (protocolType == ProtocolType::LPD) {
        auto candidates = get_possible_queue_names();
        DeviceInformation commoninfo;
        commoninfo.host = ip;
        NetworkDeviceDetector::getInstance()->detectVendorAndModel(commoninfo);
        for (auto candidate : candidates) {
            if (try_connect(QTC(ip), 515, QTC(QString("\2%1\n").arg(candidate))) != 0) {
                break;
            }

            DeviceInformation info;
            info.uri = QString("lpd://%1/%2").arg(ip, candidate);
            info.name = candidate;
            info.deviceType = DeviceType::LPD;
            info.protocolType = ProtocolType::LPD;
            info.vendor = commoninfo.vendor;
            info.model = commoninfo.model;
            res.append(info);
        }

        return res;
    }

    return NetworkDeviceDetector::getDeviceInfoFromHost(ip, port, protocolType);
}

