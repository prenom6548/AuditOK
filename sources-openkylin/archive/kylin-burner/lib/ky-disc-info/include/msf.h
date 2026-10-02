#ifndef MSF_H
#define MSF_H


#include <QDebug>
#include <QSharedDataPointer>


#include <QString>
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

    unsigned long long mode1Bytes() const;
    unsigned long long mode2Form1Bytes() const;
    unsigned long long mode2Form2Bytes() const;
    unsigned long long audioBytes() const;
    unsigned long long rawBytes() const;
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
Msf operator+(Msf const& m, int i );
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
