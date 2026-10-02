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

#ifndef DEVICEMONITOR_H
#define DEVICEMONITOR_H

#include <QObject>
#include <QVariant>
#include "input-common.h"

typedef QMap<QString, QString> DeviceInfo;

class DeviceMonitor : public QObject
{
    Q_OBJECT
public:
    static DeviceMonitor* instance();
    void initDeviceInfo();

    DeviceType getDeviceType(uint productId) const;

    QString getDeviceTypeMap() const;

Q_SIGNALS:

private:
    explicit DeviceMonitor(QObject *parent = nullptr);

    DeviceType getTypeByBluetoothMac(const QString &mac) const;
    DeviceType getTypeByBluetooth(const DeviceInfo &devInfo) const;

    bool isMacAdress(const QString &str) const;

    DeviceInfo getDevInfo(const QString &devStr) const;
    void setDevTypeMap();

    void getDevInfoListFromFile();
    DeviceType getDevTypeFromDevInfo(const QString &prodId) const;

    DeviceType transEvToDeviceType(const QString &ev) const;

    void initDeviceTypeFromUsdFile();

private:
    QList<DeviceInfo> m_devInfoList;
    QMap<QString, DeviceType> m_devTypeMap;
};

#endif // DEVICEMONITOR_H
