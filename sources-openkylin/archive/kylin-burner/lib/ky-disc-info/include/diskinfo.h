#ifndef DISKINFO_H
#define DISKINFO_H

#include "devicetypes.h"

#include "toc.h"
#include "msf.h"

#include <QSharedDataPointer>

namespace Device
{
    class DiskInfoPrivate;

    /**
     * This class is directly accociated to a strcuture from
     * the MMC draft READ_DISK_INFO.
     * It also holds some additional data.
     * This class' data will be retrieved by Device::Device.
     *
     * Before using any values one should check diskState != STATE_UNKNOWN or
     * diskState == STATE_NO_MEDIA.
     * That may mean that no disk is in the drive or an error occurred.
     * Writers should never give the STATE_UNKNOWN state. CD-ROM or DVD-ROM
     * drives on the other hand may have trouble determining the state of the disk.
     */
    class DiskInfo
    {
    public:
        DiskInfo();
        DiskInfo( const DiskInfo& );
        ~DiskInfo();

        DiskInfo& operator=( const DiskInfo& );

        /**
         * Returnes the state of the disk.
         * See enum State.
         */
        MediaState diskState() const;

        /**
         * Returnes the state of the last session.
         * See enum State.
         */
        MediaState lastSessionState() const;

        /**
         * Returnes the state of the background formatting. This does
         * only make sense for DVD+RW (and MRW which is not yet supported)
         */
        BackGroundFormattingState bgFormatState() const;

        /**
         * returnes true if diskState() == STATE_EMPTY
         */
        bool empty() const;

        /**
         * Is this a rewritable media (e.g. a CD-RW, DVD-RW, or DVD+RW)
         */
        bool rewritable() const;

        /**
         * Is this disk appendable
         * returnes true if diskState() == STATE_INCOMPLETE
         */
        bool appendable() const;

        /**
         * The type of the disk:
         * CD-ROM, CD-R, CD-RW, DVD-ROM, DVD-R(W), DVD+R(W)
         */
        MediaType mediaType() const;

        /**
         * This is the current profile of the drive. That means it may differ
         * from drive to drive.
         * -1 means no info.
         * Mainly interesting for the distiction of DVD-R(W) modes:
         * Sequential and Restricted Overwrite.
         */
        int currentProfile() const;

        /**
         * The number of sessions on the disk.
         * This does not include any leadout or the last empty session
         * on a DVD+-R(W)
         */
        int numSessions() const;

        /**
         * The number of finished tracks.
         * This does not include the empty track.
         */
        int numTracks() const;

        /**
         * Number of layers on a DVD media. For CD media this is always 1.
         */
        int numLayers() const;

        /**
         * Does only make sense for appendable disks.
         */
        Msf remainingSize() const;

        /**
         * The capacity of the disk.
         * For complete disks this may be the same as size()
         */
        Msf capacity() const;

        /**
         * Returns the size of the used part.
         * For appendable media this equals capacity() - remainingSize()
         */
        Msf size() const;

        /**
         * Returns the size of Data area in the first layer for DL DVD media.
         * Otherwise size() is returned.
         *
         * This does not specify the layer change sector as the data area on DVD media does
         * not start at sector 0 but at sector 30000h or 31000h depending on the type.
         */
        Msf firstLayerSize() const;

        QByteArray mediaId() const;

        void debug() const;

        bool operator==( const DiskInfo& ) const;
        bool operator!=( const DiskInfo& ) const;

    public:
        QSharedDataPointer<DiskInfoPrivate> d;

        friend class Device;
    };

    //  kdbgstream& operator<<( kdbgstream& s, const DiskInfo& ngInf );
}

#endif // DISKINFO_H

