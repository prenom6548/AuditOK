/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "device.h"

namespace KInstaller {

namespace Partman {
Device::Device():
m_model(""),
m_osName(""),
m_path(""),
m_sectorSize(0),
m_sectors(0),
 m_diskFreeSpace(0),
  m_readyOnly(0),
  m_partTableType(PartTableType::Empty)
{

}

QString Device::getDevicePath()
{
    if(!m_path.isEmpty()) {
        return m_path;
    }
}

qint64 Device::getByteLength()
{
    if(m_length > 0) {
        m_diskTotal = m_length * m_sectorSize;
        return m_diskTotal;
    } else {
        return -1;
    }
}

qint64 Device::getDiskUsedPercent()
{
    m_diskFreeSpace = 0;
    for(int i = 0; i < partitions.length(); i++) {
        m_diskFreeSpace += partitions.at(i)->m_freespace;
    }

    return m_diskFreeSpace;
}

int deviceIndex(const DeviceList &devices, const QString &devPath)
{
    for(int i = 0; i < devices.length(); i++) {
        if(devices.at(i)->m_path == devPath) {
            return i;
        }
    }
    return -1;
}
}

}
