#include "diskinfo.h"
#include "diskinfo_p.h"
#include "msf.h"

#include <QDebug>
#include <QStringList>


Device::DiskInfo::DiskInfo()
    : d( new DiskInfoPrivate() )
{
}


Device::DiskInfo::DiskInfo( const DiskInfo& other )
{
    d = other.d;
}


Device::DiskInfo::~DiskInfo()
{
}


Device::DiskInfo& Device::DiskInfo::operator=( const DiskInfo& other )
{
    d = other.d;
    return *this;
}


Device::MediaState Device::DiskInfo::diskState() const
{
    return d->diskState;
}


Device::MediaState Device::DiskInfo::lastSessionState() const
{
    return d->lastSessionState;
}


Device::BackGroundFormattingState Device::DiskInfo::bgFormatState() const
{
    return d->bgFormatState;
}


bool Device::DiskInfo::empty() const
{
    return diskState() == STATE_EMPTY;
}


bool Device::DiskInfo::rewritable() const
{
    return d->rewritable;
}


bool Device::DiskInfo::appendable() const
{
    return diskState() == STATE_INCOMPLETE;
}


Device::MediaType Device::DiskInfo::mediaType() const
{
    return d->mediaType;
}


int Device::DiskInfo::currentProfile() const
{
    return d->currentProfile;
}


QByteArray Device::DiskInfo::mediaId() const
{
    return d->mediaId;
}


int Device::DiskInfo::numSessions() const
{
    if( empty() )
        return 0;
    else
        return d->numSessions;
}


int Device::DiskInfo::numTracks() const
{
    if( empty() )
        return 0;
    else
        return d->numTracks;
}


int Device::DiskInfo::numLayers() const
{
    if( isDvdMedia( mediaType() ) )
        return d->numLayers;
    else
        return 1;
}


Msf Device::DiskInfo::remainingSize() const
{
    if( empty() )
        return capacity();

    //
    // There is no way to properly determine the used size on an overwrite media
    // without having a look at the filesystem (or is there?)
    //
    else if( appendable() ||
             mediaType() & (MEDIA_DVD_PLUS_RW|MEDIA_DVD_RW_OVWR) )
        return capacity() - d->usedCapacity;

    else
        return 0;
}


Msf Device::DiskInfo::capacity() const
{
    return (d->capacity == 0 ? size() : d->capacity);
}


Msf Device::DiskInfo::size() const
{
    if( empty() )
        return 0;
    else
        return d->usedCapacity;
}


Msf Device::DiskInfo::firstLayerSize() const
{
    if( numLayers() > 1 )
        return d->firstLayerSize;
    else
        return size();
}


void Device::DiskInfo::debug() const
{
}


bool Device::DiskInfo::operator==( const DiskInfo& other ) const
{
    return( d->mediaType == other.d->mediaType &&
            d->currentProfile == other.d->currentProfile &&
            d->diskState == other.d->diskState &&
            d->lastSessionState == other.d->lastSessionState &&
            d->bgFormatState == other.d->bgFormatState &&
            d->numSessions == other.d->numSessions &&
            d->numTracks == other.d->numTracks &&
            d->numLayers == other.d->numLayers &&
            d->rewritable == other.d->rewritable &&
            d->capacity == other.d->capacity &&
            d->usedCapacity == other.d->usedCapacity &&
            d->firstLayerSize == other.d->firstLayerSize &&
            d->mediaId == other.d->mediaId );
}


bool Device::DiskInfo::operator!=( const DiskInfo& other ) const
{
    return( d->mediaType != other.d->mediaType ||
            d->currentProfile != other.d->currentProfile ||
            d->diskState != other.d->diskState ||
            d->lastSessionState != other.d->lastSessionState ||
            d->bgFormatState != other.d->bgFormatState ||
            d->numSessions != other.d->numSessions ||
            d->numTracks != other.d->numTracks ||
            d->numLayers != other.d->numLayers ||
            d->rewritable != other.d->rewritable ||
            d->capacity != other.d->capacity ||
            d->usedCapacity != other.d->usedCapacity ||
            d->firstLayerSize != other.d->firstLayerSize ||
            d->mediaId != other.d->mediaId );
}



