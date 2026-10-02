#ifndef TRACK_H
#define TRACK_H

#include "msf.h"
#include <QSharedDataPointer>

namespace Device
{
    class Track
    {
    public:
        enum TrackType {
            TYPE_AUDIO,
            TYPE_DATA,
            TYPE_UNKNOWN = -1
        };

        enum DataMode {
            MODE1,
            MODE2,
            XA_FORM1,
            XA_FORM2,
            DVD,
            UNKNOWN
        };

        Track();
        Track( const Track& );
        Track( const Msf& firstSector,
               const Msf& lastSector,
               TrackType type,
               DataMode mode = UNKNOWN );
        ~Track();

        Track& operator=( const Track& );

        TrackType type() const;

        void setType( TrackType );

        /**
         * UNKNOWN for DVDs and Audio CDs
         */
        DataMode mode() const;

        void setMode( DataMode );

        /**
         * Invalid for DVDs
         */
        bool copyPermitted() const;
        void setCopyPermitted( bool b );

        /**
         * Only valid for audio tracks
         */
        bool preEmphasis() const;
        void setPreEmphasis( bool b );

        bool recordedIncremental() const;
        bool recordedUninterrupted() const;

        QByteArray isrc() const;
        void setIsrc( const QByteArray& s );

        Msf firstSector() const;
        Msf lastSector() const;
        void setFirstSector( const Msf& msf );
        void setLastSector( const Msf& msf );

        Msf nextWritableAddress() const;
        void setNextWritableAddress( const Msf& );

        Msf freeBlocks() const;
        void setFreeBlocks( const Msf& );

        Msf length() const;

        /**
         * This takes index0 into account
         */
        Msf realAudioLength() const;

        /**
         * 0 if unknown
         */
        int session() const;
        void setSession( int s );

        /**
         * @return number of indices. This does not include index 0.
         */
        int indexCount() const;

        /**
         * Returns the index relative to the track's start.
         * If it is zero there is no index0.
         */
        Msf index0() const;

        /**
         * Set the track's index0 value.
         * @param msf offset relative to track start.
         */
        void setIndex0( const Msf& msf );

        /**
         * All indices. Normally this list is empty as indices are rarely used.
         * Starts with index 2 (since index 1 are all other sectors FIXME)
         */
        QList<Msf> indices() const;

        void setIndices( const QList<Msf>& );

        bool operator==( const Track& ) const;
        bool operator!=( const Track& ) const;

    private:
        class Private;
        QSharedDataPointer<Private> d;
    };
}

QDebug operator<<( QDebug s, const Device::Track& track );

/**
* Dummy implementation to allow compilation on Windows
*/
uint qHash( const Device::Track& key );
#endif // TRACK_H
