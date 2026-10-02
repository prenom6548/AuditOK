#include "toc.h"
#include <QDebug>
#include <QString>

Device::Toc::Toc()
    : QList<Device::Track>()
{
}


Device::Toc::Toc( const Toc& toc )
    : QList<Device::Track>( toc )
{
    m_mcn = toc.m_mcn;
}


Device::Toc::~Toc()
{
}


Device::Toc& Device::Toc::operator=( const Toc& toc )
{
    if( &toc == this ) return *this;

    m_mcn = toc.m_mcn;

    QList<Device::Track>::operator=( toc );

    return *this;
}


Msf Device::Toc::firstSector() const
{
    return isEmpty() ? Msf() : first().firstSector();
}


Msf Device::Toc::lastSector() const
{
    if( isEmpty() )
        return 0;
    // the last track's last sector should be the last sector of the entire cd
    return last().lastSector();
}


Msf Device::Toc::length() const
{
    // +1 since the last sector is included
    return lastSector() - firstSector() + 1;
}


unsigned int Device::Toc::discId() const
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


Device::ContentsType Device::Toc::contentType() const
{
    int audioCnt = 0, dataCnt = 0;
    for( Toc::const_iterator it = constBegin(); it != constEnd(); ++it ) {
        if( (*it).type() == Device::Track::TYPE_AUDIO )
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


int Device::Toc::sessions() const
{
    if( isEmpty() )
        return 0;
    else if( last().session() == 0 )
        return 1; // default if unknown
    else
        return last().session();
}


QByteArray Device::Toc::mcn() const
{
    return m_mcn;
}


void Device::Toc::setMcn( const QByteArray& mcn )
{
    m_mcn = mcn;
}


void Device::Toc::clear()
{
    QList<Track>::clear();
    m_mcn.resize( 0 );
}


bool Device::Toc::operator==( const Toc& other ) const
{
    return( QList<Track>::operator==( other ) );
}


bool Device::Toc::operator!=( const Toc& other ) const
{
    return( QList<Track>::operator!=( other ) );
}


QDebug operator<<( QDebug s, const Device::Toc& toc )
{
    s.nospace() << toc.count() << " in " << toc.sessions() << " sessions";
    int sessionN = 0;
    int trackN = 0;
    for( Device::Toc::const_iterator it = toc.constBegin(); it != toc.constEnd(); ++it ) {
        ++trackN;
        if( sessionN != it->session() ) {
            sessionN = it->session();
            s.nospace() << "Session Number " << sessionN;
        }
        s.nospace() << "  Track " << trackN << *it;
    }
    return s;
}
