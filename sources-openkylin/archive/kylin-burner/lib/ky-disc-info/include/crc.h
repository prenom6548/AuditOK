#ifndef CRC_H
#define CRC_H

#include <qglobal.h>
namespace Device
{
    //  static Crc* x25();

    // bool check( unsigned char* message, unsigned int len, unsigned char* crc, unsigned int crcLen );

    quint16 calcX25( unsigned char* message, unsigned int len, quint16 start = 0x0000 );

    /**
     * subdata is 12 bytes in long.
     */
    bool checkQCrc( unsigned char* subdata );
}

#endif // CRC_H
