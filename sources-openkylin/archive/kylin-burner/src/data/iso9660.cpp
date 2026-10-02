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

#include "iso9660.h"
#include "iso9660backend.h"
#include "datacdrom.h"

#include "src/libisofs/isofs.h"
#include "src/common/encodingdetector.h"

#include <QDebug>
#include <QDir>
#include <QFile>

/* callback function for libisofs */
int Iso9660::read_callback( char* buf, sector_t start, int len, void* udata )
{
    Iso9660* isoF = static_cast<Iso9660*>(udata);

    return isoF->read( start, buf, len );
}

class Iso9660::Private
{
public:
    Private()
        : cdDevice(0),
          fd(-1),
          isOpen(false),
          startSector(0),
          plainIso9660(false),
          backend(0) {
    }

    Iso9660SimplePrimaryDescriptor primaryDesc;
    DataCDROM *cdDevice;
    int fd;
    bool isOpen;

    // only used for direkt Device::Device access
    unsigned int startSector;
    bool plainIso9660;
    Iso9660Backend* backend;
};

Iso9660::Iso9660( Iso9660Backend* backend )
{
    d = new Private();
    d->backend = backend;
}

Iso9660::~Iso9660()
{
    close();
    delete d->backend;
    delete d;
}

void Iso9660::setStartSector( unsigned int startSector )
{
    d->startSector = startSector;
}


void Iso9660::setPlainIso9660( bool b )
{
    d->plainIso9660 = b;
}

bool Iso9660::plainIso9660() const
{
    return d->plainIso9660;
}

int Iso9660::read( unsigned int sector, char* data, int count )
{
    if( count == 0 )
        return 0;
    else
        return d->backend->read( sector, data, count );
}

bool Iso9660::open()
{ 
#ifndef UNIT_TEST
    iso_vol_desc *desc;
    QString path,tmp,uid,gid;
    k3b_struct_stat buf;
    int access,c_i;

    // TODO implement win32 support

    /* We'll use the permission and user/group of the 'host' file except
     * in Rock Ridge, where the permissions are stored on the file system
     */
    if ( k3b_stat( QFile::encodeName(m_filename), &buf ) < 0 ) {
        /* defaults, if stat fails */
        memset(&buf,0,sizeof(k3b_struct_stat));
        buf.st_mode=0777;
    }
    uid.setNum(buf.st_uid);
    gid.setNum(buf.st_gid);
    access = buf.st_mode & ~S_IFMT;
    c_i=1;

    desc = ReadISO9660( &Iso9660::read_callback, d->startSector, this );
    if (desc) {
        switch (isonum_711(desc->data.type)) {
        case ISO_VD_PRIMARY:
            createSimplePrimaryDesc( (struct iso_primary_descriptor*)&desc->data );
            break;
        default:
            break;
        }
     } else {
        return false;
    }
    FreeISO9660(desc);
#endif
    return true;
}

bool Iso9660::isOpen() const
{
    return d->isOpen;
}

void Iso9660::createSimplePrimaryDesc( struct iso_primary_descriptor* desc )
{
#ifndef UNIT_TEST
    EncodingDetector detector;
    QByteArray validData = detector.toValidByteArray(desc->volume_id, 32);
    EncodingType encodType = detector.detect(validData);
    QString volumeID = detector.decode(validData);

    d->primaryDesc.volumeId = volumeID.trimmed();
    d->primaryDesc.logicalBlockSize = isonum_723(desc->logical_block_size);
    d->primaryDesc.volumeSpaceSize = isonum_733(desc->volume_space_size);
#endif
}


void Iso9660::close()
{
    if( d->isOpen ) {
        d->backend->close();
        d->isOpen = false;
    }
}

const Iso9660SimplePrimaryDescriptor& Iso9660::primaryDescriptor() const
{
    return d->primaryDesc;
}


void Iso9660::debug() const
{
#ifndef UNIT_TEST
    if( isOpen() ) {
        qDebug() << "System Id:         " << primaryDescriptor().systemId;
        qDebug() << "Volume Id:         " << primaryDescriptor().volumeId;
        qDebug() << "Volume Set Id:     " << primaryDescriptor().volumeSetId;
        qDebug() << "Preparer Id:       " << primaryDescriptor().preparerId;
        qDebug() << "Publisher Id:      " << primaryDescriptor().publisherId;
        qDebug() << "Application Id:    " << primaryDescriptor().applicationId;
        qDebug() << "Volume Set Size:   " << primaryDescriptor().volumeSetSize;
        qDebug() << "Volume Set Number: " << primaryDescriptor().volumeSetNumber;
    }
#endif
}

Iso9660SimplePrimaryDescriptor::Iso9660SimplePrimaryDescriptor()
    : volumeSetSize(0),
      volumeSetNumber(0),
      logicalBlockSize(0),
      volumeSpaceSize(0)
{
}
#ifndef UNIT_TEST
bool operator==( const Iso9660SimplePrimaryDescriptor& d1,
                      const Iso9660SimplePrimaryDescriptor& d2 )
{
    return( d1.volumeId == d2.volumeId &&
            d1.systemId == d2.systemId &&
            d1.volumeSetId == d2.volumeSetId &&
            d1.publisherId == d2.publisherId &&
            d1.preparerId == d2.preparerId &&
            d1.applicationId == d2.applicationId &&
            d1.volumeSetSize == d2.volumeSetSize &&
            d1.volumeSetNumber == d2.volumeSetNumber &&
            d1.logicalBlockSize == d2.logicalBlockSize &&
            d1.volumeSpaceSize == d2.volumeSpaceSize );
}

bool operator!=( const Iso9660SimplePrimaryDescriptor& d1,
                      const Iso9660SimplePrimaryDescriptor& d2 )
{
    return( d1.volumeId != d2.volumeId ||
            d1.systemId != d2.systemId ||
            d1.volumeSetId != d2.volumeSetId ||
            d1.publisherId != d2.publisherId ||
            d1.preparerId != d2.preparerId ||
            d1.applicationId != d2.applicationId ||
            d1.volumeSetSize != d2.volumeSetSize ||
            d1.volumeSetNumber != d2.volumeSetNumber ||
            d1.logicalBlockSize != d2.logicalBlockSize ||
            d1.volumeSpaceSize != d2.volumeSpaceSize );
}
#endif