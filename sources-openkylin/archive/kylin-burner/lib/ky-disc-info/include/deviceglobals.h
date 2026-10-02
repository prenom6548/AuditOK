#ifndef DEVICEGLOBALS_H
#define DEVICEGLOBALS_H

#include "msf.h"
#include "devicetypes.h"
#include <QString>


namespace Device
{
    class Device;

    QString deviceTypeString( int );
    QString writingModeString( int );
    QString mediaStateString( int );
    /**
     * @param simplyfied if true the formatting state of DVD media is left out.
     */
    QString mediaTypeString( int, bool simplyfied = false );
    void debugBitfield( unsigned char* data, long len );

    quint16 from2Byte( const unsigned char* );
    quint32 from4Byte( const unsigned char* );

    char fromBcd( const char& );
    char toBcd( const char& );
    bool isValidBcd( const char& );

    /**
     * @return the maximum nuber of sectors that can be read from device @p dev starting
     * at sector @p firstSector.
     */
    int determineMaxReadingBufferSize( Device* dev, const Msf& firstSector );

    QDebug& operator<<( QDebug& dbg, MediaType );
    QDebug& operator<<( QDebug& dbg, MediaTypes );

    QDebug& operator<<( QDebug& dbg, MediaState );
    QDebug& operator<<( QDebug& dbg, MediaStates );

    QDebug& operator<<( QDebug& dbg, WritingMode );
    QDebug& operator<<( QDebug& dbg, WritingModes );
}

#endif // DEVICEGLOBALS_H
