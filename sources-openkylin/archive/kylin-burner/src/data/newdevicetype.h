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


/*
 *
 * Copyright (C) 2003-2009 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2009 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef BURN_DEVICE_UTILS_H_
#define BURN_DEVICE_UTILS_H_

#include "src/data/toc.h"
#include "src/data/msf.h"
#include "src/data/iso9660.h"
#include "src/data/devicetypes.h"
#include "datacdrom.h"
#include "datasetting.h"

#include <QDebug>

class DiscRead;
class ExtendDataCDRom;
typedef QVarLengthArray< unsigned char > UByteArray;

class ExtendMediumType : public DataMedium
{
    Q_OBJECT
public:
    explicit ExtendMediumType(QString type, QString blockname, QObject * parent = nullptr);
    ExtendMediumType(DataMedium &, QObject *);

    void load();


    enum MediumContent: int {
        ContentUnknown = 0x0,
        ContentNone = 0x1,
        ContentAudio = 0x2,
        ContentData = 0x4,
        ContentVideoCD = 0x8,
        ContentVideoDVD = 0x10,
        ContentAll = ContentNone|ContentAudio|ContentData|ContentVideoCD|ContentVideoDVD
    };

    const Iso9660SimplePrimaryDescriptor& iso9660Descriptor() const;

    Msf getMsfCapacity() const;
    Msf getMsfUsedCapacity() const;
    Msf getMsfFreeCapacity() const;
    QString blockDeviceName() const;
    Device::MediaState diskState() const;
    bool empty() const;
    bool appendable() const;
    bool rewritable() const;
    int numSessions() const;
    int numTracks() const;
    Device::MediaType mediaType() const;
    Device::MediaState lastSessionState() const;
    Device::BackGroundFormattingState bgFormatState() const;

    void diskInfo();
    void capacity();
    void analyseContent();

protected slots:
    void secondFixDiscStatus();
    void fixDiscStatus();
    void slotsMountPoint();

private:
    QString msfCapacity();
    QString msfUsedCapacity();
    QString msfFreeCapacity();

private:
    Msf              m_msfCapacity;
    Msf              m_msfUsedCapacity;
    Msf              m_firstLayerSize;

    int              m_currentProfile;
    Device::MediaState       m_diskState;

    Device::MediaState       m_lastSessionState;
    Device::BackGroundFormattingState m_bgFormatState;
    Device::MediaType m_mediaType;

    int m_numSessions;
    int m_numTracks;
    int m_numLayers;  // only for DVD media
    int m_rewritable;

    ExtendDataCDRom* m_new_cdrom;

    Toc toc;
    Iso9660SimplePrimaryDescriptor m_isoDesc;
    MediumContent m_oContent;
};

class DataCDROM;
class ExtendDataCDRom : public DataCDROM
{
    Q_OBJECT
public:
    explicit ExtendDataCDRom(QString blockName, QObject *parent = nullptr);
    ExtendDataCDRom(DataCDROM &cdrom, QString blockName, QObject *parent = nullptr);

    int currentProfile();
    int featureCurrent(unsigned int feature);
    bool getFeature(UByteArray& data, unsigned int feature);
    bool readDiscInformation(UByteArray& data);
    bool readDvdStructure(UByteArray& data,
                          unsigned int format = 0x0,
                          unsigned int layer = 0x0,
                          unsigned long address = 0,
                          unsigned int agid = 0x0);
    bool readDiscStructure(UByteArray& data,
                           unsigned int mediaType = 0x0,
                           unsigned int format = 0x0,
                           unsigned int layer = 0x0,
                           unsigned long address = 0,
                           unsigned int agid = 0x0);
    bool readTocPmaAtip(UByteArray& data, int format, bool msf, int track);
    bool readFormatCapacity(int wantedFormat, Msf& r,
                             Msf* currentMax = 0, int* currentMaxFormat = 0);
    bool readTrackInformation(UByteArray& data, int type, int value);
    bool readCapacity(Msf&r);
    Toc readToc();
    bool readFormattedToc(Toc&,int mediaType);
    bool readRawToc(Toc&) ;
    bool readTocLinux(Toc&);
    bool fixupToc(Toc& toc);
    Track::DataMode getTrackDataMode(const Track& track);
    Track::DataMode getDataMode(const Msf& sector);
    int rawTocDataWithBcdValues(const UByteArray& data);
    int numSessions();
    bool testUnitReady();
    bool readCdMsf( unsigned char* data,
                    unsigned int dataLen,
                    int sectorType,
                    bool dap,
                    const Msf& startAdress,
                    const Msf& endAdress,
                    bool sync,
                    bool header,
                    bool subHeader,
                    bool userData,
                    bool edcEcc,
                    int c2,
                    int subChannel );
    Device::MediaType mediaType();
    int isEmpty();

    bool update();
    DataCDROM * clone();
protected:
    void checkMedium();
};

#endif
