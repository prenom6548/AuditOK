/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "device-manager.h"
#include "clib-syslog.h"


DeviceManager::DeviceManager(QObject *parent) : QObject(parent)
{
    initSignalTimer();
    getDevInfoFromFile();
    initUdevConnect();
}

DeviceManager::~DeviceManager()
{
    if (m_monitor != nullptr) {
        udev_monitor_unref(m_monitor);
        m_monitor = nullptr;
    }

    if (m_udev != nullptr) {
        udev_unref(m_udev);
        m_udev = nullptr;
    }
}

int DeviceManager::GetDeviceNum(const QString &devType) const
{
    if (!devType.compare("keyboard", Qt::CaseInsensitive)
        || !devType.compare("kbd", Qt::CaseInsensitive)) {
        return getDevNumByType(DEVICEINFO_EV_KBD);
    }
    if (!devType.compare("mouse", Qt::CaseInsensitive)) {
        return getMouseNum();
    }
    if (!devType.compare("touchpad", Qt::CaseInsensitive)) {
        return getDevNumByName(devType);
    }
    return 0;
}

int DeviceManager::getDevNumByType(const QString &devType) const
{
    int devNum = 0;
    for (const auto list : m_devInfoMap.values()) {
        if (list.first().ev.contains(devType)) {
            devNum++;
        }
    }

    return devNum;
}

int DeviceManager::getDevNumByName(const QString &devName) const
{
    int devNum = 0;
    for (const auto list : m_devInfoMap.values()) {
        if (list.first().name.contains(devName, Qt::CaseInsensitive)) {
            devNum++;
        }
    }

    return devNum;
}

int DeviceManager::getMouseNum() const
{
    int num = 0;
    for (const auto list : m_devInfoMap.values()) {
        if (list.first().ev.contains(DEVICEINFO_EV_MOUSE) ||
            (list.first().ev.contains(DEVICEINFO_EV_MOUSE_EX) &&list.first().name.contains(DEVICEINFO_NAME_MOUSE))) {
            num ++;
        }
    }

    return num;
}

void DeviceManager::insertDevInfoIntoMap(const QString &id, const DevInfo &info)
{
    if (id.isEmpty()) {
        return;
    }

    if (m_devInfoMap.keys().contains(id)) {
        m_devInfoMap[id].append(info);
    } else {
        DevInfoList devInfoList = {info};
        m_devInfoMap.insert(id, devInfoList);
    }
}

bool DeviceManager::getDevInfoFromFile()
{
    QFile file(PROCBUSINPUTDEVICES);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        SYS_LOG(LOG_WARNING,"open file %s error!", PROCBUSINPUTDEVICES);
        return false;
    }

    QString devId;
    DevInfo devInfoTmp;

    //文件大小为0，file.atend()返回true，但实际文件内容可读，因此不使用逐行读取方式
    QString remaining = file.readAll();
    QStringList list = remaining.split("\n");
    for (QString line : list) {
        //文件使用空行区分不同设备
        if (line.isEmpty()) {
            insertDevInfoIntoMap(devId, devInfoTmp);
            continue;
        }

        if (line.startsWith("I: ")) {
            devId = line.split("I: ").back();
        } else if (line.startsWith("N: ")) {
            devInfoTmp.name = line.split("N: ").back();
        } else if (line.startsWith("P: ")) {
            devInfoTmp.phys = line.split("P: ").back();
        } else if (line.startsWith("S: ")) {
            devInfoTmp.sysfs = line.split("S: ").back();
        } else if (line.startsWith("B: ")) {
            QString tmpStr = line.split("B: ").back();
            if (tmpStr.contains("EV=")) {
                devInfoTmp.ev = tmpStr;
            }
        }
    }
    file.close();
    return true;
}

void DeviceManager::initUdevConnect()
{
    m_udev = udev_new();
    if (m_udev == nullptr) {
        SYS_LOG(LOG_WARNING, "New udev error!");
        return;
    }

    m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    if (m_monitor == nullptr) {
        SYS_LOG(LOG_WARNING, "New udev monitor error!");
        return;
    }

    udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "input", nullptr);  // 仅监听输入设备
    udev_monitor_enable_receiving(m_monitor);
    int fd = udev_monitor_get_fd(m_monitor);

    m_notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, [this] () {
        udev_device *dev = udev_monitor_receive_device(m_monitor);
        if (dev != nullptr) {
            m_signalMergingTimer->start(1000);
            udev_device_unref(dev);
        }
    });
}

void DeviceManager::initSignalTimer()
{
    m_signalMergingTimer = new QTimer(this);
    m_signalMergingTimer->setSingleShot(true);
    connect(m_signalMergingTimer, &QTimer::timeout, this, &DeviceManager::onFileChanged);
}

void DeviceManager::onFileChanged()
{
    m_devInfoMap.clear();
    getDevInfoFromFile();
    Q_EMIT DeviceNumChanged();
}


