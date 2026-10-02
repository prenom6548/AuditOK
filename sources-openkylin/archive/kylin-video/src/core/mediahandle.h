/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MEDIAHANDLE_H
#define MEDIAHANDLE_H

#include <MediaInfo/MediaInfo.h>

#include <QString>
#include <QMap>

namespace KMedia {

enum VideoFormat {
    H261, H263, H264, H265, RMVB, RMHD, VP8, VP9, MPEG2, MPEG4, AVS, AVC, WMV, MOV
};

enum AudioFormat {
    CDA, WAV, MP3, WMA, RA, MIDI, OGG, APE, FLAC, AAC
};

class VideoInfo {
public:
    VideoInfo(){}
    ~VideoInfo(){}

    int ID;
    int Width;
    int Height;
    int FPSnum;
    int FPSden;
    int BitDepth;
    double FPSRate;

    quint64 Duration;
    quint64 Bitrate;
    quint64 StreamSize;

    QString CodecID;
    VideoFormat Format;
    QString CodecFormat;
    QString CodecFormatVersion;
    QString HdrInfo;
    QString TransferCharacteristics;
};

class AudioInfo {
public:
    AudioInfo(){}
    ~AudioInfo(){}

    int ID;
    int Channels;

    quint64 Duration;
    quint64 Bitrate;
    quint64 StreamSize;
    quint64 SamplingRate;

    QString CodecID;
    QString ChannelLayout;
    AudioFormat Format;
};

class MediaHandle
{
    friend class unitConstruct;
public:
    MediaHandle(const QString &filePath = "");
    ~MediaHandle();

    void setFile(const QString &filePath);
    int getVideoCount()     {return m_videoCount;}
    int getAudioCount()     {return m_audioCount;}
    int getSubTitleCount()  {return m_subtitleCount;}

    quint64 getDuration()   {return m_duration;}
    quint64 getFileSize()   {return m_fileSize;}
    quint64 getBitRate()    {return m_overallBitRate;}

    quint64 getVBitRate(int id);
    quint64 getABitRate(int id);

    QMap<int, VideoInfo*>& getVideoMap() {return m_videoMap;}
    QMap<int, AudioInfo*>& getAudioMap() {return m_audioMap;}
    qint64 getFps() { return m_fps;}

    bool getCanGetPreviewPicVal() { return m_canGetPreviewPic; }
    bool getIsMpeg2() { return m_isMpeg2; }
    bool getIsHdr() { return m_isHdr; }

private:
    MediaInfoLib::MediaInfo* m_mediaInfo;
    QString m_filePath;
    quint64 m_fileSize;         // kb
    quint64 m_duration;         // ms
    quint64 m_overallBitRate;   // kb/s

    int m_videoCount;
    int m_audioCount;
    int m_subtitleCount;

    int m_fps;

    QMap<int, VideoInfo*> m_videoMap;
    QMap<int, AudioInfo*> m_audioMap;

    bool m_canGetPreviewPic { true };
    bool m_isMpeg2 { false };
    bool m_isHdr { false };

    void setAll();
    void unsetAll();
};
}

#endif // MEDIAHANDLE_H
