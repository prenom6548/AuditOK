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

#include "msf.h"
#include <cmath>
#include <sstream>
#include <regex>
#include <iostream>
#include <iomanip>
#include <cstdlib>

class Msf::Private
{
public:
    Private(int m = 0, int s = 0, int f = 0)
        : minutes(m),
          seconds(s),
          frames(f)
    {
        makeValid();
    }

    int minutes;
    int seconds;
    int frames;
    int refCount;

    void setValue(int m = 0, int s = 0, int f = 0);
    void makeValid();
};

void Msf::Private::makeValid()
{
    if (frames < 0)
    {
        int newFrames = frames / -75 + 1;
        seconds -= newFrames;
        frames += 75 * newFrames;
    }
    seconds += frames / 75;
    frames = frames % 75;
    if (seconds < 0)
    {
        int newSecs = seconds / -60 + 1;
        minutes -= newSecs;
        seconds += 60 * newSecs;
    }
    minutes += seconds / 60;
    seconds = seconds % 60;
    if (minutes < 0)
    {
        minutes = 0;
        seconds = 0;
        frames = 0;
    }
}

void Msf::Private::setValue(int m, int s, int f)
{
    minutes = m;
    seconds = s;
    frames = f;
    makeValid();
}

Msf::Msf()
    : d(new Private())
{
    d->refCount = 1;
}

Msf::Msf(const Msf &m)
{
    d = m.d;
    d->refCount++;
}

Msf::Msf(int m, int s, int f)
    : d(new Private(m, s, f))
{
    d->refCount = 1;
}

Msf::Msf(int i)
    : d(new Private(0, 0, i))
{
    d->refCount = 1;
}

Msf::~Msf()
{
    if (--d->refCount == 0)
    {
        delete d;
    }
}

int Msf::minutes() const
{
    return d->minutes;
}

int Msf::seconds() const
{
    return d->seconds;
}

int Msf::frames() const
{
    return d->frames;
}

int Msf::totalFrames() const
{
    return (d->minutes * 60 + d->seconds) * 75 + d->frames;
}

int Msf::lba() const
{
    return totalFrames();
}

void Msf::setValue(int m, int s, int f)
{
    if (d->refCount > 1)
    {
        d->refCount--;
        d = new Private(m, s, f);
        d->refCount = 1;
    }
    else
    {
        d->setValue(m, s, f);
    }
}

void Msf::addMinutes(int m)
{
    setValue(d->minutes + m, d->seconds, d->frames);
}

void Msf::addSeconds(int s)
{
    setValue(d->minutes, d->seconds + s, d->frames);
}

void Msf::addFrames(int f)
{
    setValue(d->minutes, d->seconds, d->frames + f);
}

Msf &Msf::operator=(const Msf &m)
{
    if (this != &m)
    {
        if (--d->refCount == 0)
        {
            delete d;
        }
        d = m.d;
        d->refCount++;
    }
    return *this;
}

Msf &Msf::operator=(int i)
{
    if (d->refCount > 1)
    {
        d->refCount--;
        d = new Private(0, 0, i);
        d->refCount = 1;
    }
    else
    {
        d->setValue(0, 0, i);
    }
    return *this;
}

Msf &Msf::operator+=(const Msf &m)
{
    setValue(d->minutes + m.minutes(),
             d->seconds + m.seconds(),
             d->frames + m.frames());
    return *this;
}

Msf &Msf::operator+=(int i)
{
    addFrames(i);
    return *this;
}

Msf &Msf::operator-=(const Msf &m)
{
    setValue(d->minutes - m.minutes(),
             d->seconds - m.seconds(),
             d->frames - m.frames());
    return *this;
}

Msf &Msf::operator-=(int i)
{
    addFrames(-i);
    return *this;
}

const Msf Msf::operator++(int)
{
    Msf old = *this;
    ++(*this);
    return old;
}

Msf &Msf::operator++()
{
    (*this) += 1;
    return *this;
}

const Msf Msf::operator--(int)
{
    Msf old = *this;
    --(*this);
    return old;
}

Msf &Msf::operator--()
{
    (*this) -= 1;
    return *this;
}

std::string Msf::toString(bool showFrames) const
{
    std::ostringstream str;

    if (showFrames)
        str << std::setfill('0') << std::setw(2) << d->minutes << ":"
            << std::setfill('0') << std::setw(2) << d->seconds << ":"
            << std::setfill('0') << std::setw(2) << d->frames;
    else
        str << std::setfill('0') << std::setw(2) << d->minutes << ":"
            << std::setfill('0') << std::setw(2) << d->seconds;

    return str.str();
}

unsigned long long Msf::mode1Bytes() const
{
    return (unsigned long long)2048 * ((unsigned long long)lba());
}

unsigned long long Msf::mode2Form1Bytes() const
{
    return (unsigned long long)2048 * ((unsigned long long)lba());
}

unsigned long long Msf::mode2Form2Bytes() const
{
    return (unsigned long long)2324 * ((unsigned long long)lba());
}

unsigned long long Msf::audioBytes() const
{
    return (unsigned long long)2352 * ((unsigned long long)lba());
}

unsigned long long Msf::rawBytes() const
{
    return (unsigned long long)2448 * ((unsigned long long)lba());
}

unsigned long long Msf::pcmSamples() const
{
    return (unsigned long long)lba() * 588;
}

Msf Msf::fromSeconds(double ms)
{
    return Msf(static_cast<int>(std::ceil(ms * 75.0)));
}

Msf Msf::fromAudioBytes(long long bytes)
{
    if (bytes % 2352 != 0)
    {
        std::cerr << "bytes: " << bytes << " (not aligned to " << 2352 << ")!" << std::endl;
    }
    return Msf(static_cast<int>(bytes / 2352));
}

Msf Msf::fromString(const std::string &s, bool *ok)
{
    // 使用 std::regex 替代 QRegExp
    static std::regex rx("(\\d+)(?::([0-5]?\\d)(?:[:\\.]((?:[0-6]?\\d)|(?:7[0-4])))?)?");
    std::smatch match;

    Msf msf;

    if (std::regex_match(s, match, rx))
    {
        if (match[2].str().empty())
        {
            msf.setValue(0, 0, std::stoi(match[1].str()));
        }
        else
        {
            int m = std::stoi(match[1].str());
            int s = std::stoi(match[2].str());
            int f = std::stoi(match[3].str());
            msf.setValue(m, s, f);
        }

        if (ok)
        {
            *ok = true;
        }
    }
    else if (ok)
    {
        *ok = false;
    }

    return msf;
}

Msf operator+(const Msf &m1, const Msf &m2)
{
    Msf msf(m1);
    return msf += m2;
}

Msf operator+(const Msf &m, int i)
{
    Msf msf(m);
    return msf += i;
}

Msf operator-(const Msf &m1, const Msf &m2)
{
    Msf msf(m1);
    return msf -= m2;
}

Msf operator-(const Msf &m, int i)
{
    Msf msf(m);
    return msf -= i;
}

bool operator==(const Msf &m1, const Msf &m2)
{
    return (m1.minutes() == m2.minutes() &&
            m1.seconds() == m2.seconds() &&
            m1.frames() == m2.frames());
}

bool operator!=(const Msf &m1, const Msf &m2)
{
    return !(m1 == m2);
}

bool operator<(const Msf &m1, const Msf &m2)
{
    return (m1.lba() < m2.lba());
}

bool operator>(const Msf &m1, const Msf &m2)
{
    return (m1.lba() > m2.lba());
}

bool operator<=(const Msf &m1, const Msf &m2)
{
    return (m1.lba() <= m2.lba());
}

bool operator>=(const Msf &m1, const Msf &m2)
{
    return (m1.lba() >= m2.lba());
}

std::ostream &operator<<(std::ostream &s, const Msf &m)
{
    return s << m.toString();
}
