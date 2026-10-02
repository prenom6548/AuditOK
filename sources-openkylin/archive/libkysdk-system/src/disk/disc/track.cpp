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

#include "track.h"
#include <iostream>

class Track::Private
{
public:
    Private(const Msf &fs = Msf(),
            const Msf &ls = Msf(),
            Track::TrackType t = TYPE_UNKNOWN,
            Track::DataMode m = UNKNOWN)
        : firstSector(fs),
          lastSector(ls),
          type(t),
          mode(m),
          copyPermitted(true),
          preEmphasis(false),
          session(0)
    {
    }

    // 引用计数
    long refCount = 1;

    Msf firstSector;
    Msf lastSector;
    Msf index0;

    Msf nextWritableAddress;
    Msf freeBlocks;

    Track::TrackType type;
    Track::DataMode mode;
    bool copyPermitted;
    bool preEmphasis;

    int session;

    std::vector<Msf> indices;

    std::string isrc;
};

Track::Track()
    : d(new Private())
{
}

Track::Track(const Track &track)
{
    d = track.d;
    d->refCount++;
}

Track::Track(const Msf &firstSector,
             const Msf &lastSector,
             TrackType type,
             DataMode mode)
    : d(new Private(firstSector, lastSector, type, mode))
{
}

Track::~Track()
{
    if (--d->refCount == 0)
    {
        d.reset();
    }
}

Track &Track::operator=(const Track &track)
{
    if (this != &track)
    {
        if (--d->refCount == 0)
        {
            d.reset();
        }
        d = track.d;
        d->refCount++;
    }
    return *this;
}

Msf Track::length() const
{
    // +1 since the last sector is included
    return d->lastSector - d->firstSector + 1;
}

Track::TrackType Track::type() const
{
    return d->type;
}

void Track::setType(TrackType t)
{
    d->type = t;
}

Track::DataMode Track::mode() const
{
    return d->mode;
}

void Track::setMode(DataMode m)
{
    d->mode = m;
}

bool Track::copyPermitted() const
{
    return d->copyPermitted;
}

void Track::setCopyPermitted(bool b)
{
    d->copyPermitted = b;
}

bool Track::preEmphasis() const
{
    return d->preEmphasis;
}

void Track::setPreEmphasis(bool b)
{
    d->preEmphasis = b;
}

bool Track::recordedIncremental() const
{
    return d->preEmphasis;
}

bool Track::recordedUninterrupted() const
{
    return !recordedIncremental();
}

std::string Track::isrc() const
{
    return d->isrc;
}

void Track::setIsrc(const std::string &s)
{
    d->isrc = s;
}

Msf Track::firstSector() const
{
    return d->firstSector;
}

Msf Track::lastSector() const
{
    return d->lastSector;
}

void Track::setFirstSector(const Msf &msf)
{
    d->firstSector = msf;
}

void Track::setLastSector(const Msf &msf)
{
    d->lastSector = msf;
}

Msf Track::nextWritableAddress() const
{
    return d->nextWritableAddress;
}

void Track::setNextWritableAddress(const Msf &m)
{
    d->nextWritableAddress = m;
}

void Track::setFreeBlocks(const Msf &m)
{
    d->freeBlocks = m;
}

Msf Track::freeBlocks() const
{
    return d->freeBlocks;
}

Msf Track::realAudioLength() const
{
    if (index0() > 0)
        return index0();
    else
        return length();
}

int Track::session() const
{
    return d->session;
}

void Track::setSession(int s)
{
    d->session = s;
}

Msf Track::index0() const
{
    return d->index0;
}

std::vector<Msf> Track::indices() const
{
    return d->indices;
}

void Track::setIndices(const std::vector<Msf> &il)
{
    d->indices = il;
}

void Track::setIndex0(const Msf &msf)
{
    if (msf <= d->lastSector - d->firstSector)
        d->index0 = msf;
}

int Track::indexCount() const
{
    return static_cast<int>(d->indices.size()) - 1;
}

bool Track::operator==(const Track &other) const
{
    return (d->firstSector == other.d->firstSector &&
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
            d->isrc == other.d->isrc);
}

bool Track::operator!=(const Track &other) const
{
    return !operator==(other);
}

std::ostream &operator<<(std::ostream &s, const Track &track)
{
    s << (track.type() == Track::TYPE_AUDIO ? " AUDIO" : " DATA")
      << " " << track.firstSector().lba() << " - " << track.lastSector().lba()
      << " (" << track.length().lba() << ")";
    return s;
}
