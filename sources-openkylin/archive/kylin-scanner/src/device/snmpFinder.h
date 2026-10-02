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

#ifndef DEVICE_SNMPFINDER_H
#define DEVICE_SNMPFINDER_H
#include "device_information.h"

class snmpFinder {
public:
    snmpFinder();
    ~snmpFinder();
    void dowork();
    QList<DeviceInformation> getList();



private:
    QList<DeviceInformation> m_snmpInfoList;
    void parseSnmpInfo(QString &info);
};

#endif  // DEVICE_SNMPFINDER_H
