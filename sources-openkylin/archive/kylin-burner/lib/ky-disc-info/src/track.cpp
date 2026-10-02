#include "track.h"

#include <QSharedData>

class Device::Track::Private : public QSharedData
{
public:
    Private( const Msf& fs = Msf(),
             const Msf& ls = Msf(),
             TrackType t = TYPE_UNKNOWN,
             DataMode m = UNKNOWN )
        : firstSector( fs ),
          lastSector( ls ),
          type( t ),
          mode( m ),
          copyPermitted(true),
          preEmphasis(false),
          session(0) {
    }

    Msf firstSector;
    Msf lastSector;
    Msf index0;

    Msf nextWritableAddress;
    Msf freeBlocks;

    TrackType type;
    DataMode mode;
    bool copyPermitted;
    bool preEmphasis;

    int session;

    QList<Msf> indices;

    QByteArray isrc;
};


Device::Track::Track()
    : d( new Private() )
{
}


Device::Track::Track( const Track& track )
{
    d = track.d;
}


Device::Track::Track( const Msf& firstSector,
                           const Msf& lastSector,
                           TrackType type,
                           DataMode mode )
    : d( new Private( firstSector,
                      lastSector,
                      type,
                      mode ) )
{
}


Device::Track::~Track()
{
}


Device::Track& Device::Track::operator=( const Track& track )
{
    d = track.d;
    return *this;
}


Msf Device::Track::length() const
{
    // +1 since the last sector is included
    return d->lastSector - d->firstSector + 1;
}


Device::Track::TrackType Device::Track::type() const
{
    return d->type;
}


void Device::Track::setType( TrackType t )
{
    d->type = t;
}


Device::Track::DataMode Device::Track::mode() const
{
    return d->mode;
}


void Device::Track::setMode( DataMode m )
{
    d->mode = m;
}


bool Device::Track::copyPermitted() const
{
    return d->copyPermitted;
}


void Device::Track::setCopyPermitted( bool b )
{
    d->copyPermitted = b;
}


bool Device::Track::preEmphasis() const
{
    return d->preEmphasis;
}


void Device::Track::setPreEmphasis( bool b )
{
    d->preEmphasis = b;
}


bool Device::Track::recordedIncremental() const
{
    return d->preEmphasis;
}


bool Device::Track::recordedUninterrupted() const
{
    return !recordedIncremental();
}


QByteArray Device::Track::isrc() const
{
    return d->isrc;
}


void Device::Track::setIsrc( const QByteArray& s )
{
    d->isrc = s;
}


Msf Device::Track::firstSector() const
{
    return d->firstSector;
}


Msf Device::Track::lastSector() const
{
    return d->lastSector;
}


void Device::Track::setFirstSector( const Msf& msf )
{
    d->firstSector = msf;
}


void Device::Track::setLastSector( const Msf& msf )
{
    d->lastSector = msf;
}


Msf Device::Track::nextWritableAddress() const
{
    return d->nextWritableAddress;
}


void Device::Track::setNextWritableAddress( const Msf& m )
{
    d->nextWritableAddress = m;
}


void Device::Track::setFreeBlocks( const Msf& m )
{
    d->freeBlocks = m;
}


Msf Device::Track::freeBlocks() const
{
    return d->freeBlocks;
}


Msf Device::Track::realAudioLength() const
{
    if( index0() > 0 )
        return index0();
    else
        return length();
}


int Device::Track::session() const
{
    return d->session;
}


void Device::Track::setSession( int s )
{
    d->session = s;
}


Msf Device::Track::index0() const
{
    return d->index0;
}


QList<Msf> Device::Track::indices() const
{
    return d->indices;
}


void Device::Track::setIndices( const QList<Msf>& il )
{
    d->indices = il;
}


void Device::Track::setIndex0( const Msf& msf )
{
    if( msf <= d->lastSector-d->firstSector )
        d->index0 = msf;
}


int Device::Track::indexCount() const
{
    return d->indices.count()-1;
}


bool Device::Track::operator==( const Track& other ) const
{
    return( d->firstSector == other.d->firstSector &&
            d->lastSector == other.d->lastSector &&
            d->index0 == other.d->index0 &&
            d->nextWritableAddress == other.d->nextWritableAddress &&
            d->freeBlocks == other.d->freeBlocks &&
            d->type == other.d->type &&
            d->mode == other.d->mode &&
            d->copyPermitted == other.d->copyPermitted &&
            d->preEmphasis == other.d->preEmphasis &&
            d->session == other.d->session &&
            d->indices == other.d->indices &&
            d->isrc == other.d->isrc );
}


bool Device::Track::operator!=( const Track& other ) const
{
    return !operator==( other );
}


QDebug operator<<( QDebug s, const Device::Track& track )
{
    s.nospace() << ( track.type() == Device::Track::TYPE_AUDIO ? " AUDIO" : " DATA" )
                << " " << track.firstSector().lba() << " - " << track.lastSector().lba()
                << " (" << track.length().lba() << ")";
    return s;
}


uint qHash( const Device::Track& key )
{
    // this is a dummy implementation to make it compile on windows
    return qHash((long)&key);
}
