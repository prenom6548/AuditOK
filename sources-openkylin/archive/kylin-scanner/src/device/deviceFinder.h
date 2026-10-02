/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef UNTITLED6_DEVICEFINDER_H
#define UNTITLED6_DEVICEFINDER_H

#include <QTimer>
#include "device_information.h"
//#include "network_device_detector.h"
#include <QThread>
#include <sane/sane.h>

class deviceFinder : public QObject
{
    Q_OBJECT
public:
    deviceFinder(QObject *parent = nullptr);
    ~deviceFinder();
    QList<DeviceInformation> getList();
Q_SIGNALS:
    void finished();
    void succeed();
    void failed();
public:
    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }
    void buriedDeviceInfomation(QList<DeviceInformation> list);

private:
    void dowork();
    void doPackageWork();
    bool infoCheck(DeviceInformation &deviceInformation);
    bool checkDeviceExistInSane(DeviceInformation &deviceInformation);
    QThread *m_thread{nullptr};
    QList<DeviceInformation> m_infoList;
    QList<DeviceInformation> m_saneInfoList;
    bool m_isServerAddressConnected{false};
};


#endif//UNTITLED6_DEVICEFINDER_H
