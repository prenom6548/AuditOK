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



#include "toc.h"

#include <QDebug>
#include <QString>

Toc::Toc()
    : QList<Track>()
{
}


Toc::Toc( const Toc& toc )
    : QList<Track>( toc )
{
    m_mcn = toc.m_mcn;
}


Toc::~Toc()
{
}


Toc& Toc::operator=( const Toc& toc )
{
    if( &toc == this ) return *this;

    m_mcn = toc.m_mcn;

    QList<Track>::operator=( toc );

    return *this;
}


Msf Toc::firstSector() const
{
    return isEmpty() ? Msf() : first().firstSector();
}


Msf Toc::lastSector() const
{
    if( isEmpty() )
        return 0;
    // the last track's last sector should be the last sector of the entire cd
    return last().lastSector();
}


Msf Toc::length() const
{
    // +1 since the last sector is included
    return lastSector() - firstSector() + 1;
}


unsigned int Toc::discId() const
{
    // calculate cddb-id
    unsigned int id = 0;
    for( Toc::const_iterator it = constBegin(); it != constEnd(); ++it ) {
        unsigned int n = (*it).firstSector().lba() + 150;
        n /= 75;
        while( n > 0 ) {
            id += n % 10;
            n /= 10;
        }
    }
    unsigned int l = length().lba();
    if ( !empty() )
        l -= first().firstSector().lba();
    l /= 75;
    id = ( ( id % 0xff ) << 24 ) | ( l << 8 ) | count();

    return id;
}


Device::ContentsType Toc::contentType() const
{
    int audioCnt = 0, dataCnt = 0;
    for( Toc::const_iterator it = constBegin(); it != constEnd(); ++it ) {
        if( (*it).type() == Track::TYPE_AUDIO )
            audioCnt++;
        else
            dataCnt++;
    }

    if( audioCnt + dataCnt == 0 )
        return Device::NONE;
    if( audioCnt == 0 )
        return Device::DATA;
    if( dataCnt == 0 )
        return Device::AUDIO;
    return Device::MIXED;
}


int Toc::sessions() const
{
    if( isEmpty() )
        return 0;
    else if( last().session() == 0 )
        return 1; // default if unknown
    else
        return last().session();
}


QByteArray Toc::mcn() const
{
    return m_mcn;
}


void Toc::setMcn( const QByteArray& mcn )
{
    m_mcn = mcn;
}


void Toc::clear()
{
    QList<Track>::clear();
    m_mcn.resize( 0 );
}


bool Toc::operator==( const Toc& other ) const
{
    return( QList<Track>::operator==( other ) );
}


bool Toc::operator!=( const Toc& other ) const
{
    return( QList<Track>::operator!=( other ) );
}


QDebug operator<<( QDebug s, const Toc& toc )
{
    s.nospace() << toc.count() << " in " << toc.sessions() << " sessions";
    int sessionN = 0;
    int trackN = 0;
    for( Toc::const_iterator it = toc.constBegin(); it != toc.constEnd(); ++it ) {
        ++trackN;
        if( sessionN != it->session() ) {
            sessionN = it->session();
            s.nospace() << "Session Number " << sessionN;
        }
        s.nospace() << "  Track " << trackN << *it;
    }
    return s;
}

