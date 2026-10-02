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

#include "datacdromimpl.h"
#include "../disc/discscsi.h"
#include "../disc/deviceglobals.h"
#include "../disc/mmc.h"

DataCDROMImpl::DataCDROMImpl(std::string &blockName)
{
    m_oBlockName = blockName;
    m_iHandle = -1;
    m_oMediumType.clear();

    m_u32MediumRSupport = 0;
    m_u32MediumWSupport = 0;

    m_u32TrackNumber = 0;
    m_u64UsedCapacity = 0;
    m_u64FreeCapacity = 0;
    m_u64Capacity = 0;

    m_oMedium = new DataMedium(blockName, this);
}

DataCDROMImpl::~DataCDROMImpl()
{
    // Ensure device handle is closed in case callers missed close()
    close();
    if (m_oMedium)
    {
        delete m_oMedium;
    }
}

unsigned long DataCDROMImpl::getCDROMCapacity()
{
    return m_oMedium->get32Capacity();
}

unsigned long DataCDROMImpl::getCDROMUsedCapacity()
{
    return m_oMedium->get32UsedCapacity();
}

void DataCDROMImpl::calculateDiscCapacity()
{
    m_oMedium->diskInfo();
    m_oMedium->capacity();
}

std::string DataCDROMImpl::getDiscType()
{
    return Device::mediaTypeString(mediaType());
}

std::string DataCDROMImpl::getCDROMType()
{
    return m_oMedium->getType();
}

std::string DataCDROMImpl::device() const
{
    return m_oBlockName;
}

bool DataCDROMImpl::isOpen()
{
    return m_iHandle >= 0;
}

bool DataCDROMImpl::open()
{
    if (-1 != m_iHandle)
    {
        return true;
    }

    m_iHandle = ::open(m_oBlockName.c_str(), O_NONBLOCK | O_RDONLY | O_CLOEXEC);
    if (m_iHandle < 0)
    {
        return false;
    }

    // Fallback: ensure CLOEXEC even if O_CLOEXEC was ignored
    int fdFlags = fcntl(m_iHandle, F_GETFD);
    if (fdFlags != -1)
        fcntl(m_iHandle, F_SETFD, fdFlags | FD_CLOEXEC);

    return true;
}

void DataCDROMImpl::close()
{
    if (m_iHandle >= 0)
        ::close(m_iHandle);
    m_iHandle = -1;
}

bool DataCDROMImpl::execSCSI(int fd, const unsigned char *cdb, const int cdbLength, unsigned char *result, const int resultLength)
{
    sg_io_hdr_t sgio;

    if (!cdb || !result || resultLength < 1)
        return false;
    memset(&sgio, 0, sizeof(sg_io_hdr_t));
    sgio.interface_id = 'S';
    sgio.dxfer_direction = SG_DXFER_FROM_DEV;
    sgio.flags = SG_FLAG_LUN_INHIBIT;
    sgio.cmdp = (unsigned char *)cdb;
    sgio.cmd_len = cdbLength;
    sgio.dxferp = result;
    sgio.dxfer_len = resultLength;

    if (0 != ioctl(fd, SG_IO, &sgio))
    {
        // LOGGER->error(_("Do SCSI CDB [%1] failed.%2").arg(cdb[0]).arg(strLOGGER->error(errno)));
        return false;
    }
    return true;
}

bool DataCDROMImpl::testUnitReady()
{
    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_TEST_UNIT_READY;
    cmd[5] = 0; // Necessary to set the proper command length
    return (0 == cmd.transport());
}

int DataCDROMImpl::currentProfile()
{
    unsigned char profileBuf[8];
    ::memset(profileBuf, 0, 8);

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_GET_CONFIGURATION;
    cmd[1] = 1;
    cmd[8] = 8;
    cmd[9] = 0; // Necessary to set the proper command length

    if (0 != cmd.transport(TR_DIR_READ, profileBuf, 8))
    {
        return Device::MEDIA_UNKNOWN;
    }

    short profile = Device::from2Byte(&profileBuf[6]);

    //
    // Plextor drives might not set a current profile
    // In that case we get the list of all current profiles
    // and simply use the first one in that list.
    //
    if (profile == 0x00)
    {
        std::vector<unsigned char> data;
        if (getFeature(data, Device::FEATURE_PROFILE_LIST))
        {
            if(data.size() >= 11) {
                int featureLen(data[11]);
                for (int j = 0; j < featureLen; j += 4)
                {
                    // use the first current profile we encounter
                    if (data[12 + j + 2] & 0x1)
                    {
                        profile = Device::from2Byte(&data[12 + j]);
                        break;
                    }
                }
            } else {
                profile = 0x1;
            }
        }
    }

    switch (profile)
    {
    case 0x00:
        return Device::MEDIA_NONE;
    case 0x08:
        return Device::MEDIA_CD_ROM;
    case 0x09:
        return Device::MEDIA_CD_R;
    case 0x0A:
        return Device::MEDIA_CD_RW;
    case 0x10:
        return Device::MEDIA_DVD_ROM;
    case 0x11:
        return Device::MEDIA_DVD_R_SEQ;
    case 0x12:
        return Device::MEDIA_DVD_RAM;
    case 0x13:
        return Device::MEDIA_DVD_RW_OVWR;
    case 0x14:
        return Device::MEDIA_DVD_RW_SEQ;
    case 0x15:
        return Device::MEDIA_DVD_R_DL_SEQ;
    case 0x16:
        return Device::MEDIA_DVD_R_DL_JUMP;
    case 0x1A:
        return Device::MEDIA_DVD_PLUS_RW;
    case 0x1B:
        return Device::MEDIA_DVD_PLUS_R;
    case 0x2B:
        return Device::MEDIA_DVD_PLUS_R_DL;
    case 0x40:
        return Device::MEDIA_BD_ROM;
    case 0x41:
    {
        if (featureCurrent(Device::FEATURE_BD_PSEUDO_OVERWRITE) == 1)
            return Device::MEDIA_BD_R_SRM_POW;
        else
            return Device::MEDIA_BD_R_SRM;
    }
    case 0x42:
        return Device::MEDIA_BD_R_RRM;
    case 0x43:
        return Device::MEDIA_BD_RE;
    case 0x50:
        return Device::MEDIA_HD_DVD_ROM;
    case 0x51:
        return Device::MEDIA_HD_DVD_R;
    case 0x52:
        return Device::MEDIA_HD_DVD_RAM;
    default:
        return Device::MEDIA_UNKNOWN;
    }
}

int DataCDROMImpl::featureCurrent(unsigned int feature)
{
    std::vector<unsigned char> data;
    if (getFeature(data, feature))
    {
        int ret = -1;
        if (data.size() >= 11)
            ret = (data[8 + 2] & 1 ? 1 : 0); // check the current flag

        return ret;
    }
    else
        return -1;
}

bool DataCDROMImpl::getFeature(std::vector<unsigned char> &data, unsigned int feature)
{
    unsigned char header[2048];
    ::memset(header, 0, 2048);

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_GET_CONFIGURATION;
    cmd[1] = 2; // read only specified feature
    cmd[2] = feature >> 8;
    cmd[3] = feature;
    cmd[8] = 8; // we only read the data length first
    cmd[9] = 0; // Necessary to set the proper command length

    // we only read the data length first
    unsigned int dataLen = 8;
    if (0 == cmd.transport(TR_DIR_READ, header, 8))
        dataLen = Device::from4Byte(header) + 4;
    else
        std::cout << "(Device::Device) " << device() << ": GET CONFIGURATION length det failed." << std::endl;

    //
    // Some buggy firmwares do not return the size of the available data
    // but the returned data or something invalid altogether.
    // So we simply use the maximum possible value to be on the safe side
    // with these buggy drives.
    // We cannot use this as default since many firmwares fail with a too high data length.
    //
    if ((dataLen - 8) % 8 || dataLen <= 8)
        dataLen = 0xFFFF;

    // again with real length
    data.resize(dataLen);
    ::memset(data.data(), 0, data.size());

    cmd[7] = data.size() >> 8;
    cmd[8] = data.size();
    if (0 == cmd.transport(TR_DIR_READ, data.data(), data.size()))
    {
        data.resize(KYSDKUdfBurn::min(data.size(), (size_t)Device::from4Byte(data.data()) + 4));
        return true;
    }
    else
    {
        std::cout << "(Device::Device) " << device() << ": GET CONFIGURATION with real length " << data.size() << " failed." << std::endl;
        data.clear();
        return false;
    }
}

bool DataCDROMImpl::readDiscInformation(std::vector<unsigned char> &data)
{
    unsigned char header[2];
    ::memset(header, 0, 2);

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_READ_DISC_INFORMATION;
    cmd[8] = 2;
    cmd[9] = 0; // Necessary to set the proper command length

    unsigned int dataLen = 0;
    if (0 == cmd.transport(TR_DIR_READ, header, 2))
        dataLen = Device::from2Byte(header) + 2u;
    else
        std::cout << "(Device::Device) " << device() << ": READ DISC INFORMATION length det failed" << std::endl;

    if (dataLen < 32)
    {
        std::cout << "(Device::Device) " << device() << ": Device reports bogus disc information length of " << dataLen << std::endl;
        dataLen = 32;
    }

    data.resize(dataLen);
    ::memset(data.data(), 0, data.size());

    cmd[7] = data.size() >> 8;
    cmd[8] = data.size();
    if (0 == cmd.transport(TR_DIR_READ, data.data(), data.size()))
    {
        data.resize(KYSDKUdfBurn::min(data.size(), (size_t)Device::from2Byte(data.data()) + 2));
        return true;
    }
    else
    {
        std::cout << "(Device::Device) " << device() << ": READ DISC INFORMATION with real length " << dataLen << " failed." << std::endl;
        data.clear();
        return false;
    }
}

bool DataCDROMImpl::readDvdStructure(std::vector<unsigned char> &data, unsigned int format, unsigned int layer, unsigned long address, unsigned int agid)
{
    return readDiscStructure(data, 0x0, format, layer, address, agid);
}

bool DataCDROMImpl::readDiscStructure(std::vector<unsigned char> &data, unsigned int mediaType, unsigned int format, unsigned int layer, unsigned long address, unsigned int agid)
{
    unsigned char header[4];
    ::memset(header, 0, 4);

    DiscScsi cmd(m_oBlockName);

    cmd[0] = MMC_READ_DVD_STRUCTURE;
    cmd[1] = mediaType & 0xF;
    cmd[2] = address >> 24;
    cmd[3] = address >> 16;
    cmd[4] = address >> 8;
    cmd[5] = address;
    cmd[6] = layer;
    cmd[7] = format;
    cmd[10] = (agid << 6);
    cmd[11] = 0; // Necessary to set the proper command length

    cmd[9] = 4;
    if (0 == cmd.transport(TR_DIR_READ, header, 4))
    {
        // again with real length
        unsigned int dataLen = Device::from2Byte(header) + 2;

        data.resize(dataLen);
        ::memset(data.data(), 0, data.size());

        cmd[8] = data.size() >> 8;
        cmd[9] = data.size();
        if (cmd.transport(TR_DIR_READ, data.data(), data.size()) == 0)
        {
            data.resize(KYSDKUdfBurn::min(data.size(), (size_t)Device::from2Byte(data.data()) + 2));
            return true;
        }
        else
        {
            std::cout << "(Device::Device) " << device() << ": READ DVD STRUCTURE with real length failed." << std::endl;
            data.clear();
            return false;
        }
    }
    else
    {
        std::cout << "(Device::Device) " << device() << ": READ DVD STRUCTURE length det failed" << std::endl;
        return false;
    }
}

bool DataCDROMImpl::readTocPmaAtip(std::vector<unsigned char> &data, int format, bool msf, int track)
{
    unsigned int descLen = 0;

    switch (format)
    {
    case 0x0:
        descLen = 8;
        break;
    case 0x1:
        descLen = 8;
        break;
    case 0x2:
        descLen = 11;
        break;
    case 0x3:
        descLen = 11;
        break;
    case 0x4:
        descLen = 4; // MMC2: 24 and MMC4: 28, so we use the highest common factor
        break;
    case 0x5:
        descLen = 18;
        break;
    }

    unsigned char header[2048];
    ::memset(header, 0, 2048);

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_READ_TOC_PMA_ATIP;
    cmd[1] = (time ? 0x2 : 0x0);
    cmd[2] = format & 0x0F;
    cmd[6] = track;
    cmd[8] = 4;
    cmd[9] = 0; // Necessary to set the proper command length

    // we only read the header
    unsigned int dataLen = 4;
    if (cmd.transport(TR_DIR_READ, header, 4) == 0)
        dataLen = Device::from2Byte(header) + 2;
    else
        std::cout << "(K3b::Device::Device) " << device() << ": READ TOC/PMA/ATIP length det failed.";

    //
    // Some buggy firmwares return an invalid size here
    // So we simply use the maximum possible value to be on the safe side
    // with these buggy drives.
    // We cannot use this as default since many firmwares fail with a too high data length.
    //
    if (descLen != 0 && ((dataLen - 4) % descLen || dataLen < 4 + descLen))
    {
        std::cout << "(K3b::Device::Device) " << device() << ": READ TOC/PMA/ATIP invalid length returned: " << dataLen;
        dataLen = 0xFFFF;
    }

    //
    // Not all drives like uneven numbers
    //
    if (dataLen % 2)
        ++dataLen;

    // again with real length
    data.resize(dataLen);
    ::memset(data.data(), 0, data.size());

    cmd[7] = data.size() >> 8;
    cmd[8] = data.size();
    if (cmd.transport(TR_DIR_READ, data.data(), data.size()) == 0)
    {
        dataLen = KYSDKUdfBurn::min(dataLen, Device::from2Byte(data.data()) + 2u);
        if (descLen == 0 || (dataLen - 4) % descLen || dataLen < 4 + descLen)
        {
            // useless length
            data.clear();
            return false;
        }
        else
        {
            data.resize(dataLen);
            return true;
        }
    }
    else
    {
        std::cout << "(K3b::Device::Device) " << device() << ": READ TOC/PMA/ATIP format "
                  << format << " with real length "
                  << data.size() << " failed." << std::endl;
        data.clear();
        return false;
    }
}

bool DataCDROMImpl::readFormatCapacity(int wantedFormat, Msf &r, Msf *currentMax, int *currentMaxFormat)
{
    bool success = false;

    // the maximal length as stated in MMC4
    static const unsigned int maxLen = 4 + (8 * 32);

    unsigned char buffer[maxLen];
    ::memset(buffer, 0, maxLen);

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_READ_FORMAT_CAPACITIES;
    cmd[7] = maxLen >> 8;
    cmd[8] = maxLen & 0xFF;
    cmd[9] = 0; // Necessary to set the proper command length
    if (cmd.transport(TR_DIR_READ, buffer, maxLen) == 0)
    {

        unsigned int realLength = buffer[3] + 4;

        std::cout << "(Device::Device) " << device() << " READ FORMAT CAPACITY: Current/Max "
                  << (int)(buffer[8] & 0x3) << " " << Device::from4Byte(&buffer[4]) << std::endl;

        if (currentMax)
            *currentMax = Device::from4Byte(&buffer[4]);
        if (currentMaxFormat)
            *currentMaxFormat = (int)(buffer[8] & 0x3);

        //
        // Descriptor Type:
        // 0 - reserved
        // 1 - unformatted :)
        // 2 - formatted. Here we get the used capacity (lead-in to last lead-out/border-out)
        // 3 - No media present
        //
        for (unsigned int i = 12; i < realLength - 4; i += 8)
        {
            int format = (int)((buffer[i + 4] >> 2) & 0x3f);
            std::cout << "(Device::Device) " << device() << " READ FORMAT CAPACITY: "
                      << format << " " << Device::from4Byte(&buffer[i])
                      << " " << (int)((buffer[i + 5] << 16 & 0xFF0000) | (buffer[i + 6] << 8 & 0xFF00) | (buffer[i + 7] & 0xFF)) << std::endl;

            if (format == wantedFormat)
            {
                // found the descriptor
                r = KYSDKUdfBurn::max((int)Device::from4Byte(&buffer[i]), r.lba());
                success = true;
            }
        }
    }

    return success;
}

bool DataCDROMImpl::readTrackInformation(std::vector<unsigned char> &data, int type, int value)
{
    unsigned char header[2048];
    ::memset(header, 0, 2048);

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_READ_TRACK_INFORMATION;
    cmd[9] = 0; // Necessary to set the proper command length

    switch (type)
    {
    case 0:
    case 1:
    case 2:
        cmd[1] = type & 0x3;
        cmd[2] = value >> 24;
        cmd[3] = value >> 16;
        cmd[4] = value >> 8;
        cmd[5] = value;
        break;
    default:
        std::cout << "(Device::readTrackInformation) wrong type parameter: " << type;
        return false;
    }

    // first we read the header
    unsigned int dataLen = 4;
    cmd[8] = 4;
    if (cmd.transport(TR_DIR_READ, header, 4) == 0)
        dataLen = Device::from2Byte(header) + 2;
    else
        std::cout << "(Device::Device) " << device() << ": READ TRACK INFORMATION length det failed." << std::endl;

    //
    // Some buggy firmwares do not return the size of the available data
    // but the returned data.
    // So we try to determine the correct size based on the medium type
    // DVD+R:  40 (MMC4)
    // DVD-DL: 48 (MMC5)
    // CD:     36 (MMC2)
    //
    if (dataLen <= 6)
    {
        int m = mediaType();
        if (m & (Device::MEDIA_DVD_R_DL | Device::MEDIA_DVD_R_DL_SEQ | Device::MEDIA_DVD_R_DL_JUMP).toInt())
            dataLen = 48;
        else if (m & (Device::MEDIA_DVD_PLUS_R | Device::MEDIA_DVD_PLUS_R_DL).toInt())
            dataLen = 40;
        else
            dataLen = 36;
    }

    // again with real length
    data.resize(dataLen);
    ::memset(data.data(), 0, data.size());

    cmd[7] = data.size() >> 8;
    cmd[8] = data.size();
    if (cmd.transport(TR_DIR_READ, data.data(), data.size()) == 0)
    {
        data.resize(KYSDKUdfBurn::min(data.size(), (size_t)Device::from2Byte(data.data()) + 2));
        return true;
    }
    else
    {
        std::cout << "(Device::Device) " << device() << ": READ TRACK INFORMATION with real length "
                  << data.size() << " failed." << std::endl;
        data.clear();
        return false;
    }
}

bool DataCDROMImpl::readCapacity(Msf &r)
{
    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_READ_CAPACITY;
    cmd[9] = 0; // Necessary to set the proper command length
    unsigned char buf[8];
    ::memset(buf, 0, 8);
    if (cmd.transport(TR_DIR_READ, buf, 8) == 0)
    {
        r = Device::from4Byte(buf);
        return true;
    }
    else
        return false;
}

Toc DataCDROMImpl::readToc()
{
    bool needToClose = !isOpen();

    Toc toc;

    if (!open())
        return toc;

    int mt = mediaType();

    //
    // Use the profile if available because DVD-ROM units need to treat DVD+-R(W) media as DVD-ROM
    // if supported at all
    //
    if (currentProfile() == Device::MEDIA_DVD_ROM)
        mt = Device::MEDIA_DVD_ROM;

    if (mt & (Device::MEDIA_DVD_MINUS_ALL | Device::MEDIA_DVD_PLUS_RW | Device::MEDIA_DVD_ROM).toInt())
    {
        if (!readFormattedToc(toc, mt))
        {
            Msf size;
            if (readCapacity(size))
            {
                Track track;
                track.setFirstSector(0);
                track.setLastSector(size.lba());
                track.setSession(1);
                track.setType(Track::TYPE_DATA);
                track.setMode(Track::DVD);
                track.setCopyPermitted(mt != Device::MEDIA_DVD_ROM);
                track.setPreEmphasis(mt != Device::MEDIA_DVD_ROM);

                toc.append(track);
            }
            else
                std::cout << "(Device::Device) " << device()
                          << "READ CAPACITY for toc failed." << std::endl;
        }
    }

    else if (mt & (Device::MEDIA_DVD_PLUS_R | Device::MEDIA_DVD_PLUS_R_DL).toInt())
    {
        //
        // a DVD+R disk may have multiple sessions
        // every session may contain up to 16 fragments
        // if the disk is open there is one open session
        // every closed session is viewed as a track whereas
        // every fragment of the open session is viewed as a track
        //
        // We may use
        // READ DISK INFORMATION
        // READ TRACK INFORMATION: track number FFh, however, does not refer to the invisible track
        // READ TOC/PMA/ATIP: form 0 refers to all closed sessions
        //                    form 1 refers to the last closed session
        //
        readFormattedToc(toc, mt);
    }

    else if (mt & Device::MEDIA_BD_ALL)
    {
        readFormattedToc(toc, mt);
    }

    else if (mt == Device::MEDIA_DVD_RAM)
    {
        std::cout << "(Device::readDvdToc) no dvdram support" << std::endl;
    }

    else if (mt & Device::MEDIA_CD_ALL)
    {
        bool success = readRawToc(toc);
        if (!success)
        {
            success = readFormattedToc(toc, mt);

            if (!success)
            {
                std::cout << "(Device::Device) MMC READ TOC failed. falling back to cdrom.h." << std::endl;
                readTocLinux(toc);
            }

            if (success)
                fixupToc(toc);
        }
    }

    if (needToClose)
        close();

    return toc;
}

bool DataCDROMImpl::readFormattedToc(Toc &toc, int mt)
{
    // if the device is already opened we do not close it
    // to allow fast multiple method calls in a row
    bool needToClose = !isOpen();

    bool success = false;

    toc.clear();

    int lastTrack = 0;

    std::vector<unsigned char> data;
    if (!(mt & Device::MEDIA_CD_ALL))
    {
        //
        // on DVD-R(W) multisession disks only two sessions are represented as tracks in the readTocPmaAtip
        // response (fabricated TOC). Thus, we use readDiscInformation for DVD media to get the proper number of tracks
        //
        if (readDiscInformation(data))
        {
            lastTrack = (int)(data[11] << 8 | data[6]);

            if (readTrackInformation(data, 1, lastTrack))
            {
                track_info_t *trackInfo = (track_info_t *)data.data();

                if (trackInfo->blank)
                {
                    lastTrack--;
                }

                success = true;
            }
            else
            {
                success = false;
                goto cleanup_readFormattedToc;
            }
        }
        else
        {
            success = false;
            goto cleanup_readFormattedToc;
        }
    }
    else
    {
        if (readTocPmaAtip(data, 0, 0, 1))
        {

            if (data.size() < 4)
            {
                std::cout << "(Device::Device) " << device() << ": formatted toc data too small." << std::endl;
            }
            else if (data.size() != ((int)sizeof(toc_track_descriptor) * ((int)data[3] + 1)) + 4)
            {
                std::cout << "(Device::Device) " << device() << ": invalid formatted toc data length: "
                          << (data.size() - 2) << std::endl;
            }
            else
            {
                lastTrack = data[3];
                toc_track_descriptor *td = (toc_track_descriptor *)&data[4];
                for (int i = 0; i < lastTrack; ++i)
                {

                    Track track;
                    unsigned int control = 0;

                    //
                    // In case READ TRACK INFORMATION fails:
                    // no session number info
                    // no track length and thus possibly incorrect last sector for
                    // multisession disks
                    //
                    track.setFirstSector(Device::from4Byte(td[i].start_adr));
                    track.setLastSector(Device::from4Byte(td[i + 1].start_adr) - 1);
                    control = td[i].control;

                    track.setType((control & 0x4) ? Track::TYPE_DATA : Track::TYPE_AUDIO);
                    track.setMode(getTrackDataMode(track));
                    track.setCopyPermitted(control & 0x2);
                    track.setPreEmphasis(control & 0x1);

                    toc.append(track);
                }

                success = true;
            }
        }
    }

    //
    // Try to get information for all the tracks
    //
    for (int i = 0; i < lastTrack; ++i)
    {
        if (toc.count() < i + 1)
            toc.append(Track());

        std::vector<unsigned char> trackData;
        if (readTrackInformation(trackData, 1, i + 1))
        {
            track_info_t *trackInfo = (track_info_t *)trackData.data();

            toc[i].setFirstSector(Device::from4Byte(trackInfo->track_start));

            if (i > 0 && toc[i - 1].lastSector() == 0)
                toc[i - 1].setLastSector(toc[i].firstSector() - 1);

            // There are drives that return 0 track length here!
            // Some drives even return an invalid length here. :(
            if (Device::from4Byte(trackInfo->track_size) > 0)
                toc[i].setLastSector(toc[i].firstSector() + Device::from4Byte(trackInfo->track_size) - 1);

            if (trackInfo->nwa_v)
            {
                toc[i].setNextWritableAddress(Device::from4Byte(trackInfo->next_writable));
                toc[i].setFreeBlocks(Device::from4Byte(trackInfo->free_blocks));
            }

            toc[i].setSession((int)((trackInfo->session_number_m << 8 & 0xf0) |
                                    (trackInfo->session_number_l & 0x0f))); // FIXME: is this BCD?

            int control = trackInfo->track_mode;

            if (mt & Device::MEDIA_CD_ALL)
            {
                toc[i].setType((control & 0x4) ? Track::TYPE_DATA : Track::TYPE_AUDIO);
                toc[i].setMode(getTrackDataMode(toc[i]));
            }
            else
            {
                toc[i].setType(Track::TYPE_DATA);
                toc[i].setMode(Track::DVD);
            }
            toc[i].setCopyPermitted(control & 0x2);
            toc[i].setPreEmphasis(control & 0x1);
        }
        else if (!(mt & Device::MEDIA_CD_ALL))
        {
            success = false;
        }
    }

    // this can only happen with DVD media
    if (!toc.isEmpty() && toc.last().lastSector() == 0)
    {
        std::cout << "(Device::Device) " << device() << " no track length for the last non-empty track." << std::endl;
        std::vector<unsigned char> trackData;
        if (readTrackInformation(trackData, 1, lastTrack + 1))
        {
            track_info_t *trackInfo = (track_info_t *)trackData.data();

            toc.last().setLastSector(Device::from4Byte(trackInfo->track_start) - 1);
        }
    }

cleanup_readFormattedToc:
    if (needToClose)
        close();

    return success;
}

bool DataCDROMImpl::readRawToc(Toc &toc)
{
    // if the device is already opened we do not close it
    // to allow fast multiple method calls in a row
    bool needToClose = !isOpen();

    bool success = false;

    toc.clear();

    if (open())
    {
        //
        // Read Raw TOC (format: 0010b)
        //
        // For POINT from 01h-63h we get all the tracks
        // POINT a1h gices us the last track number in the session in PMIN
        // POINT a2h gives the start of the session lead-out in PMIN,PSEC,PFRAME
        //

        std::vector<unsigned char> data;

        if (readTocPmaAtip(data, 2, false, 1))
        {
            if (data.size() > 4)
            {
                success = true;

                toc_raw_track_descriptor *tr = (toc_raw_track_descriptor *)&data[4];

                //
                // debug the raw toc data
                //
                std::cout << "Session |  ADR   | CONTROL|  TNO   | POINT  |  Min   |  Sec   | Frame  |  Zero  |  PMIN  |  PSEC  | PFRAME |" << std::endl;
                for (int i = 0; i < (data.size() - 4) / (int)sizeof(toc_raw_track_descriptor); ++i)
                {
                    std::cout << std::setw(6) << (int)tr[i].session_number << " |"
                              << std::setw(6) << (int)tr[i].adr << " |"
                              << std::setw(6) << (int)tr[i].control << " |"
                              << std::setw(6) << (int)tr[i].tno << " |"
                              << std::setw(6) << std::hex << (int)tr[i].point << " |" << std::dec
                              << std::setw(6) << (int)tr[i].min << " |"
                              << std::setw(6) << (int)tr[i].sec << " |"
                              << std::setw(6) << (int)tr[i].frame << " |"
                              << std::setw(6) << std::hex << (int)tr[i].zero << " |" << std::dec
                              << std::setw(6) << (int)tr[i].p_min << " |"
                              << std::setw(6) << (int)tr[i].p_sec << " |"
                              << std::setw(6) << (int)tr[i].p_frame << " |"
                              << std::endl;
                }

                //
                // First we try to determine if the raw toc data uses BCD values
                //
                int isBcd = rawTocDataWithBcdValues(data);
                if (isBcd == -1)
                {
                    success = false;
                    goto cleanup_readRawToc;
                }

                Msf sessionLeadOut;

                for (unsigned int i = 0; i < (data.size() - 4) / (unsigned int)sizeof(toc_raw_track_descriptor); ++i)
                {
                    if (tr[i].adr == 1 && tr[i].point <= 0x63)
                    {
                        // track
                        Track track;
                        track.setSession(tr[i].session_number);

                        // :( We use 00:00:00 == 0 lba)
                        if (isBcd)
                            track.setFirstSector(Msf(Device::fromBcd(tr[i].p_min),
                                                     Device::fromBcd(tr[i].p_sec),
                                                     Device::fromBcd(tr[i].p_frame)) -
                                                 150);
                        else
                            track.setFirstSector(Msf(tr[i].p_min, tr[i].p_sec, tr[i].p_frame) - 150);

                        track.setType(tr[i].control & 0x4 ? Track::TYPE_DATA : Track::TYPE_AUDIO);
                        track.setMode(track.type() == Track::TYPE_DATA ? getTrackDataMode(track) : Track::UNKNOWN);
                        track.setCopyPermitted(tr[i].control & 0x2);
                        track.setPreEmphasis(tr[i].control & 0x1);

                        //
                        // only do this within a session because otherwise we already set the last sector with the session leadout
                        //
                        if (!toc.isEmpty())
                            if (toc[toc.count() - 1].session() == track.session())
                                toc[toc.count() - 1].setLastSector(track.firstSector() - 1);

                        toc.append(track);
                    }
                    else if (tr[i].point == 0xa2)
                    {
                        //
                        // since the session is always reported before the tracks this is where we do this:
                        // set the previous session's last tracks's last sector to the first sector of the
                        // session leadout (which was reported before the tracks)
                        //
                        // This only happens on multisession CDs
                        //
                        if (!toc.isEmpty())
                            toc[toc.count() - 1].setLastSector(sessionLeadOut - 1);

                        // this is save since the descriptors are reported in ascending order of the session number
                        // :( We use 00:00:00 == 0 lba)
                        if (isBcd)
                            sessionLeadOut = Msf(Device::fromBcd(tr[i].p_min),
                                                 Device::fromBcd(tr[i].p_sec),
                                                 Device::fromBcd(tr[i].p_frame)) -
                                             150;
                        else
                            sessionLeadOut = Msf(tr[i].p_min, tr[i].p_sec, tr[i].p_frame) - 150;
                    }
                }

                std::cout << device() << ": setting last sector of last track to " << (sessionLeadOut - 1).lba() << std::endl;

                // set the last track's last sector
                if (!toc.isEmpty())
                    toc[toc.count() - 1].setLastSector(sessionLeadOut - 1);
            }
            else
                std::cout << "(Device::Device) " << device() << " empty raw toc." << std::endl;
        }
    }

cleanup_readRawToc:
    if (needToClose)
        close();

    return success;
}

bool DataCDROMImpl::readTocLinux(Toc &toc)
{
    // if the device is already opened we do not close it
    // to allow fast multiple method calls in a row
    bool needToClose = !isOpen();

    bool success = true;

    toc.clear();

    struct cdrom_tochdr tochdr;
    struct cdrom_tocentry tocentry;

    if (open())
    {
        //
        // CDROMREADTOCHDR ioctl returns:
        // cdth_trk0: First Track Number
        // cdth_trk1: Last Track Number
        //
        if (::ioctl(m_iHandle, CDROMREADTOCHDR, &tochdr))
        {
            std::cout << "(Device) could not get toc header !" << std::endl;
            success = false;
        }
        else
        {
            Track lastTrack;
            for (int i = tochdr.cdth_trk0; i <= tochdr.cdth_trk1 + 1; i++)
            {
                ::memset(&tocentry, 0, sizeof(struct cdrom_tocentry));
                // get Lead-Out Information too
                tocentry.cdte_track = (i <= tochdr.cdth_trk1) ? i : CDROM_LEADOUT;
                tocentry.cdte_format = CDROM_LBA;

                if (::ioctl(m_iHandle, CDROMREADTOCENTRY, &tocentry))
                {
                    std::cout << "(Device) error reading tocentry " << i << std::endl;
                    success = false;
                    break;
                }

                int startSec = tocentry.cdte_addr.lba;
                int control = tocentry.cdte_ctrl & 0x0f;
                int mode = tocentry.cdte_datamode;
                if (i > tochdr.cdth_trk0)
                {
                    Track track(lastTrack.firstSector(), startSec - 1, lastTrack.type(), lastTrack.mode());
                    track.setPreEmphasis(control & 0x1);
                    track.setCopyPermitted(control & 0x2);
                    toc.append(track);
                }
                Track::TrackType trackType = Track::TYPE_UNKNOWN;
                Track::DataMode trackMode = Track::UNKNOWN;
                if ((control & 0x04) && (tocentry.cdte_track != CDROM_LEADOUT))
                {
                    trackType = Track::TYPE_DATA;
                    if (mode == 1)
                        trackMode = Track::MODE1;
                    else if (mode == 2)
                        trackMode = Track::MODE2;

                    Track::DataMode tm = getDataMode(startSec);
                    if (tm != Track::UNKNOWN)
                        trackMode = tm;
                }
                else
                    trackType = Track::TYPE_AUDIO;

                lastTrack = Track(startSec, startSec, trackType, trackMode);
            }
        }

        if (needToClose)
            close();
    }
    else
        success = false;

    return success;
}

bool DataCDROMImpl::fixupToc(Toc &toc)
{
    bool success = false;

    //
    // This is a very lame method of fixing the TOC of an Advanced Audio CD
    // (a CD with two sessions: one with audio tracks and one with the data track)
    // If a drive does not support reading raw toc or reading track info we only
    // get every track's first sector. But between sessions there is a gap which is used
    // for ms stuff. In this case it's 11400 sectors in size. When ripping ausio we would
    // include these 11400 sectors which would result in a strange ending audio file.
    //
    if (numSessions() > 1 || toc.contentType() == MIXED)
    {
        std::cout << "(K3b::Device::Device) fixup multisession toc..." << std::endl;

        //
        // we need to update the last sector of every last track in every session
        // for now we only update the track before the last session...
        // This is the most often case: Advanced Audio CD
        //

        std::vector<unsigned char> data;
        if (readTocPmaAtip(data, 1, false, 0))
        {

            //
            // data[6]    - first track number in last complete session
            // data[8-11] - start address of first track in last session
            //

            toc[(unsigned int)data[6] - 2].setLastSector(Device::from4Byte(&data[8]) - 11400 - 1);

            success = true;
        }
        else
            std::cout << "(K3b::Device::Device) FIXUP TOC failed." << std::endl;
    }

    return success;
}

Track::DataMode DataCDROMImpl::getTrackDataMode(const Track &track)
{
    return getDataMode(track.firstSector());
}

Track::DataMode DataCDROMImpl::getDataMode(const Msf &sector)
{
    bool needToClose = !isOpen();

    Track::DataMode ret = Track::UNKNOWN;

    if (!open())
        return ret;

    // we use readCdMsf here since it's defined mandatory in MMC1 and
    // we only use this method for CDs anyway
    unsigned char data[2352];
    bool readSuccess = readCdMsf(data, 2352,
                                 0,     // all sector types
                                 false, // no dap
                                 sector,
                                 sector + 1,
                                 true, // SYNC
                                 true, // HEADER
                                 true, // SUBHEADER
                                 true, // USER DATA
                                 true, // EDC/ECC
                                 0,    // no c2 info
                                 0);

    if (readSuccess)
    {
        if (data[15] == 0x1)
            ret = Track::MODE1;
        else if (data[15] == 0x2)
            ret = Track::MODE2;
        if (ret == Track::MODE2)
        {
            if (data[16] == data[20] &&
                data[17] == data[21] &&
                data[18] == data[22] &&
                data[19] == data[23])
            {
                if (data[18] & 0x20)
                    ret = Track::XA_FORM2;
                else
                    ret = Track::XA_FORM1;
            }
        }
    }

    if (needToClose)
        close();

    return ret;
}

int DataCDROMImpl::rawTocDataWithBcdValues(const std::vector<unsigned char> &data)
{
    toc_raw_track_descriptor *tr = (toc_raw_track_descriptor *)&data[4];

    bool notBcd = false;
    bool notHex = false;

    //
    // in most cases this will already tell us if a drive does not provide bcd numbers
    // (which should be all newer MMC drives)
    //
    for (unsigned int i = 0; i < (data.size() - 4) / (unsigned int)sizeof(toc_raw_track_descriptor); ++i)
    {
        if (tr[i].adr == 1 && tr[i].point <= 0xa2)
        {
            if (!Device::isValidBcd(tr[i].p_min) ||
                !Device::isValidBcd(tr[i].p_sec) ||
                !Device::isValidBcd(tr[i].p_frame))
            {
                notBcd = true;
                break;
            }

            // we only need to check sec and frame since min needs to be <= 99
            // and bcd values are never bigger than 99.
            else if ((int)Device::fromBcd(tr[i].p_sec) >= 60 ||
                     (int)Device::fromBcd(tr[i].p_frame) >= 75)
            {
                notBcd = true;
                break;
            }
        }
    }

    //
    // all values are valid bcd values but we still don't know for sure if they are really
    // used as bcd. So we also check the HEX values.
    //
    for (unsigned int i = 0; i < (data.size() - 4) / (unsigned int)sizeof(toc_raw_track_descriptor); ++i)
    {
        if (tr[i].adr == 1 && tr[i].point <= 0xa2)
        {
            if ((int)tr[i].p_min > 99 ||
                (int)tr[i].p_sec >= 60 ||
                (int)tr[i].p_frame >= 75)
            {
                notHex = true;
                break;
            }
        }
    }

    //
    // If all values are valid bcd and valid hex we check the start sectors of the tracks.
    //
    if (!notHex || !notBcd)
    {
        Msf sessionLeadOutHex, sessionLeadOutBcd;
        Msf lastTrackHex, lastTrackBcd;

        for (unsigned int i = 0; i < (data.size() - 4) / (unsigned int)sizeof(toc_raw_track_descriptor); ++i)
        {

            if (tr[i].adr == 1)
            {
                if (tr[i].point < 0x64)
                {

                    // check hex values
                    if (Msf(tr[i].p_min, tr[i].p_sec, tr[i].p_frame) <
                        lastTrackHex)
                        notHex = true;

                    // check bcd values
                    if (Msf(Device::fromBcd(tr[i].p_min), Device::fromBcd(tr[i].p_sec), Device::fromBcd(tr[i].p_frame)) <
                        lastTrackBcd)
                        notBcd = true;

                    lastTrackBcd = Msf(Device::fromBcd(tr[i].p_min), Device::fromBcd(tr[i].p_sec), Device::fromBcd(tr[i].p_frame));
                    lastTrackHex = Msf(tr[i].p_min, tr[i].p_sec, tr[i].p_frame);
                }
                else if (tr[i].point == 0xa2)
                {
                    if (sessionLeadOutHex < lastTrackHex)
                        notHex = true;
                    if (sessionLeadOutBcd < lastTrackBcd)
                        notBcd = true;

                    sessionLeadOutHex = Msf(tr[i].p_min, tr[i].p_sec, tr[i].p_frame);
                    sessionLeadOutBcd = Msf(Device::fromBcd(tr[i].p_min), Device::fromBcd(tr[i].p_sec), Device::fromBcd(tr[i].p_frame));
                }
            }
        }

        // check the last track
        if (sessionLeadOutHex < lastTrackHex)
            notHex = true;
        if (sessionLeadOutBcd < lastTrackBcd)
            notBcd = true;
    }

    if (!notBcd && !notHex)
    {
        std::cout << "(Device::Device) need to compare raw toc to formatted toc. :(" << std::endl;
        //
        // All values are valid bcd and valid HEX values so we compare with the formatted toc.
        // This slows us down a lot but in most cases this should not be reached anyway.
        //
        // TODO: also check the bcd values
        //
        Toc formattedToc;
        if (readFormattedToc(formattedToc, Device::MEDIA_CD_ROM))
        {
            for (unsigned int i = 0; i < (data.size() - 4) / (unsigned int)sizeof(toc_raw_track_descriptor); ++i)
            {
                if (tr[i].adr == 1 && tr[i].point < 0x64)
                {
                    unsigned int track = (int)tr[i].point;

                    // FIXME: do bcd drive also encode the track number in bcd? If so test it, too.

                    if ((int)track > formattedToc.count())
                    {
                        notHex = true;
                        break;
                    }

                    Msf posHex(tr[i].p_min,
                               tr[i].p_sec,
                               tr[i].p_frame);
                    Msf posBcd(Device::fromBcd(tr[i].p_min),
                               Device::fromBcd(tr[i].p_sec),
                               Device::fromBcd(tr[i].p_frame));
                    posHex -= 150;
                    posBcd -= 150;
                    if (posHex != formattedToc[track - 1].firstSector())
                        notHex = true;
                    if (posBcd != formattedToc[track - 1].firstSector())
                        notBcd = true;
                }
            }
        }
    }

    if (notBcd)
        std::cout << "(Device::Device) found invalid bcd values. No bcd toc." << std::endl;
    if (notHex)
        std::cout << "(Device::Device) found invalid hex values. No hex toc." << std::endl;

    if (notBcd == notHex)
    {
        std::cout << "(Device::Device) unable to determine if hex (" << notHex << ") or bcd (" << notBcd << ")." << std::endl;
        if (!notHex)
        {
            std::cout << "Assuming hex encoding in favor of newer drives and the more reliable raw toc." << std::endl;
            return 0;
        }
        return -1;
    }
    else if (notBcd)
        return 0;
    else
        return 1;
}

int DataCDROMImpl::numSessions()
{
    //
    // Session Info
    // ============
    // Byte 0-1: Data Length
    // Byte   2: First Complete Session Number (Hex) - always 1
    // Byte   3: Last Complete Session Number (Hex)
    //

    int ret = -1;

    std::vector<unsigned char> data;

    int m = mediaType();
    if (m & Device::MEDIA_CD_ALL)
    {
        //
        // Althought disk_info should get the real value without ide-scsi
        // I keep getting wrong values (the value is too high. I think the leadout
        // gets counted as session sometimes :()
        //
        if (readTocPmaAtip(data, 1, 0, 0))
        {
            ret = data[3];
        }
        else
        {
            std::cout << "(K3b::Device::Device) " << device() << ": could not get session info !" << std::endl;
        }
    }
    else if (m & (Device::MEDIA_DVD_PLUS_RW | Device::MEDIA_DVD_RW_OVWR | Device::MEDIA_BD_RE).toInt())
    {
        // fabricate value
        int e = isEmpty();
        return (e == Device::STATE_COMPLETE || e == Device::STATE_COMPLETE ? 1 : 0);
    }
    else
    {
        if (readDiscInformation(data))
        {
            ret = (int)(data[9] << 8 | data[4]);

            // do only count complete sessions
            if ((data[2] >> 2) != 3)
                ret--;
        }
    }

    return ret;
}

bool DataCDROMImpl::read10(unsigned char *data, unsigned int dataLen, unsigned long startAdress, unsigned int length, bool fua)
{
    ::memset(data, 0, dataLen);

    DiscScsi cmd(m_oBlockName);
    cmd[0] = 0x28; //  MMC_READ_10
    cmd[1] = (fua ? 0x8 : 0x0);
    cmd[2] = startAdress >> 24;
    cmd[3] = startAdress >> 16;
    cmd[4] = startAdress >> 8;
    cmd[5] = startAdress;
    cmd[7] = length >> 8;
    cmd[8] = length;
    cmd[9] = 0; // Necessary to set the proper command length

    return (cmd.transport(TR_DIR_READ, data, dataLen) == 0)? true : false;
}

bool DataCDROMImpl::readCdMsf(unsigned char *data, unsigned int dataLen, int sectorType, bool dap, const Msf &startAdress, const Msf &endAdress, bool sync, bool header, bool subHeader, bool userData, bool edcEcc, int c2, int subChannel)
{
    ::memset(data, 0, dataLen);

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_READ_CD_MSF;
    cmd[1] = (sectorType << 2 & 0x1c) | (dap ? 0x2 : 0x0);
    cmd[3] = (startAdress + 150).minutes();
    cmd[4] = (startAdress + 150).seconds();
    cmd[5] = (startAdress + 150).frames();
    cmd[6] = (endAdress + 150).minutes();
    cmd[7] = (endAdress + 150).seconds();
    cmd[8] = (endAdress + 150).frames();
    cmd[9] = ((sync ? 0x80 : 0x0) |
              (subHeader ? 0x40 : 0x0) |
              (header ? 0x20 : 0x0) |
              (userData ? 0x10 : 0x0) |
              (edcEcc ? 0x8 : 0x0) |
              (c2 << 1 & 0x6));
    cmd[10] = subChannel & 0x7;
    cmd[11] = 0; // Necessary to set the proper command length

    if (0 != cmd.transport(TR_DIR_READ, data, dataLen))
    {
        std::cout << "(K3b::Device::Device) " << device() << ": READ CD MSF failed!" << std::endl;
        return false;
    }
    return true;
}

Device::MediaType DataCDROMImpl::mediaType()
{
    Device::MediaType m = Device::MEDIA_UNKNOWN;

    if (testUnitReady())
    {

        int p = currentProfile();
        if (p != -1)
            m = (Device::MediaType)p;

        if (m & (Device::MEDIA_UNKNOWN | Device::MEDIA_DVD_ROM | Device::MEDIA_HD_DVD_ROM).toInt())
        {
            //
            // We prefer the Device::MediaType as reported by the media since this way
            // even ROM drives may report the correct type of writable media.
            //

            // 4 bytes header + 2048 bytes layer descriptor
            std::vector<unsigned char> data;
            if (readDvdStructure(data))
            {
                if (data.size() >=4) {
                    switch (data[4] & 0xF0)
                    {
                    case 0x00:
                        m = Device::MEDIA_DVD_ROM;
                        break;
                    case 0x10:
                        m = Device::MEDIA_DVD_RAM;
                        break;
                    case 0x20:
                        m = Device::MEDIA_DVD_R;
                        break; // there seems to be no value for DVD-R DL, it reports DVD-R
                    case 0x30:
                        m = Device::MEDIA_DVD_RW;
                        break;
                    case 0x40:
                        m = Device::MEDIA_HD_DVD_ROM;
                        break;
                    case 0x50:
                        m = Device::MEDIA_HD_DVD_R;
                        break;
                    case 0x60:
                        m = Device::MEDIA_HD_DVD_RAM;
                        break;
                    case 0x90:
                        m = Device::MEDIA_DVD_PLUS_RW;
                        break;
                    case 0xA0:
                        m = Device::MEDIA_DVD_PLUS_R;
                        break;
                    case 0xE0:
                        m = Device::MEDIA_DVD_PLUS_R_DL;
                        break;
                    default:
                        break;
                    }
                } else {
                    
                }
            }
        }

        if (m & (Device::MEDIA_UNKNOWN | Device::MEDIA_BD_ROM).toInt())
        {
            //
            // We prefer the Device::MediaType as reported by the media since this way
            // even ROM drives may report the correct type of writable media.
            //

            std::vector<unsigned char> data;
            if (readDiscStructure(data, 1, 0))
            {
                if (data.size() > 4 + 12 &&
                    data[4 + 8] == 'B' && data[4 + 9] == 'D')
                {
                    switch (data[4 + 10])
                    {
                    case 'O':
                        m = Device::MEDIA_BD_ROM;
                        break;
                    case 'W':
                        m = Device::MEDIA_BD_RE;
                        break;
                    case 'R':
                        m = Device::MEDIA_BD_R;
                        break;
                    }
                }
            }
        }

        //
        // Only old CD or DVD devices do not report a current profile
        // or report CD-ROM profile for all CD types
        //
#if 0
        /* fix 410105 智权顿的u盘被识别为CD-R盘 */
        if (m & (Device::MEDIA_UNKNOWN | Device::MEDIA_CD_ROM).toInt())
        {
            std::vector<unsigned char> data;
            if (readTocPmaAtip(data, 4, false, 0))
            {
                if ((data[6] >> 6) & 1)
                    m = Device::MEDIA_CD_RW;
                else
                    m = Device::MEDIA_CD_R;
            }
            else
                m = Device::MEDIA_CD_ROM;
        }
#endif
    }

    return m;
}

int DataCDROMImpl::isEmpty()
{
    // if the device is already opened we do not close it
    // to allow fast multiple method calls in a row
    bool needToClose = !isOpen();

    int ret = Device::STATE_UNKNOWN;
    if (!open())
        return Device::STATE_UNKNOWN;

    // Ensure objects with non-trivial constructors are initialized
    // before any potential jump to cleanup label.
    std::vector<unsigned char> data;

    if (!testUnitReady())
    {
        ret = Device::STATE_NO_MEDIA;
        goto cleanup_isEmpty;
    }

    if (readDiscInformation(data))
    {
        disc_info_t *inf = (disc_info_t *)data.data();
        switch (inf->status)
        {
        case 0:
            ret = Device::STATE_EMPTY;
            break;
        case 1:
            ret = Device::STATE_INCOMPLETE;
            break;
        case 2:
            ret = Device::STATE_COMPLETE;
            break;
        default:
            ret = Device::STATE_UNKNOWN;
            break;
        }
    }

cleanup_isEmpty:
    if (needToClose)
        close();

    return ret;
}
