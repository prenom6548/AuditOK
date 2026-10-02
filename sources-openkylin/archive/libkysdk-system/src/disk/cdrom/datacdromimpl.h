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

#ifndef DATACDROMIMPL_H
#define DATACDROMIMPL_H

#include <string>
#include <vector>
#include "../disc/datamedium.h"

enum MEDIUM_TYPE
{
    MEDIUM_UNKOWN = 0x0000,
    MEDIUM_CD_ROM = 0x0001,
    MEDIUM_CD_R = 0x0002,
    MEDIUM_CD_RW = 0x0004,
    MEDIUM_DVD_ROM = 0x0008,
    MEDIUM_DVD_R = 0x0010,
    MEDIUM_DVD_RAM = 0x0020,
    MEDIUM_DVD_R_DL_SEQ = 0x0040,
    MEDIUM_DVD_R_DL_JUMP = 0x0080,
    MEDIUM_DVD_PLUS_RW = 0x0100,
    MEDIUM_DVD_PLUS_R = 0x0101,
    MEDIUM_DVD_PLUS_RW_DL = 0x0102,
    MEDIUM_DVD_PLUS_R_DL = 0x0104,
    MEDIUM_DVD_RW_OVERWRITE = 0x0108,
    MEDIUM_DVD_RW_SEQ = 0x0110,
    MEDIUM_CD_RW_S0 = 0x0120, // in MMC-5 CD-RW has 8 subtypes.
    MEDIUM_CD_RW_S1 = 0x0140,
    MEDIUM_CD_RW_S2 = 0x0180,
    MEDIUM_CD_RW_S3 = 0x0200,
    MEDIUM_CD_RW_S4 = 0x0201,
    MEDIUM_CD_RW_S5 = 0x0202,
    MEDIUM_CD_RW_S6 = 0x0204,
    MEDIUM_CD_RW_S7 = 0x0208,
    MEDIUM_DVD_RW = 0x0210
};

enum MEDIUM_STATUS
{
    MEDIUM_EMPTY = 0,
    MEDIUM_INCOMPLETE,
    MEDIUM_FINALIZED,
    MEDIUM_OTHER
};

enum BURN_MODE
{
    BURN_CD_TAO = 0x01,
    BURN_CD_SAO = 0x02,
    BURN_CD_DAO = 0x04 // close session/track when TAO over.
};

namespace KYSDKUdfBurn
{
    template <typename T>
    static constexpr inline const T &min(const T &a, const T &b)
    {
        return (a < b) ? a : b;
    }
    template <typename T>
    static constexpr inline const T &max(const T &a, const T &b)
    {
        return (a < b) ? b : a;
    }
}

class DataCDROMImpl
{
public:
    explicit DataCDROMImpl(std::string &blockName);
    ~DataCDROMImpl();

    unsigned long getCDROMCapacity();
    unsigned long getCDROMUsedCapacity();
    void calculateDiscCapacity();

public:
    void getCDROMInfo();
    std::string getDiscType();
    std::string getCDROMType();
    std::string device() const;
    bool isOpen();
    bool open();
    void close();
    bool read10(unsigned char *data,
                unsigned int dataLen,
                unsigned long startAdress,
                unsigned int length,
                bool fua = false);

public:
    bool execSCSI(int fd, const unsigned char *cdb, const int cdbLength,
                  unsigned char *result, const int resultLength);
    int currentProfile();
    int featureCurrent(unsigned int feature);
    bool getFeature(std::vector<unsigned char> &data, unsigned int feature);
    bool readDiscInformation(std::vector<unsigned char> &data);
    bool readDvdStructure(std::vector<unsigned char> &data,
                          unsigned int format = 0x0,
                          unsigned int layer = 0x0,
                          unsigned long address = 0,
                          unsigned int agid = 0x0);
    bool readDiscStructure(std::vector<unsigned char> &data,
                           unsigned int mediaType = 0x0,
                           unsigned int format = 0x0,
                           unsigned int layer = 0x0,
                           unsigned long address = 0,
                           unsigned int agid = 0x0);
    bool readTocPmaAtip(std::vector<unsigned char> &data, int format, bool msf, int track);
    bool readFormatCapacity(int wantedFormat, Msf &r,
                            Msf *currentMax = 0, int *currentMaxFormat = 0);
    bool readTrackInformation(std::vector<unsigned char> &data, int type, int value);
    bool readCapacity(Msf &r);
    Toc readToc();
    bool readFormattedToc(Toc &toc, int mt);
    bool readRawToc(Toc &toc);
    bool readTocLinux(Toc &toc);
    bool fixupToc(Toc &toc);
    Track::DataMode getTrackDataMode(const Track &track);
    Track::DataMode getDataMode(const Msf &sector);
    int rawTocDataWithBcdValues(const std::vector<unsigned char> &data);
    int numSessions();
    bool testUnitReady();
    bool readCdMsf(unsigned char *data,
                   unsigned int dataLen,
                   int sectorType,
                   bool dap,
                   const Msf &startAdress,
                   const Msf &endAdress,
                   bool sync,
                   bool header,
                   bool subHeader,
                   bool userData,
                   bool edcEcc,
                   int c2,
                   int subChannel);
    Device::MediaType mediaType();
    int isEmpty();

private:
    int m_iHandle;
    unsigned int m_u32MediumRSupport;
    unsigned int m_u32MediumWSupport;

    std::string m_oBlockName;
    std::string m_oMediumType;

    unsigned int m_u32TrackNumber;
    unsigned long m_u64UsedCapacity;
    unsigned long m_u64FreeCapacity;
    unsigned long m_u64Capacity;
    DataMedium *m_oMedium;
};

#endif // DATACDROMIMPL_H
