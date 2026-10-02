/*
 *
 * Copyright (C) 2003-2008 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2008 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "msf.h"
#include <QDebug>
#include <QRegExp>
#include <QSharedData>

#include <cmath>

class Msf::Private : public QSharedData
{
public:
    Private( int m = 0, int s = 0, int f = 0 )
        : minutes(m),
          seconds(s),
          frames(f)
    {
        makeValid();
    }

    int minutes;
    int seconds;
    int frames;

    void setValue( int m = 0, int s = 0, int f = 0 );
    void makeValid();
};


void Msf::Private::makeValid()
{
    if( frames < 0 ) {
        int newFrames = frames/-75 + 1;
        seconds -= newFrames;
        frames += 75*newFrames;
    }
    seconds += frames/75;
    frames = frames % 75;
    if( seconds < 0 ) {
        int newSecs = seconds/-60 + 1;
        minutes -= newSecs;
        seconds += 60*newSecs;
    }
    minutes += seconds/60;
    seconds = seconds % 60;
    if( minutes < 0 ) {
        minutes = 0;
        seconds = 0;
        frames = 0;
    }
}


void Msf::Private::setValue( int m, int s, int f )
{
    minutes = m;
    seconds = s;
    frames = f;
    makeValid();
}


Msf::Msf()
    : d( new Private() )
{
}


Msf::Msf( const Msf& m )
{
    d = m.d;
}


Msf::Msf( int m, int s, int f )
    : d( new Private( m, s, f ) )
{
}


Msf::Msf( int i )
    : d( new Private( 0, 0, i ) )
{
}


Msf::~Msf()
{
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
    return ( d->minutes*60 + d->seconds )*75 + d->frames;
}


int Msf::lba() const
{
    return totalFrames();
}


void Msf::setValue( int m, int s, int f )
{
    d->setValue( m, s, f );
}


void Msf::addMinutes( int m )
{
    d->setValue( d->minutes + m, d->seconds, d->frames );
}


void Msf::addSeconds( int s )
{
    d->setValue( d->minutes, d->seconds + s, d->frames );
}


void Msf::addFrames( int f )
{
    d->setValue( d->minutes, d->seconds, d->frames + f );
}


Msf& Msf::operator=( const Msf& m )
{
    d = m.d;
    return *this;
}


Msf& Msf::operator=( int i )
{
    d->setValue( 0, 0, i );
    return *this;
}


Msf& Msf::operator+=( const Msf& m )
{
    d->setValue( d->minutes + m.minutes(),
                 d->seconds + m.seconds(),
                 d->frames + m.frames() );
    return *this;
}


Msf& Msf::operator+=( int i )
{
    addFrames(i);
    return *this;
}


Msf& Msf::operator-=( const Msf& m )
{
    d->setValue( d->minutes - m.minutes(),
                 d->seconds - m.seconds(),
                 d->frames - m.frames() );
    return *this;
}

Msf& Msf::operator-=( int i )
{
    addFrames( -i );
    return *this;
}


const Msf Msf::operator++( int )
{
    Msf old = *this;
    ++(*this);
    return old;
}


Msf& Msf::operator++()
{
    (*this) += 1;
    return *this;
}


const Msf Msf::operator--( int )
{
    Msf old = *this;
    --(*this);
    return old;
}


Msf& Msf::operator--()
{
    (*this) -= 1;
    return *this;
}


QString Msf::toString( bool showFrames ) const
{
    QString str;

    if( showFrames )
        str.sprintf( "%.2i:%.2i:%.2i", d->minutes, d->seconds, d->frames );
    else
        str.sprintf( "%.2i:%.2i", d->minutes, d->seconds );

    return str;
}


qulonglong Msf::mode1Bytes() const
{
    return (qulonglong)2048 * ( (qulonglong)lba() );
}


qulonglong Msf::mode2Form1Bytes() const
{
    return (qulonglong)2048 * ( (qulonglong)lba() );
}


qulonglong Msf::mode2Form2Bytes() const
{
    return (qulonglong)2324 * ( (qulonglong)lba() );
}


qulonglong Msf::audioBytes() const
{
    return (qulonglong)2352 * ( (qulonglong)lba() );
}


qulonglong Msf::rawBytes() const
{
    return (qulonglong)2448 * ( (qulonglong)lba() );
}


unsigned long long Msf::pcmSamples() const
{
    return lba()*588;
}


QRegExp Msf::regExp()
{
    //
    // An MSF can have the following formats:
    // 100        (treated as frames)
    // 100:23     (minutes:seconds)
    // 100:23:72  (minutes:seconds:frames)
    // 100:23.72  (minutes:seconds.frames)
    //
    static QRegExp rx( "(\\d+)(?::([0-5]?\\d)(?:[:\\.]((?:[0-6]?\\d)|(?:7[0-4])))?)?" );
    return rx;
}


Msf Msf::fromSeconds( double ms )
{
    return Msf( static_cast<int>( ::ceil(ms*75.0) ) );
}


Msf Msf::fromAudioBytes( qint64 bytes )
{
    if( bytes % 2352 != 0 ) {
        qWarning() << "bytes:" << bytes << "(not aligned to" << 2352 << ")!";
    }
    return Msf( bytes/2352 );
}


Msf Msf::fromString( const QString& s, bool* ok )
{
    QRegExp rx = regExp();

    Msf msf;

    if( rx.exactMatch( s ) ) {
        //
        // first number - cap(1)
        // second number - cap(2)
        // third number - cap(3)
        //
        if( rx.cap(2).isEmpty() ) {
            msf.d->setValue( 0, 0, rx.cap(1).toInt() );
        }
        else {
            msf.d->setValue( rx.cap(1).toInt(), rx.cap(2).toInt(), rx.cap(3).toInt() );
        }

        if( ok ) {
            *ok = true;
        }
    }
    else if( ok ) {
        *ok = false;
    }

    return msf;
}



Msf operator+( const Msf& m1, const Msf& m2 )
{
    Msf msf(m1);
    return msf += m2;
}


Msf operator+( const Msf& m, int i )
{
    Msf msf(m);
    return msf += i;
}


Msf operator-( const Msf& m1, const Msf& m2 )
{
    Msf msf(m1);
    return msf -= m2;
}


Msf operator-( const Msf& m, int i )
{
    Msf msf(m);
    return msf -= i;
}


bool operator==( const Msf& m1, const Msf& m2 )
{
    return ( m1.minutes() == m2.minutes() &&
             m1.seconds() == m2.seconds() &&
             m1.frames() == m2.frames() );
}


bool operator!=( const Msf& m1, const Msf& m2 )
{
    return !operator==( m1, m2 );
}


bool operator<( const Msf& m1, const Msf& m2 )
{
    return ( m1.lba() < m2.lba() );
}


bool operator>( const Msf& m1, const Msf& m2 )
{
    return ( m1.lba() > m2.lba() );
}


bool operator<=( const Msf& m1, const Msf& m2 )
{
    return ( m1.lba() <= m2.lba() );
}


bool operator>=( const Msf& m1, const Msf& m2 )
{
    return ( m1.lba() >= m2.lba() );
}


QDebug& operator<<( QDebug& s, const Msf& m )
{
    return s << m.toString();
}
