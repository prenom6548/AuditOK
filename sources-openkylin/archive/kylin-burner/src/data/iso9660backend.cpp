/*
 *
 * Copyright (C) 2005-2009 Sebastian Trueg <trueg@k3b.org>
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

#include "iso9660backend.h"
#include "src/status-fix/discscsi.h"


#include "datacdrom.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "src/data/datacdrom.h"
#include <QFile>
#include <QDebug>



Iso9660DeviceBackend::Iso9660DeviceBackend( DataCDROM *dev )
    : m_device( dev ),
      m_isOpen(false)
{
}


Iso9660DeviceBackend::~Iso9660DeviceBackend()
{
    close();
}


bool Iso9660DeviceBackend::open()
{
    if( m_isOpen )
        return true;
    else if( m_device->open() ) {
        // set optimal reading speed
        //m_device->setSpeed( 0xffff, 0xffff );
        m_isOpen = true;
        return true;
    }
    else
        return false;
}


void Iso9660DeviceBackend::close()
{
    if( m_isOpen ) {
        m_isOpen = false;
        m_device->close();
    }
}


int Iso9660DeviceBackend::read( unsigned int sector, char* data, int len )
{
    if( m_device->open() )
    {
        //
        // split the number of sectors to be read
        // FIXME: use a "real" value, not some arbitrary one
        //
        static const int maxReadSectors = 20;
        int sectorsRead = 0;
        int retries = 10;  // TODO: no fixed value
        while( retries ) {
            int read = qMin(len-sectorsRead, maxReadSectors);

            if( !m_device->read10( (unsigned char*)(data+sectorsRead*2048),
                                   read*2048,
                                   sector+sectorsRead,
                                   read))  {
                retries--;
            }
            else {
                sectorsRead += read;
                retries = 10; // new retires for every read part
                if( sectorsRead == len )
                    return len;
            }
        }
    }
    return -1;
}
