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

#ifndef TOC_H
#define TOC_H

#include "track.h"
#include "msf.h"
#include <vector>
#include <cstdint>
#include <string>
#include <iostream>

enum ContentsType
{
    DATA,
    AUDIO,
    MIXED,
    NONE // no tracks
};

class Toc
{
public:
    typedef std::vector<Track>::iterator iterator;
    typedef std::vector<Track>::const_iterator const_iterator;

    Toc();
    Toc(const Toc &toc);
    ~Toc();

    Toc &operator=(const Toc &toc);

    // Container operations
    void append(const Track &track) { m_tracks.push_back(track); }
    void clear();
    bool isEmpty() const { return m_tracks.empty(); }
    size_t count() const { return m_tracks.size(); }
    size_t size() const { return m_tracks.size(); }

    // Iterator support
    iterator begin() { return m_tracks.begin(); }
    iterator end() { return m_tracks.end(); }
    const_iterator constBegin() const { return m_tracks.begin(); }
    const_iterator constEnd() const { return m_tracks.end(); }

    // Element access
    Track &first() { return m_tracks.front(); }
    const Track &first() const { return m_tracks.front(); }
    Track &last() { return m_tracks.back(); }
    const Track &last() const { return m_tracks.back(); }
    Track &operator[](size_t i) { return m_tracks[i]; }
    const Track &operator[](size_t i) const { return m_tracks[i]; }
    const Track &at(size_t i) const { return m_tracks.at(i); }

    // Toc specific methods
    Msf firstSector() const;
    Msf lastSector() const;
    Msf length() const;
    unsigned int discId() const;
    ContentsType contentType() const;
    int sessions() const;

    std::string mcn() const;
    void setMcn(const std::string &mcn);

    bool operator==(const Toc &other) const;
    bool operator!=(const Toc &other) const;

private:
    std::vector<Track> m_tracks;
    std::string m_mcn;
};

std::ostream &operator<<(std::ostream &s, const Toc &toc);

#endif // TOC_H
