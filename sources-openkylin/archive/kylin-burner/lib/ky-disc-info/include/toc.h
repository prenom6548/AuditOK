#ifndef TOC_H
#define TOC_H

#include "msf.h"
#include "track.h"

namespace Device
{
    enum ContentsType {
        DATA,
        AUDIO,
        MIXED,
        NONE // no tracks
    };

    /**
     * A list of Tracks that represents the contents
     * of a cd.
     * The Toc deletes all its tracks when it is deleted and
     * deletes removed tracks.
     */
    class Toc : public QList<Track>
    {
    public:
        Toc();
        /** deep copy */
        Toc( const Toc& );
        /** deletes all tracks */
        ~Toc();
        /** deep copy */
        Toc& operator=( const Toc& );

        /**
         * CDDB disc Id
         */
        unsigned int discId() const;

        QByteArray mcn() const;

        /**
         * determine the contents type based on the tracks' types.
         * Audio, Data, or Mixed
         */
        ContentsType contentType() const;

        /**
         * \return the number of sessions in this TOC.
         */
        int sessions() const;

        /**
         * The first track's first sector could differ from the disc's
         * first sector if there is a pregap before index 1
         */
        Msf firstSector() const;
        Msf lastSector() const;
        Msf length() const;

        void setMcn( const QByteArray& mcn );

        void clear();

        bool operator==( const Toc& ) const;
        bool operator!=( const Toc& ) const;

    private:
        QByteArray m_mcn;
    };
}

QDebug operator<<( QDebug s, const Device::Toc& );


#endif // TOC_H
