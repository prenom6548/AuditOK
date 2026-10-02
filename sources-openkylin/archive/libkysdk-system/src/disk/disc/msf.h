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

#ifndef MSF_H
#define MSF_H

#include <string>
#include <iostream>

class Msf
{
public:
    class Private;

    Msf();
    Msf(const Msf &m);
    Msf(int m, int s, int f);
    Msf(int i);
    ~Msf();

    int minutes() const;
    int seconds() const;
    int frames() const;
    int totalFrames() const;
    int lba() const;

    void setValue(int m, int s, int f);
    void addMinutes(int m);
    void addSeconds(int s);
    void addFrames(int f);

    Msf &operator=(const Msf &m);
    Msf &operator=(int i);
    Msf &operator+=(const Msf &m);
    Msf &operator+=(int i);
    Msf &operator-=(const Msf &m);
    Msf &operator-=(int i);

    const Msf operator++(int);
    Msf &operator++();
    const Msf operator--(int);
    Msf &operator--();

    std::string toString(bool showFrames = true) const;

    unsigned long long mode1Bytes() const;
    unsigned long long mode2Form1Bytes() const;
    unsigned long long mode2Form2Bytes() const;
    unsigned long long audioBytes() const;
    unsigned long long rawBytes() const;
    unsigned long long pcmSamples() const;

    static Msf fromSeconds(double ms);
    static Msf fromAudioBytes(long long bytes);
    static Msf fromString(const std::string &s, bool *ok = nullptr);

private:
    Private *d;
};

// Free functions
Msf operator+(const Msf &m1, const Msf &m2);
Msf operator+(const Msf &m, int i);
Msf operator-(const Msf &m1, const Msf &m2);
Msf operator-(const Msf &m, int i);
bool operator==(const Msf &m1, const Msf &m2);
bool operator!=(const Msf &m1, const Msf &m2);
bool operator<(const Msf &m1, const Msf &m2);
bool operator>(const Msf &m1, const Msf &m2);
bool operator<=(const Msf &m1, const Msf &m2);
bool operator>=(const Msf &m1, const Msf &m2);
std::ostream &operator<<(std::ostream &s, const Msf &m);

#endif // MSF_H
