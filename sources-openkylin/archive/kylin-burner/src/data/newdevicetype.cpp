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


#include "newdevicetype.h"
#include "src/status-fix/discread.h"
#include "src/status-fix/discscsi.h"
#include "deviceglobals.h"
#include "devicetypes.h"
#include "mmc.h"
#include "src/frame/logger.h"
#include "src/frame/signals.h"
#include "glib.h"
#include "src/data/iso9660.h"
#include "src/data/iso9660backend.h"
#include "src/po/translation.h"
#include <mntent.h>
#undef signals
#include <gio/gio.h>
#define signals Q_SIGNAL

ExtendDataCDRom::ExtendDataCDRom(QString blockName, QObject *parent):DataCDROM(blockName,parent)
{
}

ExtendDataCDRom::ExtendDataCDRom(DataCDROM &cdrom, QString blockName, QObject *parent):DataCDROM(cdrom,blockName,parent)
{
}

int ExtendDataCDRom::currentProfile()
{
    unsigned char profileBuf[8];
    ::memset( profileBuf, 0, 8 );

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_GET_CONFIGURATION;
    cmd[1] = 1;
    cmd[8] = 8;
    cmd[9] = 0;      // Necessary to set the proper command length

    if( cmd.transport( TR_DIR_READ, profileBuf, 8 ) ) {
        qDebug() << "(Device::Device) " << device()
                 << " GET_CONFIGURATION failed." ;
        return Device::MEDIA_UNKNOWN;
    }
    else {
        short profile = Device::from2Byte( &profileBuf[6] );

        //
        // Plextor drives might not set a current profile
        // In that case we get the list of all current profiles
        // and simply use the first one in that list.
        //
        if( profile == 0x00 ) {
            qDebug() << "(Device::Device) " << device()
                     << " current profile 0. Checking current profile list instead.";
            UByteArray data;
            if( getFeature( data, Device::FEATURE_PROFILE_LIST ) ) {
                int featureLen( data[11] );
                for( int j = 0; j < featureLen; j+=4 ) {
                    // use the first current profile we encounter
                    if( data[12+j+2] & 0x1 ) {
                        profile = Device::from2Byte( &data[12+j] );
                        break;
                    }
                }
            }
        }

        switch (profile) {
        case 0x00: return Device::MEDIA_NONE;
        case 0x08: return Device::MEDIA_CD_ROM;
        case 0x09: return Device::MEDIA_CD_R;
        case 0x0A: return Device::MEDIA_CD_RW;
        case 0x10: return Device::MEDIA_DVD_ROM;
        case 0x11: return Device::MEDIA_DVD_R_SEQ;
        case 0x12: return Device::MEDIA_DVD_RAM;
        case 0x13: return Device::MEDIA_DVD_RW_OVWR;
        case 0x14: return Device::MEDIA_DVD_RW_SEQ;
        case 0x15: return Device::MEDIA_DVD_R_DL_SEQ;
        case 0x16: return Device::MEDIA_DVD_R_DL_JUMP;
        case 0x1A: return Device::MEDIA_DVD_PLUS_RW;
        case 0x1B: return Device::MEDIA_DVD_PLUS_R;
        case 0x2B: return Device::MEDIA_DVD_PLUS_R_DL;
        case 0x40: return Device::MEDIA_BD_ROM;
        case 0x41: {
            if( featureCurrent( Device::FEATURE_BD_PSEUDO_OVERWRITE ) == 1 )
                return Device::MEDIA_BD_R_SRM_POW;
            else
                return Device::MEDIA_BD_R_SRM;
        }
        case 0x42: return Device::MEDIA_BD_R_RRM;
        case 0x43: return Device::MEDIA_BD_RE;
        case 0x50: return Device::MEDIA_HD_DVD_ROM;
        case 0x51: return Device::MEDIA_HD_DVD_R;
        case 0x52: return Device::MEDIA_HD_DVD_RAM;
        default: return Device::MEDIA_UNKNOWN;
        }
    }
}

int ExtendDataCDRom::featureCurrent(unsigned int feature)
{
    UByteArray data;
    if( getFeature( data, feature ) ) {
        int ret = -1;
        if( data.size() >= 11 )
            ret = ( data[8+2]&1 ? 1 : 0 );  // check the current flag

        return ret;
    }
    else
        return -1;
}

bool ExtendDataCDRom::getFeature(UByteArray &data, unsigned int feature)
{
    unsigned char header[2048];
    ::memset( header, 0, 2048 );

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_GET_CONFIGURATION;
    cmd[1] = 2;      // read only specified feature
    cmd[2] = feature>>8;
    cmd[3] = feature;
    cmd[8] = 8;      // we only read the data length first
    cmd[9] = 0;      // Necessary to set the proper command length

    // we only read the data length first
    unsigned int dataLen = 8;
    if( cmd.transport( TR_DIR_READ, header, 8 ) )
        dataLen = Device::from4Byte( header ) + 4;
    else
        qDebug() << "(Device::Device) " << device() << ": GET CONFIGURATION length det failed.";

    //
    // Some buggy firmwares do not return the size of the available data
    // but the returned data or something invalid altogether.
    // So we simply use the maximum possible value to be on the safe side
    // with these buggy drives.
    // We cannot use this as default since many firmwares fail with a too high data length.
    //
    if( (dataLen-8) % 8 || dataLen <= 8 )
        dataLen = 0xFFFF;

    // again with real length
    data.resize( dataLen );
    ::memset( data.data(), 0, data.size() );

    cmd[7] = data.size() >> 8;
    cmd[8] = data.size();
    if( cmd.transport( TR_DIR_READ, data.data(), data.size() ) == 0 ) {
        data.resize( qMin( data.size(), (int)Device::from4Byte( data.data() ) + 4 ) );
        return true;
    }
    else {
        qDebug() << "(Device::Device) " << device() << ": GET CONFIGURATION with real length "
                 << data.size() << " failed.";
        qDebug();
        data.clear();
        return false;
    }
}

bool ExtendDataCDRom::readDiscInformation(UByteArray &data)
{
    unsigned char header[2048];
    ::memset( header, 0, 2048 );

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_READ_DISC_INFORMATION;
    cmd[8] = 2;      // we only read the data length first
    cmd[9] = 0;      // Necessary to set the proper command length

    // we only read the data length first
    unsigned int dataLen = 8;
    if( cmd.transport( TR_DIR_READ, header, 8 ) == 0 )
        dataLen = Device::from4Byte( header ) + 4;
    else
        qDebug() << "(Device::Device) " << device() << ": GET CONFIGURATION length det failed.";

    //
    // Some buggy firmwares do not return the size of the available data
    // but the returned data or something invalid altogether.
    // So we simply use the maximum possible value to be on the safe side
    // with these buggy drives.
    // We cannot use this as default since many firmwares fail with a too high data length.
    //
    if( (dataLen-8) % 8 || dataLen <= 8 )
        dataLen = 0xFFFF;

    // again with real length
    data.resize( dataLen );
    ::memset( data.data(), 0, data.size() );

    cmd[7] = data.size() >> 8;
    cmd[8] = data.size();
    if( cmd.transport( TR_DIR_READ, data.data(), data.size() ) == 0 ) {
        data.resize( qMin( data.size(), (int)Device::from4Byte( data.data() ) + 4 ) );
        return true;
    }
    else {
        qDebug() << "(Device::Device) " << device() << ": GET CONFIGURATION with real length "
                 << data.size() << " failed.";
        qDebug();
        data.clear();
        return false;
    }
}

bool ExtendDataCDRom::readDvdStructure(UByteArray &data, unsigned int format, unsigned int layer, unsigned long address, unsigned int agid)
{
    return readDiscStructure( data, 0x0, format, layer, address, agid );
}

bool ExtendDataCDRom::readDiscStructure(UByteArray &data, unsigned int mediaType, unsigned int format, unsigned int layer, unsigned long address, unsigned int agid)
{
    unsigned char header[4];
    ::memset( header, 0, 4 );

    DiscScsi cmd(m_oBlockName);

    cmd[0] = MMC_READ_DVD_STRUCTURE;
    cmd[1] = mediaType & 0xF;
    cmd[2] = address>>24;
    cmd[3] = address>>16;
    cmd[4] = address>>8;
    cmd[5] = address;
    cmd[6] = layer;
    cmd[7] = format;
    cmd[10] = (agid<<6);
    cmd[11] = 0;      // Necessary to set the proper command length

    cmd[9] = 4;
    if( cmd.transport( TR_DIR_READ, header, 4 ) == 0 ) {
        // again with real length
        unsigned int dataLen = Device::from2Byte( header ) + 2;

        data.resize( dataLen );
        ::memset( data.data(), 0, data.size() );

        cmd[8] = data.size() >> 8;
        cmd[9] = data.size();
        if( cmd.transport( TR_DIR_READ, data.data(), data.size() ) == 0 ) {
            data.resize( qMin( data.size(), Device::from2Byte( data.data() ) + 2 ) );
            return true;
        }
        else {
            qDebug() << "(Device::Device) " << device() << ": READ DVD STRUCTURE with real length failed.";
            data.clear();
            return false;
        }
    }
    else {
        qDebug() << "(Device::Device) " << device() << ": READ DVD STRUCTURE length det failed";
        return false;
    }
}

bool ExtendDataCDRom::readTocPmaAtip(UByteArray &data, int format, bool msf, int track)
{
    unsigned int descLen = 0;

    switch( format ) {
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
    ::memset( header, 0, 2048 );

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_READ_TOC_PMA_ATIP;
    cmd[1] = ( time ? 0x2 : 0x0 );
    cmd[2] = format & 0x0F;
    cmd[6] = track;
    cmd[8] = 4;
    cmd[9] = 0;      // Necessary to set the proper command length

    // we only read the header
    unsigned int dataLen = 4;
    if( cmd.transport( TR_DIR_READ, header, 4 ) == 0 )
        dataLen = Device::from2Byte( header ) + 2;
    else
        qDebug() << "(Device::Device) " << device() << ": READ TOC/PMA/ATIP length det failed.";

    //
    // Some buggy firmwares return an invalid size here
    // So we simply use the maximum possible value to be on the safe side
    // with these buggy drives.
    // We cannot use this as default since many firmwares fail with a too high data length.
    //
    if( descLen != 0 && ((dataLen-4) % descLen || dataLen < 4+descLen) ) {
        qDebug() << "(Device::Device) " << device() << ": READ TOC/PMA/ATIP invalid length returned: " << dataLen;
        dataLen = 0xFFFF;
    }

    //
    // Not all drives like uneven numbers
    //
    if( dataLen%2 )
        ++dataLen;

    // again with real length
    data.resize( dataLen );
    ::memset( data.data(), 0, data.size() );

    cmd[7] = data.size() >> 8;
    cmd[8] = data.size();
    if( cmd.transport( TR_DIR_READ, data.data(), data.size() ) == 0 ) {
        dataLen = qMin( dataLen, Device::from2Byte( data.data() ) + 2u );
        if( descLen == 0 || (dataLen-4) % descLen || dataLen < 4+descLen ) {
            // useless length
            data.clear();
            return false;
        }
        else {
            data.resize( dataLen );
            return true;
        }
    }
    else {
        qDebug() << "(Device::Device) " << device() << ": READ TOC/PMA/ATIP format "
                 << format << " with real length "
                 << data.size() << " failed." << endl;
        data.clear();
        return false;
    }
}

bool ExtendDataCDRom::readFormatCapacity(int wantedFormat, Msf &r, Msf *currentMax, int *currentMaxFormat)
{
    bool success = false;

    // the maximal length as stated in MMC4
    static const unsigned int maxLen = 4 + (8*32);

    unsigned char buffer[maxLen];
    ::memset( buffer, 0, maxLen );

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_READ_FORMAT_CAPACITIES;
    cmd[7] = maxLen >> 8;
    cmd[8] = maxLen & 0xFF;
    cmd[9] = 0;      // Necessary to set the proper command length
    if( cmd.transport( TR_DIR_READ, buffer, maxLen ) == 0 ) {

        unsigned int realLength = buffer[3] + 4;

        qDebug() << "(Device::Device) " << device() << " READ FORMAT CAPACITY: Current/Max "
                 << (int)(buffer[8]&0x3) << " " << Device::from4Byte( &buffer[4] ) << endl;

        if( currentMax )
            *currentMax = Device::from4Byte( &buffer[4] );
        if( currentMaxFormat )
            *currentMaxFormat = (int)(buffer[8]&0x3);

        //
        // Descriptor Type:
        // 0 - reserved
        // 1 - unformatted :)
        // 2 - formatted. Here we get the used capacity (lead-in to last lead-out/border-out)
        // 3 - No media present
        //
        for( unsigned int i = 12; i < realLength-4; i+=8 ) {
            int format = (int)((buffer[i+4]>>2)&0x3f);
            qDebug() << "(Device::Device) " << device() << " READ FORMAT CAPACITY: "
                     << format << " " << Device::from4Byte( &buffer[i] )
                     << " " << (int)( (buffer[i+5] << 16 & 0xFF0000) |
                                      (buffer[i+6] << 8  & 0xFF00) |
                                      (buffer[i+7]       & 0xFF) ) << endl;

            if( format == wantedFormat ) {
                // found the descriptor
                r = qMax( (int)Device::from4Byte( &buffer[i] ), r.lba() );
                success = true;
            }
        }
    }

    return success;
}

bool ExtendDataCDRom::readTrackInformation(UByteArray &data, int type, int value)
{
    unsigned char header[2048];
    ::memset( header, 0, 2048 );

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_READ_TRACK_INFORMATION;
    cmd[9] = 0;      // Necessary to set the proper command length

    switch( type ) {
    case 0:
    case 1:
    case 2:
        cmd[1] = type & 0x3;
        cmd[2] = value>>24;
        cmd[3] = value>>16;
        cmd[4] = value>>8;
        cmd[5] = value;
        break;
    default:
        qDebug() << "(Device::readTrackInformation) wrong type parameter: " << type;
        return false;
    }

    // first we read the header
    unsigned int dataLen = 4;
    cmd[8] = 4;
    if( cmd.transport( TR_DIR_READ, header, 4 ) == 0 )
        dataLen = Device::from2Byte( header ) + 2;
    else
        qDebug() << "(Device::Device) " << device() << ": READ TRACK INFORMATION length det failed.";

    //
    // Some buggy firmwares do not return the size of the available data
    // but the returned data.
    // So we try to determine the correct size based on the medium type
    // DVD+R:  40 (MMC4)
    // DVD-DL: 48 (MMC5)
    // CD:     36 (MMC2)
    //
    if( dataLen <= 6 ) {
        int m = mediaType();
        if( m & (Device::MEDIA_DVD_R_DL|Device::MEDIA_DVD_R_DL_SEQ|Device::MEDIA_DVD_R_DL_JUMP) )
            dataLen = 48;
        else if( m & (Device::MEDIA_DVD_PLUS_R|Device::MEDIA_DVD_PLUS_R_DL) )
            dataLen = 40;
        else
            dataLen = 36;
    }

    // again with real length
    data.resize( dataLen );
    ::memset( data.data(), 0, data.size() );

    cmd[7] = data.size() >> 8;
    cmd[8] = data.size();
    if( cmd.transport( TR_DIR_READ, data.data(), data.size() ) == 0 ) {
        data.resize( qMin( data.size(), Device::from2Byte( data.data() ) + 2 ) );
        return true;
    }
    else {
        qDebug() << "(Device::Device) " << device() << ": READ TRACK INFORMATION with real length "
                 << data.size() << " failed." << endl;
        data.clear();
        return false;
    }
}

bool ExtendDataCDRom::readCapacity(Msf &r)
{
    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_READ_CAPACITY;
    cmd[9] = 0;      // Necessary to set the proper command length
    unsigned char buf[8];
    ::memset( buf, 0, 8 );
    if( cmd.transport( TR_DIR_READ, buf, 8 ) == 0 ) {
        r = Device::from4Byte( buf );
        return true;
    }
    else
        return false;
}

Toc ExtendDataCDRom::readToc()
{
    bool needToClose = !isOpen();

    Toc toc;

    if( !open() )
        return toc;

    int mt = mediaType();

    //
    // Use the profile if available because DVD-ROM units need to treat DVD+-R(W) media as DVD-ROM
    // if supported at all
    //
    if( currentProfile() == Device::MEDIA_DVD_ROM )
        mt = Device::MEDIA_DVD_ROM;

    if( mt & (Device::MEDIA_DVD_MINUS_ALL|Device::MEDIA_DVD_PLUS_RW|Device::MEDIA_DVD_ROM) ) {
        if( !readFormattedToc( toc, mt ) ) {
            Msf size;
            if( readCapacity( size ) ) {
                Track track;
                track.setFirstSector( 0 );
                track.setLastSector( size.lba() );
                track.setSession( 1 );
                track.setType( Track::TYPE_DATA );
                track.setMode( Track::DVD );
                track.setCopyPermitted( mt != Device::MEDIA_DVD_ROM );
                track.setPreEmphasis( mt != Device::MEDIA_DVD_ROM );

                toc.append( track );
            }
            else
                qDebug() << "(Device::Device) " << device()
                         << "READ CAPACITY for toc failed." << endl;
        }
    }

    else if( mt & (Device::MEDIA_DVD_PLUS_R|Device::MEDIA_DVD_PLUS_R_DL) ) {
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
        readFormattedToc( toc, mt );
    }

    else if( mt & Device::MEDIA_BD_ALL ) {
        readFormattedToc( toc, mt );
    }

    else if( mt == Device::MEDIA_DVD_RAM ) {
        qDebug() << "(Device::readDvdToc) no dvdram support";
    }


    else if( mt & Device::MEDIA_CD_ALL ) {
        bool success = readRawToc( toc );
        if( !success ) {
            success = readFormattedToc( toc, mt );

            if( !success ) {
                qDebug() << "(Device::Device) MMC READ TOC failed. falling back to cdrom.h.";
                readTocLinux(toc);
            }

            if( success )
                fixupToc( toc );
        }
    }

    if( needToClose )
        close();

    return toc;
}

bool ExtendDataCDRom::readFormattedToc(Toc &toc, int mt)
{
    // if the device is already opened we do not close it
    // to allow fast multiple method calls in a row
    bool needToClose = !isOpen();

    bool success = false;

    toc.clear();

    int lastTrack = 0;

    UByteArray data;
    if( !(mt & Device::MEDIA_CD_ALL) ) {
        //
        // on DVD-R(W) multisession disks only two sessions are represented as tracks in the readTocPmaAtip
        // response (fabricated TOC). Thus, we use readDiscInformation for DVD media to get the proper number of tracks
        //
        if( readDiscInformation( data ) ) {
            lastTrack = (int)( data[11]<<8 | data[6] );

            if( readTrackInformation( data, 1, lastTrack ) ) {
                track_info_t* trackInfo = (track_info_t*)data.data();

                if( trackInfo->blank ) {
                    lastTrack--;
                }

                success = true;
            }
            else
                return false;
        }
        else
            return false;
    }
    else {
        if( readTocPmaAtip( data, 0, 0, 1 ) ) {

            if( data.size() < 4 ) {
                qDebug() << "(Device::Device) " << device() << ": formatted toc data too small.";
            }
            else if( data.size() != ( (int)sizeof(toc_track_descriptor) * ((int)data[3]+1) ) + 4 ) {
                qDebug() << "(Device::Device) " << device() << ": invalid formatted toc data length: "
                         << (data.size()-2) << endl;
            }
            else {
                lastTrack = data[3];
                toc_track_descriptor* td = (toc_track_descriptor*)&data[4];
                for( int i = 0; i < lastTrack; ++i ) {

                    Track track;
                    unsigned int control = 0;

                    //
                    // In case READ TRACK INFORMATION fails:
                    // no session number info
                    // no track length and thus possibly incorrect last sector for
                    // multisession disks
                    //
                    track.setFirstSector( Device::from4Byte( td[i].start_adr ) );
                    track.setLastSector( Device::from4Byte( td[i+1].start_adr ) - 1 );
                    control = td[i].control;

                    track.setType( (control & 0x4) ? Track::TYPE_DATA : Track::TYPE_AUDIO );
                    track.setMode( getTrackDataMode( track ) );
                    track.setCopyPermitted( control & 0x2 );
                    track.setPreEmphasis( control & 0x1 );

                    toc.append( track );
                }

                success = true;
            }
        }
    }


    //
    // Try to get information for all the tracks
    //
    for( int i = 0; i < lastTrack; ++i ) {
        if( toc.count() < i+1 )
            toc.append( Track() );

        UByteArray trackData;
        if( readTrackInformation( trackData, 1, i+1 ) ) {
            track_info_t* trackInfo = (track_info_t*)trackData.data();

            toc[i].setFirstSector( Device::from4Byte( trackInfo->track_start ) );

            if( i > 0 && toc[i-1].lastSector() == 0 )
                toc[i-1].setLastSector( toc[i].firstSector() - 1 );

            // There are drives that return 0 track length here!
            // Some drives even return an invalid length here. :(
            if( Device::from4Byte( trackInfo->track_size ) > 0 )
                toc[i].setLastSector( toc[i].firstSector() + Device::from4Byte( trackInfo->track_size ) - 1 );

            if( trackInfo->nwa_v ) {
                toc[i].setNextWritableAddress( Device::from4Byte( trackInfo->next_writable ) );
                toc[i].setFreeBlocks( Device::from4Byte( trackInfo->free_blocks ) );
            }

            toc[i].setSession( (int)((trackInfo->session_number_m<<8 & 0xf0) |
                                     (trackInfo->session_number_l & 0x0f)) );  //FIXME: is this BCD?

            int control = trackInfo->track_mode;

            if( mt & Device::MEDIA_CD_ALL ) {
                toc[i].setType( (control & 0x4) ? Track::TYPE_DATA : Track::TYPE_AUDIO );
                toc[i].setMode( getTrackDataMode( toc[i] ) );
            }
            else {
                toc[i].setType( Track::TYPE_DATA );
                toc[i].setMode( Track::DVD );
            }
            toc[i].setCopyPermitted( control & 0x2 );
            toc[i].setPreEmphasis( control & 0x1 );
        }
        else if( !(mt & Device::MEDIA_CD_ALL) ) {
            success = false;
        }
    }

    // this can only happen with DVD media
    if( !toc.isEmpty() && toc.last().lastSector() == 0 ) {
        qDebug() << "(Device::Device) " << device() << " no track length for the last non-empty track.";
        UByteArray trackData;
        if( readTrackInformation( trackData, 1, lastTrack+1 ) ) {
            track_info_t* trackInfo = (track_info_t*)trackData.data();

            toc.last().setLastSector( Device::from4Byte( trackInfo->track_start ) - 1 );
        }
    }


    if( needToClose )
        close();

    return success;
}

bool ExtendDataCDRom::readRawToc(Toc &toc)
{
    // if the device is already opened we do not close it
    // to allow fast multiple method calls in a row
    bool needToClose = !isOpen();

    bool success = false;

    toc.clear();

    if( open() ) {
        //
        // Read Raw TOC (format: 0010b)
        //
        // For POINT from 01h-63h we get all the tracks
        // POINT a1h gices us the last track number in the session in PMIN
        // POINT a2h gives the start of the session lead-out in PMIN,PSEC,PFRAME
        //

        UByteArray data;

        if( readTocPmaAtip( data, 2, false, 1 ) ) {
            if( data.size() > 4 ) {
                success = true;

                toc_raw_track_descriptor* tr = (toc_raw_track_descriptor*)&data[4];

                //
                // debug the raw toc data
                //
                qDebug() << "Session |  ADR   | CONTROL|  TNO   | POINT  |  Min   |  Sec   | Frame  |  Zero  |  PMIN  |  PSEC  | PFRAME |";
                for( int i = 0; i < (data.size()-4)/(int)sizeof(toc_raw_track_descriptor); ++i ) {
                    QString s;
                    s += QString( " %1 |" ).arg( (int)tr[i].session_number, 6 );
                    s += QString( " %1 |" ).arg( (int)tr[i].adr, 6 );
                    s += QString( " %1 |" ).arg( (int)tr[i].control, 6 );
                    s += QString( " %1 |" ).arg( (int)tr[i].tno, 6 );
                    s += QString( " %1 |" ).arg( (int)tr[i].point, 6, 16 );
                    s += QString( " %1 |" ).arg( (int)tr[i].min, 6 );
                    s += QString( " %1 |" ).arg( (int)tr[i].sec, 6 );
                    s += QString( " %1 |" ).arg( (int)tr[i].frame, 6 );
                    s += QString( " %1 |" ).arg( (int)tr[i].zero, 6, 16 );
                    s += QString( " %1 |" ).arg( (int)tr[i].p_min, 6 );
                    s += QString( " %1 |" ).arg( (int)tr[i].p_sec, 6 );
                    s += QString( " %1 |" ).arg( (int)tr[i].p_frame, 6 );
                    qDebug() << s;
                }

                //
                // First we try to determine if the raw toc data uses BCD values
                //
                int isBcd = rawTocDataWithBcdValues( data );
                if( isBcd == -1 ) {
                    return false;
                }

                Msf sessionLeadOut;

                for( unsigned int i = 0; i < (data.size()-4)/(unsigned int)sizeof(toc_raw_track_descriptor); ++i ) {
                    if( tr[i].adr == 1 && tr[i].point <= 0x63 ) {
                        // track
                        Track track;
                        track.setSession( tr[i].session_number );

                        // :( We use 00:00:00 == 0 lba)
                        if( isBcd )
                            track.setFirstSector( Msf( Device::fromBcd(tr[i].p_min),
                                                       Device::fromBcd(tr[i].p_sec),
                                                       Device::fromBcd(tr[i].p_frame) ) - 150 );
                        else
                            track.setFirstSector( Msf( tr[i].p_min, tr[i].p_sec, tr[i].p_frame ) - 150 );

                        track.setType( tr[i].control & 0x4 ? Track::TYPE_DATA : Track::TYPE_AUDIO );
                        track.setMode( track.type() == Track::TYPE_DATA ? getTrackDataMode(track) : Track::UNKNOWN );
                        track.setCopyPermitted( tr[i].control & 0x2 );
                        track.setPreEmphasis( tr[i].control & 0x1 );

                        //
                        // only do this within a session because otherwise we already set the last sector with the session leadout
                        //
                        if( !toc.isEmpty() )
                            if( toc[toc.count()-1].session() == track.session() )
                                toc[toc.count()-1].setLastSector( track.firstSector() - 1 );

                        toc.append(track);
                    }
                    else if( tr[i].point == 0xa2 ) {
                        //
                        // since the session is always reported before the tracks this is where we do this:
                        // set the previous session's last tracks's last sector to the first sector of the
                        // session leadout (which was reported before the tracks)
                        //
                        // This only happens on multisession CDs
                        //
                        if( !toc.isEmpty() )
                            toc[toc.count()-1].setLastSector( sessionLeadOut - 1 );

                        // this is save since the descriptors are reported in ascending order of the session number
                        // :( We use 00:00:00 == 0 lba)
                        if( isBcd )
                            sessionLeadOut = Msf( Device::fromBcd(tr[i].p_min),
                                                  Device::fromBcd(tr[i].p_sec),
                                                  Device::fromBcd(tr[i].p_frame) ) - 150;
                        else
                            sessionLeadOut = Msf( tr[i].p_min, tr[i].p_sec, tr[i].p_frame ) - 150;
                    }
                }

                qDebug() << device() << ": setting last sector of last track to " << (sessionLeadOut-1).lba();

                // set the last track's last sector
                if( !toc.isEmpty() )
                    toc[toc.count()-1].setLastSector( sessionLeadOut - 1 );
            }
            else
                qDebug() << "(Device::Device) " << device() << " empty raw toc.";
        }
    }

    if( needToClose )
        close();

    return success;
}

bool ExtendDataCDRom::readTocLinux(Toc &toc)
{
    // if the device is already opened we do not close it
    // to allow fast multiple method calls in a row
    bool needToClose = !isOpen();

    bool success = true;

    toc.clear();

    struct cdrom_tochdr tochdr;
    struct cdrom_tocentry tocentry;

    if( open() ) {
        //
        // CDROMREADTOCHDR ioctl returns:
        // cdth_trk0: First Track Number
        // cdth_trk1: Last Track Number
        //
        if( ::ioctl( m_iHandle, CDROMREADTOCHDR, &tochdr ) ) {
            qDebug() << "(Device) could not get toc header !";
            success = false;
        }
        else {
            Track lastTrack;
            for (int i = tochdr.cdth_trk0; i <= tochdr.cdth_trk1 + 1; i++) {
                ::memset(&tocentry,0,sizeof (struct cdrom_tocentry));
                // get Lead-Out Information too
                tocentry.cdte_track = (i<=tochdr.cdth_trk1) ? i : CDROM_LEADOUT;
                tocentry.cdte_format = CDROM_LBA;

                if( ::ioctl( m_iHandle, CDROMREADTOCENTRY, &tocentry ) ) {
                    qDebug() << "(Device) error reading tocentry " << i;
                    success = false;
                    break;
                }

                int startSec = tocentry.cdte_addr.lba;
                int control  = tocentry.cdte_ctrl & 0x0f;
                int mode     = tocentry.cdte_datamode;
                if( i > tochdr.cdth_trk0 ) {
                    Track track( lastTrack.firstSector(), startSec-1, lastTrack.type(), lastTrack.mode() );
                    track.setPreEmphasis( control & 0x1 );
                    track.setCopyPermitted( control & 0x2 );
                    toc.append( track );
                }
                Track::TrackType trackType = Track::TYPE_UNKNOWN;
                Track::DataMode trackMode = Track::UNKNOWN;
                if( (control & 0x04 ) && (tocentry.cdte_track != CDROM_LEADOUT) ) {
                    trackType = Track::TYPE_DATA;
                    if( mode == 1 )
                        trackMode = Track::MODE1;
                    else if( mode == 2 )
                        trackMode = Track::MODE2;

                    Track::DataMode tm = getDataMode(startSec);
                    if( tm != Track::UNKNOWN )
                        trackMode = tm;
                }
                else
                    trackType = Track::TYPE_AUDIO;

                lastTrack = Track( startSec, startSec, trackType, trackMode );
            }
        }

        if( needToClose )
            close();
    }
    else
        success = false;

    return success;
}

bool ExtendDataCDRom::fixupToc(Toc &toc)
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
    if( numSessions() > 1 || toc.contentType() == Device::MIXED ) {
        qDebug() << "(Device::Device) fixup multisession toc...";

        //
        // we need to update the last sector of every last track in every session
        // for now we only update the track before the last session...
        // This is the most often case: Advanced Audio CD
        //

        UByteArray data;
        if( readTocPmaAtip( data, 1, false, 0 ) ) {

            //
            // data[6]    - first track number in last complete session
            // data[8-11] - start address of first track in last session
            //

            toc[(unsigned int)data[6]-2].setLastSector( Device::from4Byte( &data[8] ) - 11400 - 1 );

            success = true;
        }
        else
            qDebug() << "(Device::Device) FIXUP TOC failed.";
    }

    return success;
}

Track::DataMode ExtendDataCDRom::getTrackDataMode(const Track &track)
{
    return getDataMode( track.firstSector() );
}

Track::DataMode ExtendDataCDRom::getDataMode(const Msf &sector)
{
    bool needToClose = !isOpen();

    Track::DataMode ret = Track::UNKNOWN;

    if( !open() )
        return ret;

    // we use readCdMsf here since it's defined mandatory in MMC1 and
    // we only use this method for CDs anyway
    unsigned char data[2352];
    bool readSuccess = readCdMsf( data, 2352,
                                  0,      // all sector types
                                  false,  // no dap
                                  sector,
                                  sector+1,
                                  true, // SYNC
                                  true, // HEADER
                                  true, // SUBHEADER
                                  true, // USER DATA
                                  true, // EDC/ECC
                                  0,    // no c2 info
                                  0 );

    if( readSuccess ) {
        if ( data[15] == 0x1 )
            ret = Track::MODE1;
        else if ( data[15] == 0x2 )
            ret = Track::MODE2;
        if ( ret == Track::MODE2 ) {
            if ( data[16] == data[20] &&
                 data[17] == data[21] &&
                 data[18] == data[22] &&
                 data[19] == data[23] ) {
                if ( data[18] & 0x20 )
                    ret = Track::XA_FORM2;
                else
                    ret = Track::XA_FORM1;
            }
        }
    }

    if( needToClose )
        close();

    return ret;
}

int ExtendDataCDRom::rawTocDataWithBcdValues(const UByteArray &data)
{
    toc_raw_track_descriptor* tr = (toc_raw_track_descriptor*)&data[4];

    bool notBcd = false;
    bool notHex = false;

    //
    // in most cases this will already tell us if a drive does not provide bcd numbers
    // (which should be all newer MMC drives)
    //
    for( unsigned int i = 0; i < (data.size()-4)/(unsigned int)sizeof(toc_raw_track_descriptor); ++i ) {
        if( tr[i].adr == 1 && tr[i].point <= 0xa2) {
            if( !Device::isValidBcd(tr[i].p_min) ||
                !Device::isValidBcd(tr[i].p_sec) ||
                !Device::isValidBcd(tr[i].p_frame) ) {
                notBcd = true;
                break;
            }

            // we only need to check sec and frame since min needs to be <= 99
            // and bcd values are never bigger than 99.
            else if( (int)Device::fromBcd(tr[i].p_sec) >= 60 ||
                     (int)Device::fromBcd(tr[i].p_frame) >= 75 ) {
                notBcd = true;
                break;
            }
        }
    }


    //
    // all values are valid bcd values but we still don't know for sure if they are really
    // used as bcd. So we also check the HEX values.
    //
    for( unsigned int i = 0; i < (data.size()-4)/(unsigned int)sizeof(toc_raw_track_descriptor); ++i ) {
        if( tr[i].adr == 1 && tr[i].point <= 0xa2 ) {
            if( (int)tr[i].p_min > 99 ||
                (int)tr[i].p_sec >= 60 ||
                (int)tr[i].p_frame >= 75 ) {
                notHex = true;
                break;
            }
        }
    }


    //
    // If all values are valid bcd and valid hex we check the start sectors of the tracks.
    //
    if( !notHex || !notBcd ) {
        Msf sessionLeadOutHex, sessionLeadOutBcd;
        Msf lastTrackHex, lastTrackBcd;

        for( unsigned int i = 0; i < (data.size()-4)/(unsigned int)sizeof(toc_raw_track_descriptor); ++i ) {

            if( tr[i].adr == 1 ) {
                if( tr[i].point < 0x64 ) {

                    // check hex values
                    if( Msf( tr[i].p_min, tr[i].p_sec, tr[i].p_frame ) < lastTrackHex )
                        notHex = true;

                    // check bcd values
                    if( Msf( Device::fromBcd(tr[i].p_min), Device::fromBcd(tr[i].p_sec), Device::fromBcd(tr[i].p_frame) ) < lastTrackBcd )
                        notBcd = true;

                    lastTrackBcd = Msf( Device::fromBcd(tr[i].p_min), Device::fromBcd(tr[i].p_sec), Device::fromBcd(tr[i].p_frame) );
                    lastTrackHex = Msf( tr[i].p_min, tr[i].p_sec, tr[i].p_frame );
                }
                else if( tr[i].point == 0xa2 ) {
                    if( sessionLeadOutHex < lastTrackHex )
                        notHex = true;
                    if( sessionLeadOutBcd < lastTrackBcd )
                        notBcd = true;

                    sessionLeadOutHex = Msf( tr[i].p_min, tr[i].p_sec, tr[i].p_frame );
                    sessionLeadOutBcd = Msf( Device::fromBcd(tr[i].p_min), Device::fromBcd(tr[i].p_sec), Device::fromBcd(tr[i].p_frame) );
                }
            }
        }

        // check the last track
        if( sessionLeadOutHex < lastTrackHex )
            notHex = true;
        if( sessionLeadOutBcd < lastTrackBcd )
            notBcd = true;
    }


    if( !notBcd && !notHex ) {
        qDebug() << "(Device::Device) need to compare raw toc to formatted toc. :(";
        //
        // All values are valid bcd and valid HEX values so we compare with the formatted toc.
        // This slows us down a lot but in most cases this should not be reached anyway.
        //
        // TODO: also check the bcd values
        //
        Toc formattedToc;
        if( readFormattedToc( formattedToc, Device::MEDIA_CD_ROM ) ) {
            for( unsigned int i = 0; i < (data.size()-4)/(unsigned int)sizeof(toc_raw_track_descriptor); ++i ) {
                if( tr[i].adr == 1 && tr[i].point < 0x64 ) {
                    unsigned int track = (int)tr[i].point;

                    // FIXME: do bcd drive also encode the track number in bcd? If so test it, too.

                    if( ( int )track > formattedToc.count() ) {
                        notHex = true;
                        break;
                    }

                    Msf posHex( tr[i].p_min,
                                tr[i].p_sec,
                                tr[i].p_frame );
                    Msf posBcd( Device::fromBcd(tr[i].p_min),
                                Device::fromBcd(tr[i].p_sec),
                                Device::fromBcd(tr[i].p_frame) );
                    posHex -= 150;
                    posBcd -= 150;
                    if( posHex != formattedToc[track-1].firstSector() )
                        notHex = true;
                    if( posBcd != formattedToc[track-1].firstSector() )
                        notBcd = true;
                }
            }
        }
    }

    if( notBcd )
        qDebug() << "(Device::Device) found invalid bcd values. No bcd toc.";
    if( notHex )
        qDebug() << "(Device::Device) found invalid hex values. No hex toc.";

    if( notBcd == notHex ) {
        qDebug() << "(Device::Device) unable to determine if hex (" << notHex << ") or bcd (" << notBcd << ").";
        if( !notHex ) {
            qDebug() << "Assuming hex encoding in favor of newer drives and the more reliable raw toc.";
            return 0;
        }
        return -1;
    }
    else if( notBcd )
        return 0;
    else
        return 1;
}

int ExtendDataCDRom::numSessions()
{
    //
    // Session Info
    // ============
    // Byte 0-1: Data Length
    // Byte   2: First Complete Session Number (Hex) - always 1
    // Byte   3: Last Complete Session Number (Hex)
    //

    int ret = -1;

    UByteArray data;

    int m = mediaType();
    if( m & Device::MEDIA_CD_ALL ) {
        //
        // Althought disk_info should get the real value without ide-scsi
        // I keep getting wrong values (the value is too high. I think the leadout
        // gets counted as session sometimes :()
        //
        if( readTocPmaAtip( data, 1, 0, 0 ) ) {
            ret = data[3];
        }
        else {
            qDebug() << "(Device::Device) " << device() << ": could not get session info !";
        }
    }
    else if ( m & ( Device::MEDIA_DVD_PLUS_RW|Device::MEDIA_DVD_RW_OVWR|Device::MEDIA_BD_RE ) ) {
        // fabricate value
        int e = isEmpty();
        return ( e == Device::STATE_COMPLETE ? 1 : 0 ); //?
    }
    else {
        if( readDiscInformation( data ) ) {
            ret = (int)( data[9]<<8 | data[4] );

            // do only count complete sessions
            if( (data[2]>>2) != 3 )
                ret--;
        }
    }

    return ret;
}

bool ExtendDataCDRom::testUnitReady()
{
    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_TEST_UNIT_READY;
    cmd[5] = 0; // Necessary to set the proper command length
    return( cmd.transport() == 0 );
}

bool ExtendDataCDRom::readCdMsf(unsigned char *data, unsigned int dataLen, int sectorType, bool dap, const Msf &startAdress, const Msf &endAdress, bool sync, bool header, bool subHeader, bool userData, bool edcEcc, int c2, int subChannel)
{
    ::memset( data, 0, dataLen );

    DiscScsi cmd(m_oBlockName);
    cmd[0] = MMC_READ_CD_MSF;
    cmd[1] = (sectorType<<2 & 0x1c) | ( dap ? 0x2 : 0x0 );
    cmd[3] = (startAdress+150).minutes();
    cmd[4] = (startAdress+150).seconds();
    cmd[5] = (startAdress+150).frames();
    cmd[6] = (endAdress+150).minutes();
    cmd[7] = (endAdress+150).seconds();
    cmd[8] = (endAdress+150).frames();
    cmd[9] = ( ( sync      ? 0x80 : 0x0 ) |
               ( subHeader ? 0x40 : 0x0 ) |
               ( header    ? 0x20 : 0x0 ) |
               ( userData  ? 0x10 : 0x0 ) |
               ( edcEcc    ? 0x8  : 0x0 ) |
               ( c2<<1 & 0x6 ) );
    cmd[10] = subChannel & 0x7;
    cmd[11] = 0;      // Necessary to set the proper command length

    if( cmd.transport( TR_DIR_READ, data, dataLen ) ) {
        qDebug() << "(Device::Device) " << device() << ": READ CD MSF failed!";
        return false;
    }
    else
        return true;
}

Device::MediaType ExtendDataCDRom::mediaType()
{
    Device::MediaType m = Device::MEDIA_UNKNOWN;

    if( testUnitReady() ) {

        int p = currentProfile();
        if ( p != -1 )
            m = ( Device::MediaType )p;

        if( m & (Device::MEDIA_UNKNOWN|Device::MEDIA_DVD_ROM|Device::MEDIA_HD_DVD_ROM) ) {
            //
            // We prefer the Device::MediaType as reported by the media since this way
            // even ROM drives may report the correct type of writable media.
            //

            // 4 bytes header + 2048 bytes layer descriptor
            UByteArray data;
            if( readDvdStructure( data ) ) {
                switch( data[4]&0xF0 ) {
                case 0x00: m = Device::MEDIA_DVD_ROM; break;
                case 0x10: m = Device::MEDIA_DVD_RAM; break;
                case 0x20: m = Device::MEDIA_DVD_R; break; // there seems to be no value for DVD-R DL, it reports DVD-R
                case 0x30: m = Device::MEDIA_DVD_RW; break;
                case 0x40: m = Device::MEDIA_HD_DVD_ROM; break;
                case 0x50: m = Device::MEDIA_HD_DVD_R; break;
                case 0x60: m = Device::MEDIA_HD_DVD_RAM; break;
                case 0x90: m = Device::MEDIA_DVD_PLUS_RW; break;
                case 0xA0: m = Device::MEDIA_DVD_PLUS_R; break;
                case 0xE0: m = Device::MEDIA_DVD_PLUS_R_DL; break;
                default:
                    qDebug() << "(Device::Device) unknown dvd media type: " << QString::number(data[4]&0xF0, 8);
                    break; // unknown
                }
            }
        }

        if( m & (Device::MEDIA_UNKNOWN | Device::MEDIA_BD_ROM) ) {
            //
            // We prefer the Device::MediaType as reported by the media since this way
            // even ROM drives may report the correct type of writable media.
            //

            UByteArray data;
            if( readDiscStructure( data, 1, 0 ) ) {
                if( data.size() > 4+12 &&
                    data[4+8] == 'B' &&  data[4+9] == 'D' ) {
                    switch( data[4+10] ) {
                    case 'O': m = Device::MEDIA_BD_ROM; break;
                    case 'W': m = Device::MEDIA_BD_RE; break;
                    case 'R': m = Device::MEDIA_BD_R; break;
                    }
                }
            }
        }

        //
        // Only old CD or DVD devices do not report a current profile
        // or report CD-ROM profile for all CD types
        //
        if( m & (Device::MEDIA_UNKNOWN|Device::MEDIA_CD_ROM) ) {
            UByteArray data;
            if( readTocPmaAtip( data, 4, false, 0 ) ) {
                if( (data[6]>>6)&1 )
                    m = Device::MEDIA_CD_RW;
                else
                    m = Device::MEDIA_CD_R;
            }
            else
                m = Device::MEDIA_CD_ROM;
        }
    }

    return m;
}

int ExtendDataCDRom::isEmpty()
{
    // if the device is already opened we do not close it
    // to allow fast multiple method calls in a row
    bool needToClose = !isOpen();

    int ret = Device::STATE_UNKNOWN;
    if( !open() )
        return Device::STATE_UNKNOWN;

    if( !testUnitReady() )
        return Device::STATE_NO_MEDIA;

    UByteArray data;

    if( readDiscInformation( data ) ) {
        disc_info_t* inf = (disc_info_t*)data.data();
        switch( inf->status ) {
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

    if( needToClose )
        close();

    return ret;
}

bool ExtendDataCDRom::update()
{
    connect(g_user_signal, &GlobalUserSignal::changeDiscStatus, this, &DataCDROM::receiveDiscStatusChange,Qt::UniqueConnection);
    use();
    QString blockName = m_oBlockName;
    int cnt = 3;
    if (!m_bCDROM)
    {
        release();
        LOGGER->info(QString("DataCDROM::initDevice Line:%1 非CDROM，退出，dev = %2").arg(__LINE__).arg(blockName));
        return false;
    }

    destroyMedium();

    m_oMediumType.clear();
    // real cdrom, load information by block name
    // 0. ready cdrom.

    if (isOpen()){
        close();
    }

    while(cnt--) {
        if (!isOpen()) {
            if(!open()){
                LOGGER->warn(QString("( DataCDROM::update ) device = %1 , open device faild").arg(blockName));
            } else {
                LOGGER->info((QString("( DataCDROM::update ) device = %1 , open device success, cnt = %2").arg(blockName).arg(cnt)));
                break;
            }
        }
    }
    if(cnt <= 0 && !isOpen()) {
        LOGGER->info(QString("[%1] [%2] open:[%3] error 3 times").arg(__func__).arg(__LINE__).arg(blockName));
        qDebug() << QString("[%1] [%2] open:[%3] error 3 times").arg(__func__).arg(__LINE__).arg(blockName);
        release();
        return false;
    }
    // 1. vendor and descriptio
    display(blockName);
    // 2. load support mediu types.
    support();
    //checkCDROM();
    // 3. check have medium or not.
    checkMedium();
    close();
    release();
    LOGGER->info(QString("( DataCDROM::update ) Line:%1 运行完成，dev=%2").arg(__LINE__).arg(blockName));
    return true;

}

void ExtendDataCDRom::checkMedium()
{
    unsigned short profile = 0;
    unsigned int len = 0;
    unsigned char  cdb[12] = {0x46, // operation code
                              0x01, // RT : 0x10b
                              0x00, // fearture high
                              0x00, // fearture low
                              0x00, // reserved
                              0x00, // reserved
                              0x00, // reserved
                              0x00, // allocation length high
                              0x0C, // allocation length low
                              0};
    unsigned char result[65536] = {0};

    LOGGER->info(QString("( ExtendMediumType::checkMedium) Line:%1，进入函数 dev = %1").arg(__LINE__).arg(m_oBlockName));
    use();

    m_oMediumType.clear();

    if (doorIsOpen())
    {
        m_bEjected = true;
        m_oMediumType.clear();

        destroyMedium();

        LOGGER->info(_("No medium.Because the cdrom's door is open."));
        LOGGER->info(QString("( ExtendMediumType::checkMedium ) cdrom door has opend，prepare close device(%1), handle = %2").arg(m_oBlockName).arg(m_iHandle));
        if (isOpen()) {
            close();
        }
        eject(false);

        release();
        return;
    }
    m_bEjected = false;
    eject(true);
    if (!execSCSI(cdb, 12, result, 12))
    {
        release();
        LOGGER->warn(QString("( ExtendMediumType::checkMedium ) Line:%1 执行execSCSI失败返回").arg(__LINE__));
        return;
    }
    if (!(result[8] == cdb[2] && result[9] ==cdb[3]))
    {
        LOGGER->warn(QString("( ExtendMediumType::checkMedium ) Line:%1 execSCSI返回值不符合返回").arg(__LINE__));
        release();
        return;
    }
    profile = (result[6] << 8) | result[7];
    if (0x0000 == profile) // 未读取到定义类型
    {
        // 再次尝试读取全量的返回信息
        len = result[11];
        if (len > 65536) len = 65536;
        cdb[7] = ((len & 0xFF00) >> 8);
        cdb[8] = (len & 0x00FF);
        if (!execSCSI(cdb, 12, result, len))
        {
            release();
            LOGGER->warn(QString("( ExtendMediumType::checkMedium ) Line:%1 执行execSCSI失败返回").arg(__LINE__));
            return;
        }
        for (int i = 0; i < (int)len; ++i)
        {
            if (result[14 + i] & 0x01)
            {
                profile = (result[12 + i] << 8) | result[13 + i];
                break;
            }
        }
    }

    switch (profile)
    {
    case 0x0008: // CD-ROM
        if (m_u32MediumRSupport & DataMedium::MEDIUM_CD_ROM)
        {
            m_oMediumType = "CD-ROM";
            LOGGER->info(_("Loaded CD-ROM medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read CD-ROM medium"));
        break;
    case 0x0009: //CD-R
        if (m_u32MediumRSupport & DataMedium::MEDIUM_CD_R)
        {
            m_oMediumType = "CD-R";
            LOGGER->info(_("Loaded CD-R medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read CD-R medium"));
        break;
    case 0x000A: // CD-RW
        if (m_u32MediumRSupport & DataMedium::MEDIUM_CD_RW)
        {
            m_oMediumType = "CD-RW";
            LOGGER->info(_("Loaded CD-RW medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read CD-RW medium"));
        break;
    case 0x0010: // DVD-ROM
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_ROM)
        {
            m_oMediumType = "DVD-ROM";
            LOGGER->info(_("Loaded DVD-ROM medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD-ROM medium"));
        break;
    case 0x0011: // DVD-R
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_R)
        {
            m_oMediumType = "DVD-R";
            LOGGER->info(_("Loaded DVD-R medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD-R medium"));
        break;
    case 0x0012: // DVD-RAM
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_RAM)
        {
            m_oMediumType = "DVD-RAM";
            LOGGER->info(_("Loaded DVD-RAM medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD-RAM medium"));
        break;
    case 0x0013: // DVD-RW
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_RW_OVERWRITE)
        {
            m_oMediumType = "DVD-RW";
            LOGGER->info(_("Loaded DVD-RW(OVERWRITE) medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD-RW(OVERWRITE) medium"));
        break;
    case 0x0014: // DVD-RW
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_RW_SEQ)
        {
            m_oMediumType = "DVD-RW";
            LOGGER->info(_("Loaded DVD-RW medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD-RW medium"));
        break;
    case 0x001A: // DVD+RW
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_PLUS_RW)
        {
            m_oMediumType = "DVD+RW";
            LOGGER->info(_("Loaded DVD+RW medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD+RW medium"));
        break;
    case 0x001B: // DVD+R
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_PLUS_R)
        {
            m_oMediumType = "DVD+R";
            LOGGER->info(_("Loaded DVD+R medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD+R medium"));
        break;
    case 0x002B: // DVD+R DL
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_PLUS_R_DL)
        {
            m_oMediumType = "DVD+R DL";
            LOGGER->info(_("Loaded DVD+R DL medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD+R DL medium"));
        break;
    case 0x40:
        if (m_u32MediumRSupport & DataMedium::MEDIA_BD_ROM)
        {
            m_oMediumType = "BD-ROM";
            LOGGER->info(_("Loaded BD-ROM medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read BD-ROM medium"));
        break;
    case 0x41:
        if (m_u32MediumRSupport & DataMedium::MEDIA_BD_R_SRM) {
            m_oMediumType = "BD-R(SRM)";
            LOGGER->info(_("Loaded BD-R SRM medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read BD-R SRM medium"));
        break;
    case 0x42:
        if (m_u32MediumRSupport & DataMedium::MEDIA_BD_R_RRM)
        {
            m_oMediumType = "BD-R(RRM)";
            LOGGER->info(_("Loaded BD-R RRM medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read BD-R RRM medium"));
        break;
    default:
        //LOGGER->warn(_("To be supported in KylinBurner %1").arg(profile));
        break;
    }
    if (!m_oMediumType.isEmpty())
    {
        destroyMedium();

        //将new DataMedium 分为2步，先new后调用load()，防止出现类A中调用类A的成员变量造成的错误
        m_oMedium = new ExtendMediumType(m_oMediumType, m_oBlockName, this);
        dynamic_cast<ExtendMediumType*>(m_oMedium)->load();

        LOGGER->info(_("Created medium of type %1.").arg(m_oMediumType));
        release();
        return;
    }
    LOGGER->debug(_("There is no mediun in CDROM"));
    release();
    LOGGER->info(QString("( ExtendMediumType::checkMedium ) 运行完成 dev = %1").arg(m_oBlockName));
}

DataCDROM *ExtendDataCDRom::clone()
{
    ExtendDataCDRom *cdrom = new ExtendDataCDRom(*this, this->m_oBlockName);
    cdrom->m_oSetting = new DataSetting(*this->m_oSetting,cdrom);
    cdrom->m_oMedium = new ExtendMediumType(*this->m_oMedium, cdrom);
    cdrom->m_iHandle = this->m_iHandle;
    cdrom->m_bSupportBD = this->m_bSupportBD;
    cdrom->m_u32MediumRSupport = this->m_u32MediumRSupport;
    cdrom->m_u32MediumWSupport = this->m_u32MediumWSupport;
    cdrom->m_u32BurnModeSupport = this->m_u32BurnModeSupport;
    cdrom->m_bEjected = this->m_bEjected;
    cdrom->m_bCDROM = this->m_bCDROM;
    cdrom->m_bAppRunning = this->m_bAppRunning;
    cdrom->m_oDisplay = this->m_oDisplay;
    cdrom->m_oVendor = this->m_oVendor;
    cdrom->m_oProductor = this->m_oVendor;
    cdrom->m_oBlockName = this->m_oBlockName;
    cdrom->m_oMediumType = this->m_oMediumType;
    cdrom->m_is_using = 1000; //防止光驱拔出后，触发release函数导致的析构函数
    connect(cdrom, &ExtendDataCDRom::updateCDROMBurnStart, this, &ExtendDataCDRom::slotsUpdateCDROMBurnStart);

    return cdrom;
}

Device::MediaState ExtendMediumType::diskState() const
{
    return m_diskState;
}

bool ExtendMediumType::empty() const
{
    return diskState() == Device::STATE_EMPTY;
}

bool ExtendMediumType::appendable() const
{
    return diskState() == Device::STATE_INCOMPLETE;
}

bool ExtendMediumType::rewritable() const
{
    return m_rewritable;
}

int ExtendMediumType::numSessions() const
{
    return m_numSessions;
}

int ExtendMediumType::numTracks() const
{
    return m_numTracks;
}

Device::MediaType ExtendMediumType::mediaType() const
{    
    Device::MediaType m = Device::MEDIA_UNKNOWN;

    if( m_new_cdrom->testUnitReady() ) {

        int p = m_new_cdrom->currentProfile();
        if ( p != -1 )
            m = ( Device::MediaType )p;

        if( m & (Device::MEDIA_UNKNOWN|Device::MEDIA_DVD_ROM|Device::MEDIA_HD_DVD_ROM) ) {
            //
            // We prefer the Device::MediaType as reported by the media since this way
            // even ROM drives may report the correct type of writable media.
            //

            // 4 bytes header + 2048 bytes layer descriptor
            UByteArray data;
            if( m_new_cdrom->readDvdStructure( data ) ) {
                switch( data[4]&0xF0 ) {
                case 0x00: m = Device::MEDIA_DVD_ROM; break;
                case 0x10: m = Device::MEDIA_DVD_RAM; break;
                case 0x20: m = Device::MEDIA_DVD_R; break; // there seems to be no value for DVD-R DL, it reports DVD-R
                case 0x30: m = Device::MEDIA_DVD_RW; break;
                case 0x40: m = Device::MEDIA_HD_DVD_ROM; break;
                case 0x50: m = Device::MEDIA_HD_DVD_R; break;
                case 0x60: m = Device::MEDIA_HD_DVD_RAM; break;
                case 0x90: m = Device::MEDIA_DVD_PLUS_RW; break;
                case 0xA0: m = Device::MEDIA_DVD_PLUS_R; break;
                case 0xE0: m = Device::MEDIA_DVD_PLUS_R_DL; break;
                default:
                    qDebug() << "(Device::Device) unknown dvd media type: " << QString::number(data[4]&0xF0, 8);
                    break; // unknown
                }
            }
        }

        if( m & (Device::MEDIA_UNKNOWN | Device::MEDIA_BD_ROM) ) {
            //
            // We prefer the Device::MediaType as reported by the media since this way
            // even ROM drives may report the correct type of writable media.
            //

            UByteArray data;
            if( m_new_cdrom->readDiscStructure( data, 1, 0 ) ) {
                if( data.size() > 4+12 &&
                        data[4+8] == 'B' &&  data[4+9] == 'D' ) {
                    switch( data[4+10] ) {
                    case 'O': m = Device::MEDIA_BD_ROM; break;
                    case 'W': m = Device::MEDIA_BD_RE; break;
                    case 'R': m = Device::MEDIA_BD_R; break;
                    }
                }
            }
        }

        //
        // Only old CD or DVD devices do not report a current profile
        // or report CD-ROM profile for all CD types
        //
        if( m & (Device::MEDIA_UNKNOWN|Device::MEDIA_CD_ROM) ) {
            UByteArray data;
            if( m_new_cdrom->readTocPmaAtip( data, 4, false, 0 ) ) {
                if( (data[6]>>6)&1 )
                    m = Device::MEDIA_CD_RW;
                else
                    m = Device::MEDIA_CD_R;
            }
            else
                m = Device::MEDIA_CD_ROM;
        }
    }

    return m;
}

Device::MediaState ExtendMediumType::lastSessionState() const
{
    return m_lastSessionState;
}

Device::BackGroundFormattingState ExtendMediumType::bgFormatState() const
{
    return m_bgFormatState;
}

void ExtendMediumType::secondFixDiscStatus()
{
    LOGGER->info(QString("(DataMedium::secondFixDiscStatus) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    if (!m_bAppRunning || !m_oCDROM || !m_discread) {
        LOGGER->warn(QString("(DataMedium::secondFixDiscStatus) Line:%1 medium is invalid, dev=%2").arg(__LINE__).arg(m_oBlockName));
        return;
    }
    ExtendDataCDRom *cdrom = qobject_cast<ExtendDataCDRom *>(m_oCDROM);
    if (!cdrom) {
        LOGGER->warn(QString("(DataMedium::secondFixDiscStatus) Line:%1 cdrom is invalid, dev=%2").arg(__LINE__).arg(m_oBlockName));
        return;
    }
    qDebug() << "m_discread->canAppend() = " << m_discread->canAppend();

    diskInfo();
    if (diskState() == Device::STATE_COMPLETE || diskState() == Device::STATE_INCOMPLETE) {
        toc = cdrom->readToc();
    }
    analyseContent();
    capacity();

    m_bUDF = m_discread->discFilesystemType().contains("udf") ? true : false;
    m_iNumDataTrack = m_discread->numDataTracks();

    if (m_discread->canAppend()) {
        m_u8Status = MEDIUM_INCOMPLETE;
        Q_EMIT g_user_signal->secondChangeDiscStatus();
        Q_EMIT g_user_signal->changeBtnBusnessName();// 修改mainwindow中btnBusness按钮的名字(与光盘状态有关)
        qDebug() << QString("[%1][%2] 挂载点已查到：[%3]  定时器已关，发送信号").arg(__func__).arg(__LINE__).arg(m_oMountPoint);
    }

    if (!m_oVolumeID.isEmpty()) {
        m_oCDROM->setSettingVolumnID(m_oVolumeID);
        m_oDisplay = _("%1 - %2 medium, left space : %3")
                .arg(m_oVolumeID)
                .arg(m_oMediumType).arg(m_oFreeCapacity);
    } else if (!m_discread->discLabel().isEmpty()) {
        m_oVolumeID = m_discread->discLabel();
        m_oCDROM->setSettingVolumnID(m_oVolumeID);
        m_oDisplay = _("%1 - %2 medium, left space : %3")
                .arg(m_oVolumeID.isEmpty() ? _("Empty medium") : m_oVolumeID)
                .arg(m_oMediumType).arg(m_oFreeCapacity);
    } else {
        m_oCDROM->setSettingVolumnID(_("KylinBurner"));
        m_oDisplay = _("%1 - %2 medium, left space : %3")
                .arg(m_oVolumeID.isEmpty() ? _("Empty medium") : _("KylinBurner"))
                .arg(m_oMediumType).arg(m_oFreeCapacity);
    }

    Q_EMIT g_user_signal->sigChangeDiscDisplayInfo((void*)m_oCDROM, m_oDisplay);
}

void ExtendMediumType::fixDiscStatus()
{
    LOGGER->info(QString("(DataMedium::fixDiscStatus) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    if (!m_bAppRunning || !m_oCDROM || !m_discread) {
        LOGGER->warn(QString("(DataMedium::fixDiscStatus) Line:%1 medium is invalid, dev=%2").arg(__LINE__).arg(m_oBlockName));
        return;
    }
    if (!m_discread->isSuccess()) {
        connect(this, &ExtendMediumType::secondGetDiscStatus, this, &ExtendMediumType::slotsSecondGetDiscStatus);
        Q_EMIT this->secondGetDiscStatus();
        return;
    }
    m_discread->disconnect(); //关闭connect,可能会有第二次，使用不同槽函数

    m_bUDF = m_discread->discFilesystemType().contains("udf") ? true : false;
    m_iNumDataTrack = m_discread->numDataTracks();

    if (m_discread->isBlank()){
        m_u8Status = MEDIUM_EMPTY;
    }else if (m_discread->canAppend()){
        m_u8Status = MEDIUM_INCOMPLETE;
    }else
        m_u8Status = MEDIUM_FINALIZED;

    //UDF功能
    EnableUDF();
    emit g_user_signal->changBurnFormat();

    if (m_u8Status == MEDIUM_EMPTY) {
        diskInfo();
        analyseContent();
        capacity();
    }
    if (m_mountPointTimer) {
        m_mountPointTimer->start(1000);
    }
}

void ExtendMediumType::slotsMountPoint()
{
    LOGGER->info(QString("( ExtendMediumType::slotsMountPoint ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    if (!m_bAppRunning || !m_oCDROM || !m_discread || !m_mountPointTimer) {
        LOGGER->warn(QString("( ExtendMediumType::slotsMountPoint ) Line:%1 medium is invalid, dev=%2").arg(__LINE__).arg(m_oBlockName));
        return;
    }

    if ((m_u8Status > MEDIUM_EMPTY && m_u8Status < MEDIUM_OTHER) &&
            (m_oMountPoint.isEmpty() || m_oMediumName.isEmpty()) &&
            m_bAppRunning)
    {
        FILE *fp = nullptr;
        struct mntent *mp = nullptr;

        fp = setmntent("/proc/mounts", "r");
        if (nullptr == fp)
        {
            LOGGER->error(_("Cannot load mount information.%1").arg(strerror(errno)));
            return;
        }
        while (nullptr != (mp = getmntent(fp))) {
            if (QString::fromUtf8(mp->mnt_fsname) == m_oBlockName)
            {
                m_oMountPoint = QString::fromUtf8(mp->mnt_dir);
                m_oMediumName = m_oMountPoint.split("/").takeLast();
                if (QString::fromUtf8(mp->mnt_type).contains("UDF") ||
                        QString::fromUtf8(mp->mnt_type).contains("udf"))
                {
                    m_bUDF = true;
                    LOGGER->debug(_("Medium %1 mounted by UDF").arg(m_oMountPoint));
                }

                connect(m_discread, &DiscRead::workFinished, this, &ExtendMediumType::secondFixDiscStatus);
                m_discread->work();

                m_mountPointTimer->stop();
                m_mountPointTimer->disconnect();

                LOGGER->info(QString("[%1][%2] 挂载点已查到：[%3]  定时器已关").arg(__func__).arg(__LINE__).arg(m_oMountPoint));
                break;
            }
        }
        endmntent(fp);
    } else {
        if (m_u8Status == MEDIUM_EMPTY) {
            m_mountPointTimer->stop();
            m_mountPointTimer->disconnect();
            LOGGER->info(QString("[%1][%2] 空光盘  定时器已关").arg(__func__).arg(__LINE__));
        }
    }

    if (!m_oVolumeID.isEmpty()) {
        m_oCDROM->setSettingVolumnID(m_oVolumeID);
        m_oDisplay = _("%1 - %2 medium, left space : %3")
                .arg(m_oVolumeID)
                .arg(m_oMediumType).arg(m_oFreeCapacity);
    } else if (!m_discread->discLabel().isEmpty()) {
        m_oVolumeID = m_discread->discLabel();
        m_oCDROM->setSettingVolumnID(m_oVolumeID);
        m_oDisplay = _("%1 - %2 medium, left space : %3")
                .arg(m_oVolumeID.isEmpty() ? _("Empty medium") : m_oVolumeID)
                .arg(m_oMediumType).arg(m_oFreeCapacity);
    } else {
        m_oCDROM->setSettingVolumnID(_("KylinBurner"));
        m_oDisplay = _("%1 - %2 medium, left space : %3")
                .arg(m_oVolumeID.isEmpty() ? _("Empty medium") : _("KylinBurner"))
                .arg(m_oMediumType).arg(m_oFreeCapacity);
    }

    Q_EMIT g_user_signal->changeDiscStatus();
    Q_EMIT g_user_signal->sigChangeDiscDisplayInfo((void*)m_oCDROM, m_oDisplay);
}

void ExtendMediumType::diskInfo()
{
    m_new_cdrom = qobject_cast<ExtendDataCDRom*>(m_oCDROM);
    if (!m_bAppRunning || !m_oCDROM || !m_new_cdrom) {
        LOGGER->warn(QString("( ExtendMediumType::diskInfo ) Line:%1 medium is invalid, dev=%2").arg(__LINE__).arg(m_oBlockName));
        return;
    }

    if (m_oCDROM->open()) {
        UByteArray data;
        if( !m_new_cdrom->testUnitReady() ) {
            // no disk or tray open
            m_diskState = Device::STATE_NO_MEDIA;
            m_mediaType = Device::MEDIA_NONE;
            m_currentProfile = Device::MEDIA_NONE;
        }
        else
            m_currentProfile = m_new_cdrom->currentProfile();

        if (diskState() != Device::STATE_NO_MEDIA ) {
            if (m_new_cdrom->readDiscInformation(data)) {
                disc_info_t* dInf = (disc_info_t*)data.data();

                switch (dInf->status) {
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

                switch (dInf->border) {
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

                switch(dInf->bg_f_status&0x3) {
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

                m_numTracks = (dInf->last_track_l & 0xff) | (dInf->last_track_m<<8 & 0xff00);
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
                    m_msfCapacity = Msf(dInf->lead_out_m + dInf->lead_out_r*60,
                                        dInf->lead_out_s,
                                        dInf->lead_out_f) - 150;

                //
                // This is the position where the next Session shall be recorded in HMSF format
                // This is only valid for CD-R(W) and DVD+R media.
                // For complete media this shall be filled with 0xff
                //
                if (dInf->lead_in_m != 0xff &&
                        dInf->lead_in_r != 0xff &&
                        dInf->lead_in_s != 0xff &&
                        dInf->lead_in_f != 0xff)
                    m_msfUsedCapacity = Msf(dInf->lead_in_m + dInf->lead_in_r*60,
                                            dInf->lead_in_s,
                                            dInf->lead_in_f) - 4500;
            }
            else {
                qDebug() << "(Device::Device) " << blockDeviceName()
                         << " fabricating disk information for a stupid device." << endl;
                Toc toc = m_new_cdrom->readToc();
                if (!toc.isEmpty()) {
                    m_diskState = Device::STATE_COMPLETE;
                    m_lastSessionState = Device::STATE_COMPLETE;
                    m_numTracks = toc.count();
                    m_msfCapacity = m_msfUsedCapacity = toc.length();
                }
            }

            //
            // The mediatype needs to be set
            //
            m_mediaType = mediaType();

            // At least some Plextor drives return profile NONE for CD media
            // or CD_ROM for writable media
            if (m_mediaType & (Device::MEDIA_UNKNOWN| Device::MEDIA_NONE| Device::MEDIA_CD_ROM)) {
                // probably it is a CD
                if (rewritable())
                    m_mediaType = Device::MEDIA_CD_RW;
                else if (empty() || appendable())
                    m_mediaType = Device::MEDIA_CD_R;
                else
                    m_mediaType = Device::MEDIA_CD_ROM;
            }

            if (m_mediaType & Device::MEDIA_DVD_ALL) {
                if (m_new_cdrom->readDvdStructure(data)) {
                    // some debugging stuff
                    Msf sda, eda, ea0;
                    sda = (data[4+5]<<16 | data[4+6] << 8 | data[4+7]);
                    eda = (data[4+9]<<16 | data[4+10] << 8 | data[4+11]);
                    ea0 = (data[4+13]<<16 | data[4+14] << 8 | data[4+15]);

                    qDebug() << "First sec data area: " << sda.toString()
                             << " (LBA " << QString::number(sda.lba())
                             << ") (" << QString::number(sda.mode1Bytes()) << endl;
                    qDebug() << "Last sec data area: " << eda.toString()
                             << " (LBA " << QString::number(eda.lba())
                             << ") (" << QString::number(eda.mode1Bytes()) << " Bytes)" << endl;
                    qDebug() << "Last sec layer 1: " << ea0.toString()
                             << " (LBA " << QString::number(ea0.lba())
                             << ") (" << QString::number(ea0.mode1Bytes()) << " Bytes)" << endl;


                    Msf da0 = ea0 - sda + 1;
                    Msf da1 = eda - ea0;
                    qDebug() << "Layer 1 length: " << da0.toString()
                             << " (LBA " << QString::number(da0.lba())
                             << ") (" << QString::number(da0.mode1Bytes()) << " Bytes)" << endl;
                    qDebug() << "Layer 2 length: " << da1.toString()
                             << " (LBA " << QString::number(da1.lba())
                             << ") (" << QString::number(da1.mode1Bytes()) << " Bytes)" << endl;

                    m_numLayers = ((data[6]&0x60) == 0 ? 1 : 2);

                    bool otp = (data[4+2] & 0xF);

                    // ea0 is 0 if the medium does not use Opposite track path
                    if (otp && ea0 > 0)
                        m_firstLayerSize = da0;
                    else
                        m_firstLayerSize = 0;
                }
                else {
                    qDebug() << "(Device::Device) Unable to read DVD structure for num of layers.";
                    m_numLayers = ((m_mediaType & Device::MEDIA_WRITABLE_DVD_DL) ? 2 : 1);
                }
            }


            //
            // Number of sessions for non-empty disks
            //
            if (diskState() != Device::STATE_EMPTY) {
                int sessions = static_cast<ExtendDataCDRom*>(m_oCDROM)->numSessions();
                if (sessions >= 0)
                    m_numSessions = sessions;
                else
                    qDebug() << "(Device::Device) could not get session info via READ TOC/PMA/ATIP.";
            }
            else
                m_numSessions = 0;
            //
            int media = mediaType();
            //
            // Use the profile if available because DVD-ROM units need to treat DVD+-R(W) media as DVD-ROM
            // if supported at all
            //
            if (m_new_cdrom->currentProfile() == Device::MEDIA_DVD_ROM)
                media = Device::MEDIA_DVD_ROM;

            switch(media) {
            case Device::MEDIA_CD_R:
            case Device::MEDIA_CD_RW:
                if (m_msfCapacity == 0) {
                    if (m_new_cdrom->readTocPmaAtip(data, 0x4, true, 0)) {
                        struct atip_descriptor* atip = (struct atip_descriptor*)data.data();
                        if (data.size() >= 11) {
                            m_msfCapacity = Msf(atip->lead_out_m, atip->lead_out_s, atip->lead_out_f) - 150;
                            Device::debugBitfield(&atip->lead_out_m, 3);
                            qDebug() << blockDeviceName() << ": ATIP capacity: " << m_msfCapacity.toString();
                        }
                    }
                }

                //
                // for empty and appendable media capacity and usedCapacity should be filled in from
                // diskinfo above. If not they are both still 0
                //
                if (m_msfCapacity != 0 && (diskState() == Device::STATE_EMPTY || m_msfUsedCapacity != 0)) {
                    break;
                }

            default:
            case Device::MEDIA_CD_ROM:
                if (m_msfCapacity > 0 && m_msfUsedCapacity == 0)
                    m_msfUsedCapacity = m_msfCapacity;

                if (m_msfUsedCapacity == 0) {
                    Msf readCap;
                    if (m_new_cdrom->readCapacity(readCap)) {
                        qDebug() << "(Device::Device) READ CAPACITY: " << readCap.toString()
                                 << " other capacity: " << m_msfCapacity.toString() << endl;
                        //
                        // READ CAPACITY returns the last written sector
                        // that means the size is actually readCap + 1
                        //
                        m_msfUsedCapacity = readCap + 1;
                    }
                    else {
                        qDebug() << "(Device::Device) " << blockDeviceName()
                                 << " Falling back to readToc for capacity." << endl;
                        m_msfUsedCapacity = m_new_cdrom->readToc().length();
                    }
                }

            case Device::MEDIA_DVD_ROM: {
                Msf readCap;
                if (m_new_cdrom->readCapacity(readCap)) {
                    qDebug() << "(Device::Device) READ CAPACITY: " << readCap.toString()
                             << " other capacity: " << m_msfCapacity.toString() << endl;
                    //
                    // READ CAPACITY returns the last written sector
                    // that means the size is actually readCap + 1
                    //
                    m_msfUsedCapacity = readCap + 1;
                }
                else {
                    //
                    // Only one track, use it's size
                    //
                    if (m_new_cdrom->readTrackInformation(data, 0x1, 0x1)) {
                        track_info_t* trackInfo = (track_info_t*)data.data();
                        m_msfUsedCapacity = Device::from4Byte(trackInfo->track_size);
                    }
                    else
                        qDebug() << "(Device::Device) " << blockDeviceName()
                                 << "READ TRACK INFORMATION for DVD-ROM failed." << endl;
                }

                break;
            }

            case Device::MEDIA_DVD_PLUS_R:
            case Device::MEDIA_DVD_PLUS_R_DL:
                if (appendable() || empty()) {
                    //
                    // get remaining space via the invisible track
                    //
                    if (m_new_cdrom->readTrackInformation(data, 0x1, /*0xff*/ numTracks()+1)) {
                        track_info_t* trackInfo = (track_info_t*)data.data();
                        m_msfUsedCapacity = Device::from4Byte(trackInfo->track_start);
                        m_msfCapacity = Device::from4Byte(trackInfo->track_start) + Device::from4Byte(trackInfo->track_size);
                    }
                }
                else {
                    if (m_new_cdrom->readTrackInformation(data, 0x1, numTracks())) {
                        track_info_t* trackInfo = (track_info_t*)data.data();
                        m_msfCapacity = m_msfUsedCapacity
                                = Device::from4Byte(trackInfo->track_start) + Device::from4Byte(trackInfo->track_size);
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
                if (m_new_cdrom->readTrackInformation(data, 0x1, numTracks()+1)) {
                    track_info_t* trackInfo = (track_info_t*)data.data();
                    m_msfUsedCapacity = Device::from4Byte(trackInfo->track_start);
                    m_msfCapacity = Device::from4Byte(trackInfo->free_blocks) + Device::from4Byte(trackInfo->track_start);
                }

                //
                // Get the "really" used space without border-out
                //
                if (!empty()) {
                    Msf readCap;
                    if (m_new_cdrom->readCapacity(readCap)) {
                        //
                        // READ CAPACITY returns the last written sector
                        // that means the size is actually readCap + 1
                        //
                        m_msfUsedCapacity = readCap + 1;
                    }
                    else
                        qDebug() << "(Device::Device) " << blockDeviceName()
                                 << " READ CAPACITY for DVD-R failed." << endl;
                }

                break;

            case Device::MEDIA_DVD_RW_OVWR:
                m_numSessions = 1;
            case Device::MEDIA_DVD_RW:
            case Device::MEDIA_DVD_RW_SEQ:
                // only one track on a DVD-RW media
                if (m_new_cdrom->readTrackInformation(data, 0x1, 0x1)) {
                    track_info_t* trackInfo = (track_info_t*)data.data();
                    m_msfCapacity = Device::from4Byte(trackInfo->track_size);
                    if (!empty()) {
                        if (m_new_cdrom->readFormatCapacity(0x10, m_msfCapacity))
                            qDebug() << blockDeviceName() << ": Format capacity 0x10: " << m_msfCapacity.toString();

                        m_msfUsedCapacity = Device::from4Byte(trackInfo->track_size);
                    }
                }
                break;

            case Device::MEDIA_DVD_PLUS_RW: {
                Msf currentMax;
                int currentMaxFormat = 0;
                if (m_new_cdrom->readFormatCapacity(0x26, m_msfCapacity, &currentMax, &currentMaxFormat)) {
                    if (currentMaxFormat == 0x1) { // unformatted or blank media
                        m_msfUsedCapacity = 0;
                        m_msfCapacity = currentMax;
                    }
                    else {
                        m_msfUsedCapacity = currentMax;
                        // Plextor drives tend to screw things up and report invalid values
                        // for the max format capacity of 1.4 GB DVD media
                        if (bgFormatState() == Device::BG_FORMAT_COMPLETE) {
                            m_msfCapacity = currentMax;
                        }
                    }
                }
                else
                    qDebug() << "(Device::Device) " << blockDeviceName()
                             << " READ FORMAT CAPACITIES for DVD+RW failed." << endl;

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
                if (m_new_cdrom->readDiscInformation(data)) {
                    int lastTrack = (int)(data[11]<<8 | data[6]);

                    if (m_new_cdrom->readTrackInformation(data, 1, lastTrack)) {

                        // capacity: last track's start address + last track's size
                        m_msfCapacity = Device::from4Byte(&data[8]) + Device::from4Byte(&data[24]);
                        if (m_msfCapacity.mode1Bytes() > 26000000000)
                            m_oMediumType = "BD-R DL";
                        if (data[6] & 0x80)
                            m_msfUsedCapacity = Device::from4Byte(&data[8]);
                        else if (data[7] & 0x1)
                            m_msfUsedCapacity = Device::from4Byte(&data[12]);
                        else
                            m_msfUsedCapacity = m_msfCapacity;
                    }
                }
                break;

            case Device::MEDIA_BD_RE: {
                Msf currentMax;
                int currentMaxFormat = 0;
                if (m_new_cdrom->readFormatCapacity(0x00, m_msfCapacity, &currentMax, &currentMaxFormat)) {
                    if (currentMaxFormat == 0x1) { // unformatted or blank media
                        m_msfUsedCapacity = 0;
                        m_msfCapacity = currentMax;
                    }
                    else {
                        m_msfUsedCapacity = currentMax;
                    }
                }
                else
                    qDebug() << "(Device::Device) " << blockDeviceName()
                             << " READ FORMAT CAPACITIES for BD-RE failed." << endl;
                break;
            }

            case Device::MEDIA_BD_ROM: {
                Msf readCap;
                if (m_new_cdrom->readCapacity(readCap)) {
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

void ExtendMediumType::capacity()
{
    LOGGER->info(QString("\n( ExtendMediumType::capacity ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    if (!m_bAppRunning || !m_oCDROM || !m_discread) {
        LOGGER->warn(QString("( ExtendMediumType::capacity ) Line:%1 medium is invalid, dev=%2").arg(__LINE__).arg(m_oBlockName));
        return;
    }
    /* 容量计算 */
    m_oCapacity = msfCapacity();
    m_u64Capacity = m_msfCapacity.mode1Bytes();

    if (!empty()) {
        LOGGER->info(QString("(ExtendMediumType::capacity)  Line:%1 DVD+RW?%2 总容量为：%3(%4), m_bUDF=%5").arg(__LINE__)
                     .arg((mediaType() & Device::MEDIA_DVD_PLUS_RW) > 0).arg(m_u64Capacity).arg(m_oCapacity).arg(m_bUDF==true));
        if ( (mediaType() & Device::MEDIA_DVD_PLUS_RW) && (0 == m_u64Capacity) && m_bUDF) {
            // gio 获取
            GFile *file;
            GFileInfo *info;
            QString targetUri = "file://" + m_oMountPoint;
            file = g_file_new_for_uri(targetUri.toUtf8().constData());
            info = g_file_query_filesystem_info(file, "*", nullptr, nullptr);
            if (!info) {
                LOGGER->warn(QString("DVD+RW光盘 UDF 使用gio获取总容量失败：%1").arg(targetUri));
                if (file) {
                    g_object_unref(file);
                }
                return;
            }

            m_u64Capacity = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
            m_msfCapacity = m_u64Capacity;
            m_oCapacity = toUnitString(m_u64Capacity);
            g_object_unref(info);
            g_object_unref(file);
            LOGGER->info(QString("DVD+RW光盘 UDF 使用gio获取总容量：%1(%2)").arg(m_u64Capacity).arg(m_oCapacity));
        }

        if (m_bUDF) {
            if (mediaType() & ( Device::MEDIA_DVD_PLUS_RW|Device::MEDIA_DVD_RW_OVWR|Device::MEDIA_BD_RE| Device::MEDIA_CD_RW )) {
                /* UDF 且 DVD+/-RW、CD-RW */
                GFile *file;
                GFileInfo *info;
                QString targetUri = "file://" + m_oMountPoint;

                file = g_file_new_for_uri(targetUri.toUtf8().constData());
                info = g_file_query_filesystem_info(file, "*", nullptr, nullptr);
                if (!info) {
                    LOGGER->warn(QString("UDF格式使用gio获取容量失败：%1").arg(targetUri));
                    if (file) {
                        g_object_unref(file);
                    }
                    return;
                }

                m_u64UsedCapacity =  g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_USED);
                m_oUsedCapacity = toUnitString(m_u64UsedCapacity);

                m_u64FreeCapacity = (m_u64UsedCapacity > m_u64Capacity) ? 0 : (m_u64Capacity - m_u64UsedCapacity);
                m_oFreeCapacity = toUnitString(m_u64FreeCapacity);
                g_object_unref(info);
                g_object_unref(file);

                LOGGER->info(QString("UDF2.01格式(gio): 使用：%1(%2) 剩余：%3(%4)")
                             .arg(m_u64UsedCapacity).arg(m_oUsedCapacity)
                             .arg(m_u64FreeCapacity).arg(m_oFreeCapacity));
            } else {
                /* UDF1.02 且 CD-R、DVD+/-R(DL)盘 */
                m_oUsedCapacity = msfUsedCapacity();
                m_oFreeCapacity = msfFreeCapacity();
                m_u64UsedCapacity = m_msfUsedCapacity.mode1Bytes();
                m_u64FreeCapacity = (m_u64UsedCapacity > m_u64Capacity) ? 0 : (m_u64Capacity - m_u64UsedCapacity);
                LOGGER->info(QString("UDF1.02格式(Msf): 使用：%1(%2) 剩余：%3(%4)")
                             .arg(m_u64UsedCapacity).arg(m_oUsedCapacity)
                             .arg(m_u64FreeCapacity).arg(m_oFreeCapacity));
            }
        } else {
            /* ISO 9660 */
            if (mediaType() & ( Device::MEDIA_DVD_PLUS_RW|Device::MEDIA_DVD_RW_OVWR|Device::MEDIA_BD_RE)) {
                /* ISO9660 且 DVD+/-RW、CD-RW */
                Iso9660SimplePrimaryDescriptor isoDesc;
                isoDesc = Iso9660SimplePrimaryDescriptor();
                Iso9660 iso(new Iso9660DeviceBackend(m_oCDROM));
                iso.setStartSector(0);
                iso.setPlainIso9660(true);
                if (iso.open()) {
                    isoDesc = iso.primaryDescriptor();
                    m_msfUsedCapacity = isoDesc.getVolumSpacesSize();
                } else {
                    // 没有获取到已使用数据容量，可能是Win刻录的UDF盘或空光盘
                    m_msfUsedCapacity = 0;
                    LOGGER->info(QString("[%1][%2] Failed to obtain the capacity. It may be a real UDF disk").arg(__func__).arg(__LINE__));
                }
            }
            m_oUsedCapacity = msfUsedCapacity();
            m_oFreeCapacity = msfFreeCapacity();
            m_u64UsedCapacity = m_msfUsedCapacity.mode1Bytes();
            m_u64FreeCapacity = (m_u64UsedCapacity > m_u64Capacity) ? 0 : (m_u64Capacity - m_u64UsedCapacity);
            LOGGER->info(QString("ISO9660格式(Msf): 使用：%1(%2) 剩余：%3(%4)")
                         .arg(m_u64UsedCapacity).arg(m_oUsedCapacity)
                         .arg(m_u64FreeCapacity).arg(m_oFreeCapacity));
        }
    } else {
        // 空盘
        m_u64UsedCapacity = 0;
        m_msfUsedCapacity = 0;
        m_u64FreeCapacity = (m_u64UsedCapacity > m_u64Capacity) ? 0 : (m_u64Capacity - m_u64UsedCapacity);
        m_oFreeCapacity = msfFreeCapacity();
        LOGGER->info(QString("空盘(Msf): 使用：%1(%2) 剩余：%3(%4)")
                     .arg(m_u64UsedCapacity).arg(m_oUsedCapacity)
                     .arg(m_u64FreeCapacity).arg(m_oFreeCapacity));
    }

    if ((0 == m_u64Capacity) || (0 == m_msfCapacity)) {
        m_u64Capacity = m_u64UsedCapacity;
        m_msfCapacity = m_msfUsedCapacity;
        m_oCapacity = m_oUsedCapacity;
        m_u64FreeCapacity = (m_u64UsedCapacity > m_u64Capacity) ? 0 : (m_u64Capacity - m_u64UsedCapacity);
        m_oFreeCapacity = toUnitString(m_u64FreeCapacity);
        LOGGER->info(QString("光盘总容量为0， 总容量 = 已使用容量(%1)").arg(m_u64UsedCapacity));
        LOGGER->info(QString("修正：使用：%1(%2) 剩余：%3(%4)")
                     .arg(m_u64UsedCapacity).arg(m_oUsedCapacity)
                     .arg(m_u64FreeCapacity).arg(m_oFreeCapacity));
    }

    LOGGER->info(QString("Capacity: %1 - used(%2) - free(%3)").arg(m_oCapacity).arg(m_oUsedCapacity).arg(m_oFreeCapacity));
    LOGGER->info(QString("( ExtendMediumType::capacity ) Line:%1 退出函数 dev=%2\n").arg(__LINE__).arg(m_oBlockName));
}

void ExtendMediumType::analyseContent()
{
    // set basic content types
    switch( toc.contentType() ) {
    case Device::AUDIO:
        m_oContent = ContentAudio;
        break;
    case Device::DATA:
        m_oContent = ContentData;
        break;
    case Device::MIXED:
        m_oContent = static_cast<MediumContent>(MediumContent::ContentAudio | MediumContent::ContentData);
        break;
    default:
        m_oContent = ContentNone;
    }

    // analyze filesystem
    if( m_oContent & ContentData ) {
        unsigned long startSec = 0;

        qDebug() << "m_numSessions = " << m_numSessions;
        qDebug() << "m_numTracks = " << m_numTracks;
        if( m_numSessions > 1 && !toc.isEmpty() ) {
            // We use the last data track
            // this way we get the latest session on a ms cd
            for( int i = toc.size()-1; i >= 0; --i ) {
                if( toc.at(i).type() == Track::TYPE_DATA ) {
                    startSec = toc.at(i).firstSector().lba();
                    break;
                }
            }
        }
        else if( !toc.isEmpty() ) {
            // use first data track
            for( int i = 0; i < toc.size(); ++i ) {
                if( toc.at(i).type() == Track::TYPE_DATA ) {
                    startSec = toc.at(i).firstSector().lba();
                    break;
                }
            }
        }
        else {
            qDebug() << "(Medium) ContentData is set and Toc is empty, disk is probably broken!";
        }

        // force the backend since we don't need decryption
        // which just slows down the whole process
        Iso9660 iso( new Iso9660DeviceBackend( m_oCDROM ) );
        iso.setStartSector( startSec );
        iso.setPlainIso9660( true );
        if( iso.open() ) {
            m_isoDesc = iso.primaryDescriptor();
            if (!m_isoDesc.getVolumeID().isEmpty()) {
                m_oVolumeID = m_isoDesc.volumeId;
            } else {
                LOGGER->info("( ExtendMediumType::analyseContent ) Using Iso9660SimplePrimaryDescriptor get volume failed");
            }
            qDebug() << "(ExtendMediumType::analyseContent) found volume id from start sector " << startSec
                     << ": '" << m_isoDesc.volumeId << "'" ;
        }
    }
}

QString ExtendMediumType::msfUsedCapacity()
{
    LOGGER->info(QString("msfUsedCapacity: 已使用容量：%1  %2 %3")
                 .arg(getMsfUsedCapacity().toString())
                 .arg(getMsfUsedCapacity().mode1Bytes())
                 .arg(toUnitString(getMsfUsedCapacity().mode1Bytes())));
    return toUnitString(getMsfUsedCapacity().mode1Bytes());

}

QString ExtendMediumType::msfFreeCapacity()
{
    LOGGER->info(QString("msfFreeCapacity: 剩余容量：%1  %2 %3")
                 .arg(getMsfFreeCapacity().toString())
                 .arg(getMsfFreeCapacity().mode1Bytes())
                 .arg(toUnitString(getMsfFreeCapacity().mode1Bytes())));
    return toUnitString(getMsfFreeCapacity().mode1Bytes());
}


QString ExtendMediumType::msfCapacity()
{
    LOGGER->info(QString("MsfCapacity: 总容量：%1  %2 %3")
                 .arg(getMsfCapacity().toString())
                 .arg(getMsfCapacity().mode1Bytes())
                 .arg(toUnitString(getMsfCapacity().mode1Bytes())));
    return toUnitString(getMsfCapacity().mode1Bytes());
}

ExtendMediumType::ExtendMediumType(QString type, QString blockname, QObject * parent):DataMedium(type,blockname,parent)
{

}

ExtendMediumType::ExtendMediumType(DataMedium &medium, QObject * parent):DataMedium(medium,parent)
{
}

void ExtendMediumType::load()
{
    LOGGER->info(QString("( ExtendMediumType::load ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    if (!m_oCDROM || !m_discread || !m_mountPointTimer) {
        LOGGER->warn(QString("( ExtendMediumType::load ) Line:%1 medium is invalid, dev=%2").arg(__LINE__).arg(m_oBlockName));
        return;
    }

    connect(m_mountPointTimer,&QTimer::destroyed,this,[&]{m_mountPointTimer = nullptr;},Qt::UniqueConnection);
    connect(m_mountPointTimer, &QTimer::timeout, this, &ExtendMediumType::slotsMountPoint,Qt::UniqueConnection);

    if (!m_oCDROM->isOpen()) {
        if (!m_oCDROM->open()) {
            LOGGER->warn(QString("( ExtendMediumType::load ) device = %1 , open device faild").arg(m_oBlockName));
        }
    }

    type();
    speed();
    m_oCDROM->close();
    m_oDisplay = _("%1 - %2 medium, left space : %3")
            .arg(m_oMediumName.isEmpty() ? _("Empty medium") : m_oCDROM->settingVolumnID())
            .arg(m_oMediumType).arg(m_oFreeCapacity);

    connect(m_discread, &DiscRead::workFinished, this, &ExtendMediumType::fixDiscStatus);

    m_discread->work();
    LOGGER->info(QString("( ExtendMediumType::load ) Line:%1 运行完成 dev=%2").arg(__LINE__).arg(m_oBlockName));
}

Msf ExtendMediumType::getMsfCapacity() const
{
    return m_msfCapacity;
}

Msf ExtendMediumType::getMsfUsedCapacity() const
{
    return m_msfUsedCapacity;
}

Msf ExtendMediumType::getMsfFreeCapacity() const
{
    return m_msfCapacity - m_msfUsedCapacity;
}


QString ExtendMediumType::blockDeviceName() const
{
    return m_oBlockName;
}
