#include "deviceglobals.h"
#include "device.h"
#include "diskinfo.h"

#include <QDebug>
#include <QStringList>


QString Device::deviceTypeString( int t )
{
    QStringList s;
    if( t & DEVICE_CD_R )
        s += QString("CD-R");
    if( t & DEVICE_CD_RW )
        s += QString("CD-RW");
    if( t & DEVICE_CD_ROM )
        s += QString("CD-ROM");
    if( t & DEVICE_DVD_ROM )
        s += QString("DVD-ROM");
    if( t & DEVICE_DVD_RAM )
        s += QString("DVD-RAM");
    if( t & DEVICE_DVD_R )
        s += QString("DVD-R");
    if( t & DEVICE_DVD_RW )
        s += QString("DVD-RW");
    if( t & DEVICE_DVD_R_DL )
        s += QString("DVD-R DL");
    if( t & DEVICE_HD_DVD_ROM )
        s += QString("HD DVD-ROM");
    if( t & DEVICE_HD_DVD_R )
        s += QString("HD DVD-R");
    if( t & DEVICE_HD_DVD_RAM )
        s += QString("HD DVD-RAM");
    if( t & DEVICE_BD_ROM )
        s += QString("BD-ROM");
    if( t & DEVICE_BD_R )
        s += QString("BD-R");
    if( t & DEVICE_BD_RE )
        s += QString("BD-RE");
    if( t & DEVICE_DVD_PLUS_R )
        s += QString("DVD+R");
    if( t & DEVICE_DVD_PLUS_RW )
        s += QString("DVD+RW");
    if( t & DEVICE_DVD_PLUS_R_DL )
        s += QString("DVD+R DL");

    if( s.isEmpty() )
        return QString("Error");
    else
        return s.join( ", " );
}


QString Device::writingModeString( int m )
{
    QStringList s;
    if( m & WRITINGMODE_SAO )
        s += QString("SAO");
    if( m & WRITINGMODE_TAO )
        s += QString("TAO");
    if( m & WRITINGMODE_RAW )
        s += QString("RAW");
    if( m & WRITINGMODE_SAO_R96P )
        s += QString("SAO/R96P");
    if( m & WRITINGMODE_SAO_R96R )
        s += QString("SAO/R96R");
    if( m & WRITINGMODE_RAW_R16 )
        s += QString("RAW/R16");
    if( m & WRITINGMODE_RAW_R96P )
        s += QString("RAW/R96P");
    if( m & WRITINGMODE_RAW_R96R )
        s += QString("RAW/R96R");
    if( m & WRITINGMODE_INCR_SEQ )
        s += QString("Incremental Sequential");
    if( m & WRITINGMODE_RES_OVWR )
        s += QString("Restricted Overwrite");
    if( m & WRITINGMODE_LAYER_JUMP )
        s += QString("Layer Jump");

    if( m & WRITINGMODE_RRM )
        s += QString("Random Recording");
    if( m & WRITINGMODE_SRM )
        s += QString("Sequential Recording");
    if( m & WRITINGMODE_SRM_POW )
        s += QString("Sequential Recording + POW");

    if( s.isEmpty() )
        return QString("no writing mode");
    else
        return s.join( ", " );
}


QString Device::mediaTypeString( int m, bool simple )
{
    if( m == MEDIA_UNKNOWN )
        return QString("unknown medium type");

    QStringList s;
    if( m & MEDIA_NONE )
        s += QString("No media");
    if( m & MEDIA_DVD_ROM )
        s += QString("DVD-ROM");
    if( m & MEDIA_DVD_R ||
        (simple && (m & MEDIA_DVD_R_SEQ)) )
        s += QString("DVD-R");
    if( m & MEDIA_DVD_R_SEQ && !simple )
        s += QString("DVD-R Sequential");
    if( m & MEDIA_DVD_R_DL ||
        (simple && (m & (MEDIA_DVD_R_DL_SEQ|MEDIA_DVD_R_DL_JUMP))) )
        s += QString("DVD-R Dual Layer");
    if( m & MEDIA_DVD_R_DL_SEQ && !simple )
        s += QString("DVD-R Dual Layer Sequential");
    if( m & MEDIA_DVD_R_DL_JUMP && !simple )
        s += QString("DVD-R Dual Layer Jump");
    if( m & MEDIA_DVD_RAM )
        s += QString("DVD-RAM");
    if( m & MEDIA_DVD_RW ||
        (simple && (m & (MEDIA_DVD_RW_OVWR|MEDIA_DVD_RW_SEQ))) )
        s += QString("DVD-RW");
    if( m & MEDIA_DVD_RW_OVWR && !simple )
        s += QString("DVD-RW Restricted Overwrite");
    if( m & MEDIA_DVD_RW_SEQ && !simple )
        s += QString("DVD-RW Sequential");
    if( m & MEDIA_DVD_PLUS_RW )
        s += QString("DVD+RW");
    if( m & MEDIA_DVD_PLUS_R )
        s += QString("DVD+R");
    if( m & MEDIA_DVD_PLUS_RW_DL )
        s += QString("DVD+RW Dual Layer");
    if( m & MEDIA_DVD_PLUS_R_DL )
        s += QString("DVD+R Dual Layer");
    if( m & MEDIA_CD_ROM )
        s += QString("CD-ROM");
    if( m & MEDIA_CD_R )
        s += QString("CD-R");
    if( m & MEDIA_CD_RW )
        s += QString("CD-RW");
    if( m & MEDIA_HD_DVD_ROM )
        s += QString("HD DVD-ROM");
    if( m & MEDIA_HD_DVD_R )
        s += QString("HD DVD-R");
    if( m & MEDIA_HD_DVD_RAM )
        s += QString("HD DVD-RAM");
    if( m & MEDIA_BD_ROM )
        s += QString("BD-ROM");
    if( m & MEDIA_BD_R ||
        (simple && (m & (MEDIA_BD_R_SRM|MEDIA_BD_R_RRM))) )
        s += QString("BD-R");
    if( m & MEDIA_BD_R_SRM && !simple )
        s += QString("BD-R Sequential (SRM)");
    if( m & MEDIA_BD_R_SRM_POW && !simple )
        s += QString("BD-R Sequential Pseudo Overwrite (SRM+POW)");
    if( m & MEDIA_BD_R_RRM && !simple )
        s += QString("BD-R Random (RRM)");
    if( m & MEDIA_BD_RE )
        s += QString("BD-RE");

    if( s.isEmpty() )
        return QString("Error");
    else
        return s.join( ", " );
}


QString Device::mediaStateString( int state )
{
    if( state == STATE_UNKNOWN )
        return QString("unknown medium state");

    QStringList s;
    if( state & STATE_NO_MEDIA )
        s += QString("no medium");
    if( state & STATE_COMPLETE )
        s += QString("complete medium");
    if( state & STATE_INCOMPLETE )
        s += QString("incomplete medium");
    if( state & STATE_EMPTY )
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
    if (d == NULL) {
        qWarning() << "Invalid Byte!";
        return 0;
    }
    return ((d[0] << 8 & 0xFF00) |
            (d[1]      & 0xFF));
}


quint32 Device::from4Byte(const unsigned char* d)
{
    if (d == NULL) {
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


int Device::determineMaxReadingBufferSize( Device* dev, const Msf& firstSector )
{
    //
    // As long as we do not know how to determine the max read buffer properly we simply determine it
    // by trying. :)
    //

    int bufferSizeSectors = 128;
    unsigned char buffer[2048*128];
    while( !dev->read10( buffer, 2048*bufferSizeSectors, firstSector.lba(), bufferSizeSectors ) ) {
        qDebug() << "(DataTrackReader) determine max read sectors: "
                 << bufferSizeSectors << " too high." ;
        bufferSizeSectors--;
    }
    qDebug() << "(DataTrackReader) determine max read sectors: "
             << bufferSizeSectors << " is max." ;

    return bufferSizeSectors;
}


QDebug& Device::operator<<( QDebug& dbg, MediaType type )
{
    return dbg << mediaTypeString( type );
}


QDebug& Device::operator<<( QDebug& dbg, MediaTypes types )
{
    return dbg << mediaTypeString( types );
}


QDebug& Device::operator<<( QDebug& dbg, WritingMode mode )
{
    return dbg << writingModeString( mode );
}


QDebug& Device::operator<<( QDebug& dbg, WritingModes modes )
{
    return dbg << writingModeString( modes );
}


QDebug& Device::operator<<( QDebug& dbg, MediaState state )
{
    return dbg << mediaStateString( state );
}


QDebug& Device::operator<<( QDebug& dbg, MediaStates states )
{
    return dbg << mediaStateString( states );
}
