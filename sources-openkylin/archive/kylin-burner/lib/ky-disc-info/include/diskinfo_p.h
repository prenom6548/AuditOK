#ifndef DISKINFO_P_H
#define DISKINFO_P_H

#include "msf.h"
#include "deviceglobals.h"

#include <QByteArray>
#include <QSharedData>

namespace Device {
    class DiskInfoPrivate : public QSharedData
    {
    public:
        DiskInfoPrivate()
            : mediaType(MEDIA_UNKNOWN),
              currentProfile(MEDIA_UNKNOWN),
              diskState(STATE_UNKNOWN),
              lastSessionState(STATE_UNKNOWN),
              bgFormatState(BG_FORMAT_INVALID),
              numSessions(0),
              numTracks(0),
              rewritable(false) {
        }

        MediaType mediaType;
        int currentProfile;

        MediaState diskState;
        MediaState lastSessionState;
        BackGroundFormattingState bgFormatState;
        int numSessions;
        int numTracks;
        int numLayers;  // only for DVD media
        int rewritable;

        Msf capacity;
        Msf usedCapacity;
        Msf firstLayerSize;

        QByteArray mediaId;
    };
}

#endif // DISKINFO_P_H
