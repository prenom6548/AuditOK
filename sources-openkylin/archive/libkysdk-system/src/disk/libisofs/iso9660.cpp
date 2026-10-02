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

#include "iso9660.h"
#include "iso9660backend.h"
#include "../cdrom/datacdromimpl.h"
#include "isofs.h"
#include <iostream>
#include <algorithm>
#include <cstring>

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
    DataCDROMImpl *cdDevice;
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
    iso_vol_desc *desc;
    // 移除Qt相关变量
    k3b_struct_stat buf;
    struct el_torito_boot_descriptor* bootdesc;

    // TODO implement win32 support

    /* We'll use the permission and user/group of the 'host' file except
     * in Rock Ridge, where the permissions are stored on the file system
     */
    if ( k3b_stat( m_filename.c_str(), &buf ) < 0 ) {
        /* defaults, if stat fails */
        memset(&buf,0,sizeof(k3b_struct_stat));
        buf.st_mode=0777;
    }
    
    // 移除Qt相关代码

    desc = ReadISO9660( &Iso9660::read_callback, d->startSector, this );
    if (desc) {
        std::cout << "Iso9660::open desc != null" << std::endl;
        switch (isonum_711(desc->data.type)) {
        case ISO_VD_PRIMARY:
            createSimplePrimaryDesc( (struct iso_primary_descriptor*)&desc->data );
            break;
        default:
            break;
        }
     } else {
        std::cout << "Iso9660::open desc = null" << std::endl;
        return false;
    }
    FreeISO9660(desc);

    return true;
}

bool Iso9660::isOpen() const
{
    return d->isOpen;
}

void Iso9660::createSimplePrimaryDesc( struct iso_primary_descriptor* desc )
{
    d->primaryDesc.logicalBlockSize = isonum_723(desc->logical_block_size);
    d->primaryDesc.volumeSpaceSize = isonum_733(desc->volume_space_size);

    std::cout << "d->primaryDesc.logicalBlockSize = " << d->primaryDesc.logicalBlockSize << std::endl;
    std::cout << "d->primaryDesc.volumeSpaceSize = " << d->primaryDesc.volumeSpaceSize << std::endl;
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
    if( isOpen() ) {
        std::cout << "System Id:         " << primaryDescriptor().systemId << std::endl;
        std::cout << "Volume Id:         " << primaryDescriptor().volumeId << std::endl;
        std::cout << "Volume Set Id:     " << primaryDescriptor().volumeSetId << std::endl;
        std::cout << "Preparer Id:       " << primaryDescriptor().preparerId << std::endl;
        std::cout << "Publisher Id:      " << primaryDescriptor().publisherId << std::endl;
        std::cout << "Application Id:    " << primaryDescriptor().applicationId << std::endl;
        std::cout << "Volume Set Size:   " << primaryDescriptor().volumeSetSize << std::endl;
        std::cout << "Volume Set Number: " << primaryDescriptor().volumeSetNumber << std::endl;
    }
}

Iso9660SimplePrimaryDescriptor::Iso9660SimplePrimaryDescriptor()
    : volumeSetSize(0),
      volumeSetNumber(0),
      logicalBlockSize(0),
      volumeSpaceSize(0)
{
}

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