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

#include "toc.h"
#include <iostream>
#include <sstream>

Toc::Toc()
{
}

Toc::Toc(const Toc &toc)
    : m_tracks(toc.m_tracks), m_mcn(toc.m_mcn)
{
}

Toc::~Toc()
{
}

Toc &Toc::operator=(const Toc &toc)
{
    if (&toc == this)
        return *this;

    m_mcn = toc.m_mcn;
    m_tracks = toc.m_tracks;

    return *this;
}

Msf Toc::firstSector() const
{
    return isEmpty() ? Msf() : first().firstSector();
}

Msf Toc::lastSector() const
{
    if (isEmpty())
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
    for (Toc::const_iterator it = constBegin(); it != constEnd(); ++it)
    {
        unsigned int n = (*it).firstSector().lba() + 150;
        n /= 75;
        while (n > 0)
        {
            id += n % 10;
            n /= 10;
        }
    }
    unsigned int l = length().lba();
    if (!isEmpty())
        l -= first().firstSector().lba();
    l /= 75;
    id = ((id % 0xff) << 24) | (l << 8) | count();

    return id;
}

ContentsType Toc::contentType() const
{
    int audioCnt = 0, dataCnt = 0;
    for (Toc::const_iterator it = constBegin(); it != constEnd(); ++it)
    {
        if ((*it).type() == Track::TYPE_AUDIO)
            audioCnt++;
        else
            dataCnt++;
    }

    if (audioCnt + dataCnt == 0)
        return NONE;
    if (audioCnt == 0)
        return DATA;
    if (dataCnt == 0)
        return AUDIO;
    return MIXED;
}

int Toc::sessions() const
{
    if (isEmpty())
        return 0;
    else if (last().session() == 0)
        return 1; // default if unknown
    else
        return last().session();
}

std::string Toc::mcn() const
{
    return m_mcn;
}

void Toc::setMcn(const std::string &mcn)
{
    m_mcn = mcn;
}

void Toc::clear()
{
    m_tracks.clear();
    m_mcn.clear();
}

bool Toc::operator==(const Toc &other) const
{
    return m_tracks == other.m_tracks;
}

bool Toc::operator!=(const Toc &other) const
{
    return m_tracks != other.m_tracks;
}

std::ostream &operator<<(std::ostream &s, const Toc &toc)
{
    s << toc.count() << " tracks in " << toc.sessions() << " sessions";
    int sessionN = 0;
    int trackN = 0;
    for (Toc::const_iterator it = toc.constBegin(); it != toc.constEnd(); ++it)
    {
        ++trackN;
        if (sessionN != it->session())
        {
            sessionN = it->session();
            s << "\nSession Number " << sessionN;
        }
        s << "\n  Track " << trackN << " " << *it;
    }
    return s;
}
