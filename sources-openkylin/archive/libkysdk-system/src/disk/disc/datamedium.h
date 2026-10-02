/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#ifndef DATAMEDIUM_H
#define DATAMEDIUM_H

#include "toc.h"
#include "msf.h"
#include "devicetypes.h"
#include "deviceglobals.h"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

class DataCDROMImpl;

class DataMedium
{
public:
    explicit DataMedium(std::string mediuType, std::string blockName, void *parent);
    DataMedium(std::string blockName, void *parent);
    ~DataMedium();

public:
    bool isErasable() const { return m_bErasable; }
    bool isEnableUDF() const { return m_bEnableUDF; }
    std::string getCapacity() const { return m_oCapacity; }
    unsigned long get32Capacity() const { return m_u64Capacity; }
    std::string getUsedCapacity() const { return m_oUsedCapacity; }
    unsigned long get32UsedCapacity() const { return m_u64UsedCapacity; }
    std::string getFreeCapacity() const { return m_oFreeCapacity; }
    unsigned long get32FreeCapacity() const { return m_u64FreeCapacity; }

    Msf getMsfCapacity() const;
    Msf getMsfUsedCapacity() const;
    Msf getMsfFreeCapacity() const;

    std::string blockDeviceName() const;

    std::string display() const { return m_oDisplay; }
    std::string getFileSystemType();
    std::string getType() const { return m_oMediumType; }
    std::string getBlockName() const { return m_oBlockName; }
    void setUsedSize(unsigned long s) { m_u64UsedCapacity = s; }

    Device::MediaState diskState() const;
    bool empty() const;
    bool appendable() const;
    bool rewritable() const;
    int numSessions() const;
    int numTracks() const;
    Device::MediaType mediaType() const;
    Device::MediaState lastSessionState() const;
    Device::BackGroundFormattingState bgFormatState() const;

public:
    void diskInfo();
    void capacity();

    std::string msfCapacity();
    std::string msfUsedCapacity();
    std::string msfFreeCapacity();

private:
    void mountPoint();
    std::string toUnitString(unsigned long size64);

private:
    unsigned char m_u8Status;
    bool m_bErasable;
    bool m_bUDF;
    // DVD光盘 且 空状态
    // m_bEnableUDF为真表示可以使用UDF格式刻录
    bool m_bEnableUDF;
    DataCDROMImpl *m_oCDROM;
    unsigned int m_u32TrackNumber;
    unsigned long m_u64UsedCapacity;
    unsigned long m_u64FreeCapacity;
    unsigned long m_u64Capacity;
    std::string m_oMountPoint;
    std::string m_oVolumnID;
    std::string m_oBlockName;
    std::string m_oMediumType;
    std::string m_oUsedCapacity;
    std::string m_oFreeCapacity;
    std::string m_oCapacity;
    std::string m_oDisplay;
    //    DiscRead        *m_discread;
    Msf m_msfCapacity;
    Msf m_msfUsedCapacity;
    Msf m_firstLayerSize;

    int m_currentProfile;
    Device::MediaState m_diskState;
    Device::MediaState m_lastSessionState;
    Device::BackGroundFormattingState m_bgFormatState;
    Device::MediaType m_mediaType;

    int m_numSessions;
    int m_numTracks;
    int m_numLayers; // only for DVD media
    int m_rewritable;
};

#endif // DATAMEDIUM_H
