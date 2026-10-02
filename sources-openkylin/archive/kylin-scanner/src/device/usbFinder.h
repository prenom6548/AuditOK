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

#ifndef UNTITLED6_USBFINDER_H
#define UNTITLED6_USBFINDER_H

#include "device_information.h"

struct device_id_s {
    QString full_device_id;
    QString mfg;
    QString mdl;
    QString sern;
    QString cls;
};

void
parse_device_id (const char *device_id, struct device_id_s *id);


class usbFinder {
public:
    usbFinder();
    ~usbFinder();
    void dowork();
    QList<DeviceInformation> getList();
private:
    QList<DeviceInformation> m_usbInfoList;
};


#endif//UNTITLED6_USBFINDER_H
