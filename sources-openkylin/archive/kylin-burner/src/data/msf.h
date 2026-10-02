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


#ifndef MSF_H
#define MSF_H

#include <QSharedDataPointer>

class Msf
{
public:
    Msf();
    Msf( const Msf& );
    Msf( int, int, int );
    Msf( int );
    ~Msf();

    Msf& operator=( const Msf& );
    Msf& operator=( int );
    Msf& operator+=( const Msf& );
    Msf& operator+=( int );
    Msf& operator-=( const Msf& );
    Msf& operator-=( int );
    const Msf operator++( int );
    Msf& operator++();
    const Msf operator--( int );
    Msf& operator--();

    int minutes() const;
    int seconds() const;
    int frames() const;

    int totalFrames() const;
    int lba() const;

    //      operator int () const { return lba(); }

    void setValue( int m, int s, int f );

    void addMinutes( int m );
    void addSeconds( int s );
    void addFrames( int f );

    QString toString( bool showFrames = true ) const;

    qulonglong mode1Bytes() const;
    qulonglong mode2Form1Bytes() const;
    qulonglong mode2Form2Bytes() const;
    qulonglong audioBytes() const;
    qulonglong rawBytes() const;
    unsigned long long pcmSamples() const;

    /**
     * Convert a string representation into an Msf object.
     *
     * Valid strings include:
     * \li 100       - treated as 100 frames
     * \li 100:23    - treated as 100 minutes and 23 seconds
     * \li 100:23:57 - treated as 100 minutes, 23 seconds, and 57 frames
     * \li 100:23.57 - treated as 100 minutes, 23 seconds, and 57 frames
     */
    static Msf fromString( const QString&, bool* ok = 0 );

    /**
     * @param ms seconds
     * frames will be rounded up
     */
    static Msf fromSeconds( double ms );

    static Msf fromAudioBytes( qint64 bytes );

    static QRegExp regExp();

private:
    class Private;
    QSharedDataPointer<Private> d;
};

Msf operator+( const Msf&, const Msf& );
Msf operator+( const Msf&, int );
Msf operator-( const Msf&, const Msf& );
Msf operator-( const Msf&, int );
bool operator==( const Msf&, const Msf& );
bool operator!=( const Msf&, const Msf& );
bool operator<( const Msf&, const Msf& );
bool operator>( const Msf&, const Msf& );
bool operator<=( const Msf&, const Msf& );
bool operator>=( const Msf&, const Msf& );

QDebug& operator<<( QDebug&, const Msf& );

#endif // MSF_H
