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
 *
 * Authors: chenxuechao <chenxuechao@kylinos.cn>
 */

#include "device-monitor.h"

#include <QFile>
#include <QRegularExpression>
#include <QProcess>
#include <QDebug>

#include "clib-syslog.h"
#include "usdqjsonconf.h"

#define PROCBUSINPUTDEVICES "/proc/bus/input/devices"
#define USD_DEVICETYPE_FILE "/opt/system/conf/usd/deviceType.json"

DeviceMonitor *DeviceMonitor::instance()
{
    static DeviceMonitor deviceMonitor;
    return &deviceMonitor;
}

DeviceMonitor::DeviceMonitor(QObject *parent) : QObject(parent)
{

}

DeviceType DeviceMonitor::getTypeByBluetoothMac(const QString &mac) const
{
#ifdef USE_BLUETOOTHCTL
    QProcess process;
    process.start("bluetoothctl", QStringList() << "info" << mac);
    process.waitForFinished(300);
    QString output = process.readAllStandardOutput();
    QRegularExpression iconRegex("Icon:\\s+(\\w+)");
    QRegularExpressionMatch iconMatch = iconRegex.match(output);
    QString icon = iconMatch.captured(1);

    if (icon.contains("keyboard")) {
        return DeviceType::IN_KEYBOARD;
    }
    if (icon.contains("mouse")) {
        return DeviceType::IN_MOUSE;
    }
#endif
    return DeviceType::IN_NONE;
}

DeviceType DeviceMonitor::getTypeByBluetooth(const DeviceInfo &devInfo) const
{
    if (!devInfo.contains("Uniq")) {
        return DeviceType::IN_NONE;
    }

    //若phys是mac，则认为是蓝牙设备，通过uniq获取实际设备mac地址，根据设备mac地址获取设备类型
    QString uniq = devInfo.value("Uniq");
    if (!isMacAdress(uniq)) {
        SYS_LOG(LOG_NOTICE, "Device Phys is mac but Uniq isn't mac");
        return DeviceType::IN_NONE;
    }

    return getTypeByBluetoothMac(uniq);
}

bool DeviceMonitor::isMacAdress(const QString &str) const
{
    QRegularExpression regex("^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$");
    return regex.match(str).hasMatch();
}

DeviceInfo DeviceMonitor::getDevInfo(const QString &devStr) const
{
    // 初始化字段映射
    QRegularExpression pattern("(Vendor|Product|Phys|Uniq|EV)=([^\\s]+)");
    QRegularExpressionMatchIterator matchIterator = pattern.globalMatch(devStr);

    DeviceInfo infoMap;

    // 遍历匹配结果（虽然不使用循环，但这里用迭代器）
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        QString field = match.captured(1);
        QString value = match.captured(2);
        infoMap.insert(field, value);
    }

    return infoMap;
}

void DeviceMonitor::setDevTypeMap()
{
    for (auto devInfo : m_devInfoList) {
        if (!devInfo.contains("Vendor")
            || !devInfo.contains("Product")
            || !devInfo.contains("Phys")
            || !devInfo.contains("EV")) {
            continue;
        }

        QString productId = devInfo.value("Vendor") + devInfo.value("Product");
        if (m_devTypeMap.contains(productId)) {
            continue;
        }

        m_devTypeMap.insert(productId, getDevTypeFromDevInfo(productId));
    }
}

void DeviceMonitor::getDevInfoListFromFile()
{
    QFile file(PROCBUSINPUTDEVICES);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        SYS_LOG(LOG_WARNING,"open file %s error!", PROCBUSINPUTDEVICES);
        return;
    }

    QString remaining = file.readAll();
    file.close();
    QStringList devicelist = remaining.split("\n\n");
    for (const auto& device : devicelist) {
        //文件使用空行区分不同设备
        if (device.isEmpty()) {
            USD_LOG(LOG_DEBUG,"device msg is empty!!");
            continue;
        }

        DeviceInfo devInfo = getDevInfo(device);
        if (!m_devInfoList.contains(devInfo)) {
            m_devInfoList.append(devInfo);
        }
    }
}

DeviceType DeviceMonitor::getDevTypeFromDevInfo(const QString &prodId) const
{
    DeviceType devTypeByPort = DeviceType::IN_NONE;
    QString minPortStr;
    for (auto devInfo : m_devInfoList) {
        QString productId = devInfo.value("Vendor") + devInfo.value("Product");
        DeviceType devType = transEvToDeviceType(devInfo.value("EV"));
        if (prodId != productId) {
            continue;
        }

        QString phys = devInfo.value("Phys");
        if (isMacAdress(phys)) {
            return getTypeByBluetooth(devInfo);
        }

        if (!phys.contains("/input")) {
            return devType;
        }


        QString portStr = phys.split("/input").back();
        //若phys为端口，则根据input号区分主设备，input最小的是主设备
        if (minPortStr.isEmpty()) {
            minPortStr = portStr;
        }

        if (portStr.toDouble() > minPortStr.toDouble()) {
            continue;
        }

        if (devType != DeviceType::IN_NONE) {
            devTypeByPort = devType;
        }
    }
    return devTypeByPort;
}

DeviceType DeviceMonitor::transEvToDeviceType(const QString &ev) const
{
    DeviceType devType;
    if (ev == "120013") {
        devType = DeviceType::IN_KEYBOARD;
    } else if (ev == "17" || ev == "1f") {
        devType = DeviceType::IN_MOUSE;
    } else {
        //仅收集键盘和鼠标类型，其他类型均使用NONE记录
        devType = DeviceType::IN_NONE;
    }
    return devType;
}

void DeviceMonitor::initDeviceTypeFromUsdFile()
{
    if (!QFile(USD_DEVICETYPE_FILE).exists()) {
        return;
    }

    //内容格式："十六进制的厂商ID+产品ID": int设备类型。设备类型与枚举DeviceType对应。例如："3554fc00": 1，含义为"3554fc00"的设备类型为鼠标
    QJsonObject obj = UsdQJsonConf::loadJsonObject(USD_DEVICETYPE_FILE);

    for (const auto& key : obj.keys()) {
        if (!m_devTypeMap.contains(key)) {
            m_devTypeMap.insert(key, (DeviceType)obj.value(key).toInt());
        }
    }
}

void DeviceMonitor::initDeviceInfo()
{
    getDevInfoListFromFile();
    QFile file(PROCBUSINPUTDEVICES);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        SYS_LOG(LOG_WARNING,"open file %s error!", PROCBUSINPUTDEVICES);
        return;
    }

    QString remaining = file.readAll();
    file.close();
    initDeviceTypeFromUsdFile();
    QStringList devicelist = remaining.split("\n\n");
    for (const auto& device : devicelist) {
        //文件使用空行区分不同设备
        if (device.isEmpty()) {
            USD_LOG(LOG_DEBUG,"device msg is empty!!");
            continue;
        }

        DeviceInfo devInfo = getDevInfo(device);
        if (!m_devInfoList.contains(devInfo)) {
            m_devInfoList.append(devInfo);
        }
    }
    setDevTypeMap();
}

DeviceType DeviceMonitor::getDeviceType(uint productId) const
{
    QMapIterator<QString, DeviceType> it(m_devTypeMap);
    while (it.hasNext()) {
        it.next();
        QString productIdStr = it.key();
        if (productIdStr.toUInt(nullptr, 16) == productId) {
            return it.value();
        }
    }
    return DeviceType::IN_NONE;
}

QString DeviceMonitor::getDeviceTypeMap() const
{
    QString info;
    QMapIterator<QString, DeviceType> it(m_devTypeMap);
    while ( it.hasNext()) {
        it.next();
        info += QString("%1: %2 \n").arg(it.key()).arg((uint)it.value());
    }
    info += QString("map size is %1").arg(m_devTypeMap.count());

    return info;
}
