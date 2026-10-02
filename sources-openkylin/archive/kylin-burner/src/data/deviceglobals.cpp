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

#include "deviceglobals.h"

#include <QDebug>
#include <QStringList>


QString Device::deviceTypeString( int t )
{
    QStringList s;
    if( t & Device::DEVICE_CD_R )
        s += QString("CD-R");
    if( t & Device::DEVICE_CD_RW )
        s += QString("CD-RW");
    if( t & Device::DEVICE_CD_ROM )
        s += QString("CD-ROM");
    if( t & Device::DEVICE_DVD_ROM )
        s += QString("DVD-ROM");
    if( t & Device::DEVICE_DVD_RAM )
        s += QString("DVD-RAM");
    if( t & Device::DEVICE_DVD_R )
        s += QString("DVD-R");
    if( t & Device::DEVICE_DVD_RW )
        s += QString("DVD-RW");
    if( t & Device::DEVICE_DVD_R_DL )
        s += QString("DVD-R DL");
    if( t & Device::DEVICE_HD_DVD_ROM )
        s += QString("HD DVD-ROM");
    if( t & Device::DEVICE_HD_DVD_R )
        s += QString("HD DVD-R");
    if( t & Device::DEVICE_HD_DVD_RAM )
        s += QString("HD DVD-RAM");
    if( t & Device::DEVICE_BD_ROM )
        s += QString("BD-ROM");
    if( t & Device::DEVICE_BD_R )
        s += QString("BD-R");
    if( t & Device::DEVICE_BD_RE )
        s += QString("BD-RE");
    if( t & Device::DEVICE_DVD_PLUS_R )
        s += QString("DVD+R");
    if( t & Device::DEVICE_DVD_PLUS_RW )
        s += QString("DVD+RW");
    if( t & Device::DEVICE_DVD_PLUS_R_DL )
        s += QString("DVD+R DL");

    if( s.isEmpty() )
        return QString("Error");
    else
        return s.join( ", " );
}


QString Device::writingModeString( int m )
{
    QStringList s;
    if( m & Device::WRITINGMODE_SAO )
        s += QString("SAO");
    if( m & Device::WRITINGMODE_TAO )
        s += QString("TAO");
    if( m & Device::WRITINGMODE_RAW )
        s += QString("RAW");
    if( m & Device::WRITINGMODE_SAO_R96P )
        s += QString("SAO/R96P");
    if( m & Device::WRITINGMODE_SAO_R96R )
        s += QString("SAO/R96R");
    if( m & Device::WRITINGMODE_RAW_R16 )
        s += QString("RAW/R16");
    if( m & Device::WRITINGMODE_RAW_R96P )
        s += QString("RAW/R96P");
    if( m & Device::WRITINGMODE_RAW_R96R )
        s += QString("RAW/R96R");
    if( m & Device::WRITINGMODE_INCR_SEQ )
        s += QString("Incremental Sequential");
    if( m & Device::WRITINGMODE_RES_OVWR )
        s += QString("Restricted Overwrite");
    if( m & Device::WRITINGMODE_LAYER_JUMP )
        s += QString("Layer Jump");

    if( m & Device::WRITINGMODE_RRM )
        s += QString("Random Recording");
    if( m & Device::WRITINGMODE_SRM )
        s += QString("Sequential Recording");
    if( m & Device::WRITINGMODE_SRM_POW )
        s += QString("Sequential Recording + POW");

    if( s.isEmpty() )
        return QString("no writing mode");
    else
        return s.join( ", " );
}


QString Device::mediaTypeString( int m, bool simple )
{
    if( m == Device::MEDIA_UNKNOWN )
        return QString("unknown medium type");

    QStringList s;
    if( m & Device::MEDIA_NONE )
        s += QString("No media");
    if( m & Device::MEDIA_DVD_ROM )
        s += QString("DVD-ROM");
    if( m & Device::MEDIA_DVD_R ||
        (simple && (m & Device::MEDIA_DVD_R_SEQ)) )
        s += QString("DVD-R");
    if( m & Device::MEDIA_DVD_R_SEQ && !simple )
        s += QString("DVD-R Sequential");
    if( m & Device::MEDIA_DVD_R_DL ||
        (simple && (m & (Device::MEDIA_DVD_R_DL_SEQ|Device::MEDIA_DVD_R_DL_JUMP))) )
        s += QString("DVD-R Dual Layer");
    if( m & Device::MEDIA_DVD_R_DL_SEQ && !simple )
        s += QString("DVD-R Dual Layer Sequential");
    if( m & Device::MEDIA_DVD_R_DL_JUMP && !simple )
        s += QString("DVD-R Dual Layer Jump");
    if( m & Device::MEDIA_DVD_RAM )
        s += QString("DVD-RAM");
    if( m & Device::MEDIA_DVD_RW ||
        (simple && (m & (Device::MEDIA_DVD_RW_OVWR|Device::MEDIA_DVD_RW_SEQ))) )
        s += QString("DVD-RW");
    if( m & Device::MEDIA_DVD_RW_OVWR && !simple )
        s += QString("DVD-RW Restricted Overwrite");
    if( m & Device::MEDIA_DVD_RW_SEQ && !simple )
        s += QString("DVD-RW Sequential");
    if( m & Device::MEDIA_DVD_PLUS_RW )
        s += QString("DVD+RW");
    if( m & Device::MEDIA_DVD_PLUS_R )
        s += QString("DVD+R");
    if( m & Device::MEDIA_DVD_PLUS_RW_DL )
        s += QString("DVD+RW Dual Layer");
    if( m & Device::MEDIA_DVD_PLUS_R_DL )
        s += QString("DVD+R Dual Layer");
    if( m & Device::MEDIA_CD_ROM )
        s += QString("CD-ROM");
    if( m & Device::MEDIA_CD_R )
        s += QString("CD-R");
    if( m & Device::MEDIA_CD_RW )
        s += QString("CD-RW");
    if( m & Device::MEDIA_HD_DVD_ROM )
        s += QString("HD DVD-ROM");
    if( m & Device::MEDIA_HD_DVD_R )
        s += QString("HD DVD-R");
    if( m & Device::MEDIA_HD_DVD_RAM )
        s += QString("HD DVD-RAM");
    if( m & Device::MEDIA_BD_ROM )
        s += QString("BD-ROM");
    if( m & Device::MEDIA_BD_R ||
        (simple && (m & (Device::MEDIA_BD_R_SRM|Device::MEDIA_BD_R_RRM))) )
        s += QString("BD-R");
    if( m & Device::MEDIA_BD_R_SRM && !simple )
        s += QString("BD-R Sequential (SRM)");
    if( m & Device::MEDIA_BD_R_SRM_POW && !simple )
        s += QString("BD-R Sequential Pseudo Overwrite (SRM+POW)");
    if( m & Device::MEDIA_BD_R_RRM && !simple )
        s += QString("BD-R Random (RRM)");
    if( m & Device::MEDIA_BD_RE )
        s += QString("BD-RE");

    if( s.isEmpty() )
        return QString("Error");
    else
        return s.join( ", " );
}


QString Device::mediaStateString( int state )
{
    if( state == Device::STATE_UNKNOWN )
        return QString("unknown medium state");

    QStringList s;
    if( state & Device::STATE_NO_MEDIA )
        s += QString("no medium");
    if( state & Device::STATE_COMPLETE )
        s += QString("complete medium");
    if( state & Device::STATE_INCOMPLETE )
        s += QString("incomplete medium");
    if( state & Device::STATE_EMPTY )
        s += QString("empty medium");

    if( s.isEmpty() )
        return QString("Error");
    else
        return s.join( ", " );
}


void Device::debugBitfield( unsigned char* data, long len )
{
    for( int i = 0; i < len; ++i ) {
        QString index, bitString;
        index.sprintf( "%4i", i );
        for( int bp = 7; bp >= 0; --bp )
            bitString[7-bp] = ( data[i] & (1<<bp) ? '1' : '0' );
        qDebug() << index << " - " << bitString << " - " << (int)data[i];
    }
}


quint16 Device::from2Byte(const unsigned char* d)
{
    if (NULL == d) {
        qWarning() << "Invalid Byte!";
        return 0;
    }
    return ((d[0] << 8 & 0xFF00) |
            (d[1]      & 0xFF));
}


quint32 Device::from4Byte(const unsigned char* d)
{
    if (NULL == d) {
        qWarning() << "Invalid Byte!";
        return 0;
    }
    return ((d[0] << 24 & 0xFF000000) |
            (d[1] << 16 & 0xFF0000)   |
            (d[2] << 8  & 0xFF00)     |
            (d[3]       & 0xFF));
}


char Device::fromBcd( const char& i )
{
    return (i & 0x0f) + 10 * ( (i >> 4) & 0x0f );
}


char Device::toBcd( const char& i )
{
    return ( i % 10 ) | ( ( (( i / 10 ) % 10) << 4 ) & 0xf0 );
}


bool Device::isValidBcd( const char& i )
{
    return ( i & 0x0f ) <= 0x09 && ( i & 0xf0 ) <= 0x90;
}


QDebug& Device::operator<<( QDebug& dbg, Device::MediaType type )
{
    return dbg << Device::mediaTypeString( type );
}


QDebug& Device::operator<<( QDebug& dbg, Device::MediaTypes types )
{
    return dbg << Device::mediaTypeString( types );
}


QDebug& Device::operator<<( QDebug& dbg, Device::WritingMode mode )
{
    return dbg << writingModeString( mode );
}


QDebug& Device::operator<<( QDebug& dbg, Device::WritingModes modes )
{
    return dbg << writingModeString( modes );
}


QDebug& operator<<( QDebug& dbg, Device::MediaState state )
{
    return dbg << Device::mediaStateString( state );
}


QDebug& operator<<( QDebug& dbg, Device::MediaStates states )
{
    return dbg << Device::mediaStateString( states );
}
