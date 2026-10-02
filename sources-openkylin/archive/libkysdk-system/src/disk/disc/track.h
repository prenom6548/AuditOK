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

#ifndef TRACK_H
#define TRACK_H

#include "msf.h"
#include <vector>
#include <string>
#include <iostream>
#include <memory>

class Track
{
public:
    enum TrackType
    {
        TYPE_AUDIO,
        TYPE_DATA,
        TYPE_UNKNOWN = -1,
    };

    enum DataMode
    {
        MODE1,
        MODE2,
        XA_FORM1,
        XA_FORM2,
        DVD,
        UNKNOWN
    };

    class Private;

    Track();
    Track(const Track &track);
    Track(const Msf &firstSector,
          const Msf &lastSector,
          TrackType type = TYPE_UNKNOWN,
          DataMode mode = UNKNOWN);
    ~Track();

    Track &operator=(const Track &track);

    Msf length() const;
    TrackType type() const;
    void setType(TrackType t);
    DataMode mode() const;
    void setMode(DataMode m);
    bool copyPermitted() const;
    void setCopyPermitted(bool b);
    bool preEmphasis() const;
    void setPreEmphasis(bool b);
    bool recordedIncremental() const;
    bool recordedUninterrupted() const;

    std::string isrc() const;
    void setIsrc(const std::string &s);

    Msf firstSector() const;
    Msf lastSector() const;
    void setFirstSector(const Msf &msf);
    void setLastSector(const Msf &msf);

    Msf nextWritableAddress() const;
    void setNextWritableAddress(const Msf &m);
    void setFreeBlocks(const Msf &m);
    Msf freeBlocks() const;

    Msf realAudioLength() const;
    int session() const;
    void setSession(int s);

    Msf index0() const;
    std::vector<Msf> indices() const;
    void setIndices(const std::vector<Msf> &il);
    void setIndex0(const Msf &msf);
    int indexCount() const;

    bool operator==(const Track &other) const;
    bool operator!=(const Track &other) const;

private:
    std::shared_ptr<Private> d;
};

std::ostream &operator<<(std::ostream &s, const Track &track);

#endif // TRACK_H
