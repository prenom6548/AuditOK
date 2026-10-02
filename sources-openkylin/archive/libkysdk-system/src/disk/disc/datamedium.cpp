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

#include "datamedium.h"
#include "../cdrom/datacdromimpl.h"
#include "mmc.h"
#include "../libisofs/iso9660.h"
#include "../libisofs/iso9660backend.h"

#include <fstream>
#include <cstring>
#include <algorithm> // for std::transform
#include <cctype>    // for std::tolower
#include <sys/statvfs.h>

DataMedium::DataMedium(std::string mediuType, std::string blockName, void *parent)
{
    m_oBlockName = blockName;
    m_oMediumType = mediuType;
    m_bErasable = false;
    m_u64Capacity = 0;
    m_u64FreeCapacity = 0;
    m_u32TrackNumber = 0;
    m_u64UsedCapacity = 0;
    m_oCDROM = (DataCDROMImpl *)parent;
    m_oCapacity = std::to_string(0.00) + "B";
    m_oUsedCapacity = std::to_string(0.00) + "B";
    m_oFreeCapacity = std::to_string(0.00) + "B";
}

DataMedium::DataMedium(std::string blockName, void *parent)
{
    m_oBlockName = blockName;
    m_bErasable = false;
    m_u64Capacity = 0;
    m_u64FreeCapacity = 0;
    m_u32TrackNumber = 0;
    m_u64UsedCapacity = 0;
    m_oCDROM = (DataCDROMImpl *)parent;
    m_oCapacity = std::to_string(0.00) + "B";
    m_oUsedCapacity = std::to_string(0.00) + "B";
    m_oFreeCapacity = std::to_string(0.00) + "B";
}

DataMedium::~DataMedium()
{
}

Msf DataMedium::getMsfCapacity() const
{
    return m_msfCapacity;
}

Msf DataMedium::getMsfUsedCapacity() const
{
    return m_msfUsedCapacity;
}

Msf DataMedium::getMsfFreeCapacity() const
{
    bool needPrimaryDescrptorCalculate = mediaType() & ( Device::MEDIA_DVD_PLUS_RW|Device::MEDIA_DVD_RW_OVWR|Device::MEDIA_BD_RE).toInt();
    if (!m_bUDF && (diskState() == Device::STATE_COMPLETE) && !needPrimaryDescrptorCalculate) {
        return Msf();
    }
    return m_msfCapacity - m_msfUsedCapacity;
}

std::string DataMedium::blockDeviceName() const
{
    return m_oBlockName;
}

Device::MediaState DataMedium::diskState() const
{
    return m_diskState;
}

bool DataMedium::empty() const
{
    return diskState() == Device::STATE_EMPTY;
}

bool DataMedium::appendable() const
{
    return diskState() == Device::STATE_INCOMPLETE;
}

bool DataMedium::rewritable() const
{
    return m_rewritable;
}

int DataMedium::numSessions() const
{
    return m_numSessions;
}

int DataMedium::numTracks() const
{
    return m_numTracks;
}

Device::MediaType DataMedium::mediaType() const
{
    return m_mediaType;
}

Device::MediaState DataMedium::lastSessionState() const
{
    return m_lastSessionState;
}

Device::BackGroundFormattingState DataMedium::bgFormatState() const
{
    return m_bgFormatState;
}

void DataMedium::diskInfo()
{
    if (m_oCDROM->open())
    {
        std::vector<unsigned char> data;
        if (!m_oCDROM->testUnitReady())
        {
            m_diskState = Device::STATE_NO_MEDIA;
            m_mediaType = Device::MEDIA_NONE;
            m_currentProfile = Device::MEDIA_NONE;
        }
        else
            m_currentProfile = m_oCDROM->currentProfile();

        if (diskState() != Device::STATE_NO_MEDIA)
        {
            if (m_oCDROM->readDiscInformation(data))
            {
                disc_info_t *dInf = (disc_info_t *)data.data();
                switch (dInf->status)
                {
                case 0:
                    m_diskState = Device::STATE_EMPTY;
                    break;
                case 1:
                    m_diskState = Device::STATE_INCOMPLETE;
                    break;
                case 2:
                    m_diskState = Device::STATE_COMPLETE;
                    break;
                default:
                    m_diskState = Device::STATE_UNKNOWN;
                    break;
                }
                switch (dInf->border)
                {
                case 0:
                    m_lastSessionState = Device::STATE_EMPTY;
                    break;
                case 1:
                    m_lastSessionState = Device::STATE_INCOMPLETE;
                    break;
                case 2:
                    m_lastSessionState = Device::STATE_COMPLETE;
                    break;
                default:
                    m_lastSessionState = Device::STATE_UNKNOWN;
                    break;
                }

                switch (dInf->bg_f_status & 0x3)
                {
                case 0x0:
                    m_bgFormatState = Device::BG_FORMAT_NONE;
                    break;
                case 0x1:
                    m_bgFormatState = Device::BG_FORMAT_INCOMPLETE;
                    break;
                case 0x2:
                    m_bgFormatState = Device::BG_FORMAT_IN_PROGRESS;
                    break;
                case 0x3:
                    m_bgFormatState = Device::BG_FORMAT_COMPLETE;
                    break;
                }

                m_numTracks = (dInf->last_track_l & 0xff) | (dInf->last_track_m << 8 & 0xff00);
                if (diskState() == Device::STATE_EMPTY)
                    m_numTracks = 0;
                else if (diskState() == Device::STATE_INCOMPLETE)
                    m_numTracks--;

                m_rewritable = dInf->erasable;

                //
                // This is the Last Possible Lead-Out Start Address in HMSF format
                // This is only valid for CD-R(W) and DVD+R media.
                // For complete media this shall be filled with 0xff
                //
                if (dInf->lead_out_m != 0xff &&
                    dInf->lead_out_r != 0xff &&
                    dInf->lead_out_s != 0xff &&
                    dInf->lead_out_f != 0xff)
                    m_msfCapacity = Msf(dInf->lead_out_m + dInf->lead_out_r * 60,
                                        dInf->lead_out_s,
                                        dInf->lead_out_f) -
                                    150;

                //
                // This is the position where the next Session shall be recorded in HMSF format
                // This is only valid for CD-R(W) and DVD+R media.
                // For complete media this shall be filled with 0xff
                //
                if (dInf->lead_in_m != 0xff &&
                    dInf->lead_in_r != 0xff &&
                    dInf->lead_in_s != 0xff &&
                    dInf->lead_in_f != 0xff)
                    m_msfUsedCapacity = Msf(dInf->lead_in_m + dInf->lead_in_r * 60,
                                            dInf->lead_in_s,
                                            dInf->lead_in_f) -
                                        4500;
            }
            else
            {
                std::cout << "(Device::Device) " << blockDeviceName()
                          << " fabricating disk information for a stupid device." << std::endl;
                Toc toc = m_oCDROM->readToc();
                if (!toc.isEmpty())
                {
                    m_diskState = Device::STATE_COMPLETE;
                    m_lastSessionState = Device::STATE_COMPLETE;
                    m_numTracks = toc.count();
                    m_msfCapacity = m_msfUsedCapacity = toc.length();
                }
            }

            //
            // The mediatype needs to be set
            //
            m_mediaType = m_oCDROM->mediaType();

            // At least some Plextor drives return profile NONE for CD media
            // or CD_ROM for writable media
            if (m_mediaType & (Device::MEDIA_UNKNOWN | Device::MEDIA_NONE | Device::MEDIA_CD_ROM).toInt())
            {
                // probably it is a CD
                if (rewritable())
                    m_mediaType = Device::MEDIA_CD_RW;
                else if (empty() || appendable())
                    m_mediaType = Device::MEDIA_CD_R;
                else
                    m_mediaType = Device::MEDIA_CD_ROM;
            }

            m_oMediumType = Device::mediaTypeString(m_mediaType);

            if (m_mediaType & Device::MEDIA_DVD_ALL)
            {
                if (m_oCDROM->readDvdStructure(data))
                {
                    // some debugging stuff
                    Msf sda, eda, ea0;
                    sda = (data[4 + 5] << 16 | data[4 + 6] << 8 | data[4 + 7]);
                    eda = (data[4 + 9] << 16 | data[4 + 10] << 8 | data[4 + 11]);
                    ea0 = (data[4 + 13] << 16 | data[4 + 14] << 8 | data[4 + 15]);

                    std::cout << "First sec data area: " << sda.toString()
                              << " (LBA " << std::to_string(sda.lba())
                              << ") (" << std::to_string(sda.mode1Bytes()) << std::endl;
                    std::cout << "Last sec data area: " << eda.toString()
                              << " (LBA " << std::to_string(eda.lba())
                              << ") (" << std::to_string(eda.mode1Bytes()) << " Bytes)" << std::endl;
                    std::cout << "Last sec layer 1: " << ea0.toString()
                              << " (LBA " << std::to_string(ea0.lba())
                              << ") (" << std::to_string(ea0.mode1Bytes()) << " Bytes)" << std::endl;

                    Msf da0 = ea0 - sda + 1;
                    Msf da1 = eda - ea0;
                    std::cout << "Layer 1 length: " << da0.toString()
                              << " (LBA " << std::to_string(da0.lba())
                              << ") (" << std::to_string(da0.mode1Bytes()) << " Bytes)" << std::endl;
                    std::cout << "Layer 2 length: " << da1.toString()
                              << " (LBA " << std::to_string(da1.lba())
                              << ") (" << std::to_string(da1.mode1Bytes()) << " Bytes)" << std::endl;

                    m_numLayers = ((data[6] & 0x60) == 0 ? 1 : 2);

                    bool otp = (data[4 + 2] & 0xF);

                    // ea0 is 0 if the medium does not use Opposite track path
                    if (otp && ea0 > 0)
                        m_firstLayerSize = da0;
                    else
                        m_firstLayerSize = 0;
                }
                else
                {
                    std::cout << "(Device::Device) Unable to read DVD structure for num of layers." << std::endl;
                    m_numLayers = ((m_mediaType & Device::MEDIA_WRITABLE_DVD_DL) ? 2 : 1);
                }
            }

            //
            // Number of sessions for non-empty disks
            //
            if (diskState() != Device::STATE_EMPTY)
            {
                int sessions = numSessions();
                if (sessions >= 0)
                    m_numSessions = sessions;
                else
                    std::cout << "(Device::Device) could not get session info via READ TOC/PMA/ATIP." << std::endl;
            }
            else
                m_numSessions = 0;

            // inf.d->mediaId = mediaId(inf.mediaType());

            //
            // Now we determine the size:

            // for all empty and appendable media READ FORMAT CAPACITIES should return the proper unformatted size
            // for complete disks we may use the READ_device() command or the start sector from the leadout
            //
            int media = m_oCDROM->mediaType();
            //
            // Use the profile if available because DVD-ROM units need to treat DVD+-R(W) media as DVD-ROM
            // if supported at all
            //
            if (m_oCDROM->currentProfile() == Device::MEDIA_DVD_ROM)
                media = Device::MEDIA_DVD_ROM;

            switch (media)
            {
            case Device::MEDIA_CD_R:
            case Device::MEDIA_CD_RW:
                if (m_msfCapacity == 0)
                {
                    if (m_oCDROM->readTocPmaAtip(data, 0x4, true, 0))
                    {
                        struct atip_descriptor *atip = (struct atip_descriptor *)data.data();
                        if (data.size() >= 11)
                        {
                            m_msfCapacity = Msf(atip->lead_out_m, atip->lead_out_s, atip->lead_out_f) - 150;
                            Device::debugBitfield(&atip->lead_out_m, 3);
                            std::cout << blockDeviceName() << ": ATIP capacity: " << m_msfCapacity.toString();
                        }
                    }
                }

                //
                // for empty and appendable media capacity and usedCapacity should be filled in from
                // diskinfo above. If not they are both still 0
                //
                if (m_msfCapacity != 0 && (diskState() == Device::STATE_EMPTY || m_msfUsedCapacity != 0))
                {
                    break;
                }

            default:
            case Device::MEDIA_CD_ROM:
                if (m_msfCapacity > 0 && m_msfUsedCapacity == 0)
                    m_msfUsedCapacity = m_msfCapacity;

                if (m_msfUsedCapacity == 0)
                {
                    Msf readCap;
                    if (m_oCDROM->readCapacity(readCap))
                    {
                        std::cout << "(Device::Device) READ CAPACITY: " << readCap.toString()
                                  << " other capacity: " << m_msfCapacity.toString() << std::endl;
                        //
                        // READ CAPACITY returns the last written sector
                        // that means the size is actually readCap + 1
                        //
                        m_msfUsedCapacity = readCap + 1;
                    }
                    else
                    {
                        std::cout << "(Device::Device) " << blockDeviceName()
                                  << " Falling back to readToc for capacity." << std::endl;
                        m_msfUsedCapacity = m_oCDROM->readToc().length();
                    }
                }
            case Device::MEDIA_DVD_ROM:
            {
                Msf readCap;
                if (m_oCDROM->readCapacity(readCap))
                {
                    std::cout << "(Device::Device) READ CAPACITY: " << readCap.toString()
                              << " other capacity: " << m_msfCapacity.toString() << std::endl;
                    //
                    // READ CAPACITY returns the last written sector
                    // that means the size is actually readCap + 1
                    //
                    m_msfUsedCapacity = readCap + 1;
                }
                else
                {
                    //
                    // Only one track, use it's size
                    //
                    if (m_oCDROM->readTrackInformation(data, 0x1, 0x1))
                    {
                        track_info_t *trackInfo = (track_info_t *)data.data();
                        m_msfUsedCapacity = Device::from4Byte(trackInfo->track_size);
                    }
                    else
                        std::cout << "(Device::Device) " << blockDeviceName()
                                  << "READ TRACK INFORMATION for DVD-ROM failed." << std::endl;
                }

                break;
            }

            case Device::MEDIA_DVD_PLUS_R:
            case Device::MEDIA_DVD_PLUS_R_DL:
                if (appendable() || empty())
                {
                    //
                    // get remaining space via the invisible track
                    //
                    if (m_oCDROM->readTrackInformation(data, 0x1, /*0xff*/ numTracks() + 1))
                    {
                        track_info_t *trackInfo = (track_info_t *)data.data();
                        m_msfUsedCapacity = Device::from4Byte(trackInfo->track_start);
                        m_msfCapacity = Device::from4Byte(trackInfo->track_start) + Device::from4Byte(trackInfo->track_size);
                    }
                }
                else
                {
                    if (m_oCDROM->readTrackInformation(data, 0x1, numTracks()))
                    {
                        track_info_t *trackInfo = (track_info_t *)data.data();
                        m_msfCapacity = m_msfUsedCapacity = Device::from4Byte(trackInfo->track_start) + Device::from4Byte(trackInfo->track_size);
                    }
                }
                break;

            case Device::MEDIA_DVD_R:
            case Device::MEDIA_DVD_R_SEQ:
            case Device::MEDIA_DVD_R_DL:
            case Device::MEDIA_DVD_R_DL_JUMP:
            case Device::MEDIA_DVD_R_DL_SEQ:
                //
                // get data from the incomplete track (which is NOT the invisible track 0xff)
                // This will fail in case the media is complete!
                //
                if (m_oCDROM->readTrackInformation(data, 0x1, numTracks() + 1))
                {
                    track_info_t *trackInfo = (track_info_t *)data.data();
                    m_msfUsedCapacity = Device::from4Byte(trackInfo->track_start);
                    m_msfCapacity = Device::from4Byte(trackInfo->free_blocks) + Device::from4Byte(trackInfo->track_start);
                }

                //
                // Get the "really" used space without border-out
                //
                if (!empty())
                {
                    Msf readCap;
                    if (m_oCDROM->readCapacity(readCap))
                    {
                        //
                        // READ CAPACITY returns the last written sector
                        // that means the size is actually readCap + 1
                        //
                        m_msfUsedCapacity = readCap + 1;
                    }
                    else
                        std::cout << "(Device::Device) " << blockDeviceName()
                                  << " READ CAPACITY for DVD-R failed." << std::endl;
                }

                break;

            case Device::MEDIA_DVD_RW_OVWR:
                m_numSessions = 1;
            case Device::MEDIA_DVD_RW:
            case Device::MEDIA_DVD_RW_SEQ:
                // only one track on a DVD-RW media
                if (m_oCDROM->readTrackInformation(data, 0x1, 0x1))
                {
                    track_info_t *trackInfo = (track_info_t *)data.data();
                    m_msfCapacity = Device::from4Byte(trackInfo->track_size);
                    if (!empty()) {
                        if (m_oCDROM->readFormatCapacity(0x10, m_msfCapacity)) {
                            std::cout << blockDeviceName() << ": Format capacity 0x10: " << m_msfCapacity.toString();
			            }

			            Msf readCap;
                        if (m_oCDROM->readCapacity(readCap)) {
                            std::cout << "(Device::Device) READ CAPACITY: " << readCap.toString()
                                     << " other capacity: " << m_msfCapacity.toString();
                            m_msfUsedCapacity = readCap + 1;
                        } else {
                            std::cout << "(Device::Device) " << blockDeviceName()
                                     << " Falling back to readToc for capacity.";
                            m_msfUsedCapacity = Device::from4Byte(trackInfo->track_size);
                        }
                    }
                }
                break;

            case Device::MEDIA_DVD_PLUS_RW:
            {
                Msf currentMax;
                int currentMaxFormat = 0;
                if (m_oCDROM->readFormatCapacity(0x26, m_msfCapacity, &currentMax, &currentMaxFormat))
                {
                    if (currentMaxFormat == 0x1)
                    { // unformatted or blank media
                        m_msfUsedCapacity = 0;
                        m_msfCapacity = currentMax;
                    }
                    else
                    {
                        m_msfUsedCapacity = currentMax;
                        // Plextor drives tend to screw things up and report invalid values
                        // for the max format capacity of 1.4 GB DVD media
                        if (bgFormatState() == Device::BG_FORMAT_COMPLETE)
                        {
                            m_msfCapacity = currentMax;
                        }
                    }
                }
                else
                    std::cout << "(Device::Device) " << blockDeviceName()
                              << " READ FORMAT CAPACITIES for DVD+RW failed." << std::endl;

                break;
            }
            case Device::MEDIA_BD_R:
            case Device::MEDIA_BD_R_SRM:
            case Device::MEDIA_BD_R_SRM_POW:
            case Device::MEDIA_BD_R_RRM:
                //
                // get the invisible track's first sector
                // or the next writable address of the last open track
                //
                if (m_oCDROM->readDiscInformation(data))
                {
                    int lastTrack = (int)(data[11] << 8 | data[6]);

                    if (m_oCDROM->readTrackInformation(data, 1, lastTrack))
                    {

                        // capacity: last track's start address + last track's size
                        m_msfCapacity = Device::from4Byte(&data[8]) + Device::from4Byte(&data[24]);

                        if (data[6] & 0x80)
                            m_msfUsedCapacity = Device::from4Byte(&data[8]);
                        else if (data[7] & 0x1)
                            m_msfUsedCapacity = Device::from4Byte(&data[12]);
                        else
                            m_msfUsedCapacity = m_msfCapacity;
                    }
                }
                break;

            case Device::MEDIA_BD_RE:
            {
                Msf currentMax;
                int currentMaxFormat = 0;
                if (m_oCDROM->readFormatCapacity(0x00, m_msfCapacity, &currentMax, &currentMaxFormat))
                {
                    if (currentMaxFormat == 0x1)
                    { // unformatted or blank media
                        m_msfUsedCapacity = 0;
                        m_msfCapacity = currentMax;
                    }
                    else
                    {
                        m_msfUsedCapacity = currentMax;
                    }
                }
                else
                    std::cout << "(Device::Device) " << blockDeviceName()
                              << " READ FORMAT CAPACITIES for BD-RE failed." << std::endl;
                break;
            }

            case Device::MEDIA_BD_ROM:
            {
                Msf readCap;
                if (m_oCDROM->readCapacity(readCap))
                {
                    //
                    // READ CAPACITY returns the last written sector
                    // that means the size is actually readCap + 1
                    //
                    m_msfUsedCapacity = readCap + 1;
                }

                break;
            }
            }
        }
    }
    m_oCDROM->close();
}

void DataMedium::capacity()
{
    m_oCapacity = msfCapacity();
    m_u64Capacity = m_msfCapacity.mode1Bytes();

    if (!empty())
    {
        mountPoint();

        if ((mediaType() & Device::MEDIA_DVD_PLUS_RW) && (0 == m_u64Capacity) && m_bUDF)
        {
            struct statvfs vfs;
            // 直接使用挂载点路径（无需"file://"前缀）
            if (statvfs(m_oMountPoint.c_str(), &vfs) == 0)
            {
                // 计算文件系统总容量（块数 * 块大小）
                m_u64Capacity = static_cast<uint64_t>(vfs.f_blocks) * vfs.f_frsize;
                m_msfCapacity = m_u64Capacity; // 保持相同值
                m_oCapacity = toUnitString(m_u64Capacity);
            }
        }

        if (m_bUDF)
        {
            if (mediaType() & (Device::MEDIA_DVD_PLUS_RW | Device::MEDIA_DVD_RW_OVWR | Device::MEDIA_BD_RE | Device::MEDIA_CD_RW).toInt())
            {
                /* UDF 且 DVD+/-RW、CD-RW */
                if (!m_oMountPoint.empty())
                {
                    struct statvfs vfs;
                    if (statvfs(m_oMountPoint.c_str(), &vfs) == 0)
                    {
                        // 计算文件系统总容量（块数 * 块大小）
                        uint64_t totalBlocks = static_cast<uint64_t>(vfs.f_blocks);
                        uint64_t blockSize = vfs.f_frsize;
                        uint64_t totalCapacity = totalBlocks * blockSize;

                        // 计算已用容量（总块数 - 空闲块数） * 块大小
                        uint64_t freeBlocks = static_cast<uint64_t>(vfs.f_bfree);
                        m_u64UsedCapacity = (totalBlocks - freeBlocks) * blockSize;
                    }
                    else
                    {
                        // 处理 statvfs 调用失败的情况
                        m_u64UsedCapacity = 0; // 确保有默认值
                    }
                }
                else
                {
                    m_u64UsedCapacity = 0; // 确保有默认值
                }
                m_oUsedCapacity = toUnitString(m_u64UsedCapacity);
                m_u64FreeCapacity = m_u64Capacity - m_u64UsedCapacity;
                m_oFreeCapacity = toUnitString(m_u64FreeCapacity);

                //                LOGGER->info(QString("UDF2.01格式(gio): 使用：%1(%2) 剩余：%3(%4)")
                //                             .arg(m_u64UsedCapacity).arg(m_oUsedCapacity)
                //                             .arg(m_u64FreeCapacity).arg(m_oFreeCapacity));
            }
            else
            {
                /* UDF1.02 且 CD-R、DVD+/-R(DL)盘 */
                m_oUsedCapacity = msfUsedCapacity();
                m_oFreeCapacity = msfFreeCapacity();
                m_u64UsedCapacity = m_msfUsedCapacity.mode1Bytes();
                m_u64FreeCapacity = m_u64Capacity - m_u64UsedCapacity;
                //                LOGGER->info(QString("UDF1.02格式(Msf): 使用：%1(%2) 剩余：%3(%4)")
                //                             .arg(m_u64UsedCapacity).arg(m_oUsedCapacity)
                //                             .arg(m_u64FreeCapacity).arg(m_oFreeCapacity));
            }
        }
        else
        {
            /* ISO 9660 */
            bool needPrimaryDescrptorCalculate = mediaType() & ( Device::MEDIA_DVD_PLUS_RW|Device::MEDIA_DVD_RW_OVWR|Device::MEDIA_BD_RE).toInt();
            if (needPrimaryDescrptorCalculate)
            {
                /* ISO9660 且 DVD+/-RW、CD-RW */
                Iso9660SimplePrimaryDescriptor isoDesc;
                isoDesc = Iso9660SimplePrimaryDescriptor();
                Iso9660 iso(new Iso9660DeviceBackend(m_oCDROM));
                iso.setStartSector(0);
                iso.setPlainIso9660(true);
                if (iso.open())
                {
                    isoDesc = iso.primaryDescriptor();
                    m_msfUsedCapacity = isoDesc.getVolumSpacesSize();
                }
                else
                {
                    // 没有获取到已使用数据容量，可能是Win刻录的UDF盘或空光盘
                    m_msfUsedCapacity = 0;
                    //                    LOGGER->info(QString("[%1][%2] Failed to obtain the capacity. It may be a real UDF disk").arg(__func__).arg(__LINE__));
                }
            }
            m_oUsedCapacity = msfUsedCapacity();
            m_oFreeCapacity = msfFreeCapacity();
            m_u64UsedCapacity = m_msfUsedCapacity.mode1Bytes();
            if ((diskState() == Device::STATE_COMPLETE) && !needPrimaryDescrptorCalculate) {
                /* ISO9660 + 非DVD+/-RW + 已关盘 */
                if (m_msfCapacity != m_msfUsedCapacity) {
                    m_u64Capacity = m_u64UsedCapacity;
                    m_msfCapacity = m_msfUsedCapacity;
                    m_oCapacity = m_oUsedCapacity;
                }
                m_u64FreeCapacity = 0;
            } else {
                m_u64FreeCapacity = m_u64Capacity - m_u64UsedCapacity;
            }
            //            LOGGER->info(QString("ISO9660格式(Msf): 使用：%1(%2) 剩余：%3(%4)")
            //                         .arg(m_u64UsedCapacity).arg(m_oUsedCapacity)
            //                         .arg(m_u64FreeCapacity).arg(m_oFreeCapacity));
        }
    }
    else
    {
        // 空盘
        m_u64UsedCapacity = 0;
        m_msfUsedCapacity = 0;
        m_u64FreeCapacity = m_u64Capacity - m_u64UsedCapacity;
        m_oFreeCapacity = msfFreeCapacity();
        //        LOGGER->info(QString("空盘(Msf): 使用：%1(%2) 剩余：%3(%4)")
        //                     .arg(m_u64UsedCapacity).arg(m_oUsedCapacity)
        //                     .arg(m_u64FreeCapacity).arg(m_oFreeCapacity));
    }
    if ((0 == m_u64Capacity) || (0 == m_msfCapacity))
    {
        m_u64Capacity = m_u64UsedCapacity;
        m_msfCapacity = m_msfUsedCapacity;
        m_oCapacity = msfCapacity();
        m_oFreeCapacity = msfFreeCapacity();
        m_u64FreeCapacity = m_u64Capacity - m_u64UsedCapacity;
        //        LOGGER->info(QString("光盘总容量为0， 总容量 = 已使用容量(%1)").arg(m_u64UsedCapacity));
        //        LOGGER->info(QString("修正：使用：%1(%2) 剩余：%3(%4)")
        //                     .arg(m_u64UsedCapacity).arg(m_oUsedCapacity)
        //                     .arg(m_u64FreeCapacity).arg(m_oFreeCapacity));
    }
}

std::string DataMedium::msfCapacity()
{
    //    LOGGER->info(QString("MsfCapacity: 总容量：%1  %2 %3")
    //                 .arg(getMsfCapacity().toString()
    //                      .arg(getMsfCapacity().mode1Bytes()))
    //                 .arg(toUnitString(getMsfCapacity().mode1Bytes())));
    return toUnitString(getMsfCapacity().mode1Bytes());
}

std::string DataMedium::msfUsedCapacity()
{
    //    LOGGER->info(QString("msfUsedCapacity: 已使用容量：%1  %2 %3")
    //                 .arg(getMsfUsedCapacity().toString()
    //                      .arg(getMsfUsedCapacity().mode1Bytes()))
    //                 .arg(toUnitString(getMsfUsedCapacity().mode1Bytes())));
    return toUnitString(getMsfUsedCapacity().mode1Bytes());
}

std::string DataMedium::msfFreeCapacity()
{
    //    LOGGER->info(QString("msfFreeCapacity: 剩余容量：%1  %2 %3")
    //                 .arg(getMsfFreeCapacity().toString()
    //                      .arg(getMsfFreeCapacity().mode1Bytes()))
    //                 .arg(toUnitString(getMsfFreeCapacity().mode1Bytes())));
    return toUnitString(getMsfFreeCapacity().mode1Bytes());
}

std::string DataMedium::toUnitString(unsigned long size64)
{
    double size = static_cast<double>(size64);
    int u = -1;
    const char *units[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB"}; // 支持到EB
    const int maxUnitIndex = sizeof(units) / sizeof(units[0]) - 1;

    // 循环除以1024直到size<=1024或超过最大单位
    while (size > 1024.0 && u < maxUnitIndex)
    {
        ++u;
        size /= 1024.0;
    }

    // 确定单位索引（保护数组边界）
    int unitIndex = (u == -1) ? 0 : (u + 1);
    if (unitIndex > maxUnitIndex)
    {
        unitIndex = maxUnitIndex;
    }

    // 格式化数字为字符串（保留1位小数）
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size;

    // 删除多余的".0"（可选优化）
    std::string numStr = oss.str();
    size_t pos = numStr.find(".0");
    if (pos != std::string::npos && pos == numStr.size() - 2)
    {
        numStr.erase(pos, 2);
    }

    return numStr + " " + units[unitIndex];
}

void DataMedium::mountPoint()
{
    std::ifstream mounts("/proc/mounts");
    if (!mounts.is_open())
    {
        return;
    }

    int cnt = 100;
    std::string line;
    while (std::getline(mounts, line) && cnt-- > 0)
    {
        // 解析每行：设备名 挂载点 文件系统类型 ...
        std::istringstream iss(line);
        std::string device, mountPoint, fsType;
        iss >> device >> mountPoint >> fsType;

        if (device == m_oBlockName)
        {
            m_oMountPoint = mountPoint;

            // 转换为小写进行比较
            std::string fsTypeLower = fsType;
            std::transform(fsTypeLower.begin(), fsTypeLower.end(), fsTypeLower.begin(),
                           [](unsigned char c)
                           { return std::tolower(c); });

            if (fsTypeLower.find("udf") != std::string::npos)
            {
                m_bUDF = true;
                // LOGGER->info("Medium " + m_oMountPoint + " mounted by UDF");
            }
            else if (fsTypeLower.find("iso") != std::string::npos)
            {
                m_bUDF = false;
                // LOGGER->info("Medium " + m_oMountPoint + " mounted by ISO9660");
            }
            else
            {
                // LOGGER->info("Medium " + m_oMountPoint + " mounted, Type: " + fsType);
            }
            break;
        }
    }
}
