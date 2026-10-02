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
#include "mediahandle.h"

#include <ZenLib/Ztring.h>
#include <ukui-log4qt.h>
#include <QDebug>

using namespace MediaInfoLib;
using namespace ZenLib;



// 替换原有的Ztring相关宏，使用Qt原生函数
#define wstring2QString(_DATA) \
    QString::fromWCharArray(_DATA.c_str())
#define QString2wstring(_DATA) \
    std::wstring(_DATA.toStdWString())

namespace KMedia {

MediaHandle::MediaHandle(const QString &filePath)
{
    m_mediaInfo = new MediaInfo;
    if (filePath != "") {
        m_mediaInfo->Open(QString2wstring(filePath));
        m_filePath = filePath;
        setAll();
    }
}

MediaHandle::~MediaHandle()
{
    unsetAll();
}

void MediaHandle::setFile(const QString &filePath)
{
    unsetAll();
    m_mediaInfo->Close();
    m_filePath = filePath;
    m_mediaInfo->Open(QString2wstring(filePath));
    setAll();
}

quint64 MediaHandle::getVBitRate(int id)
{
    foreach (auto info, m_videoMap) {
        if (info->ID == id)
            return info->Bitrate;
    }
    return -1;
}

quint64 MediaHandle::getABitRate(int id)
{
    foreach (auto info, m_audioMap) {
        if (info->ID == id)
            return info->Bitrate;
    }
    return -1;
}

void MediaHandle::setAll()
{
    if (m_mediaInfo) {
        m_fileSize = wstring2QString(m_mediaInfo->Get(Stream_General, 0, __T("FileSize"))).toULong();
        m_duration = wstring2QString(m_mediaInfo->Get(Stream_General, 0, __T("Duration"))).toULong();
        m_overallBitRate = wstring2QString(m_mediaInfo->Get(Stream_General, 0, __T("OverallBitRate"))).toULong();

        m_videoCount = wstring2QString(m_mediaInfo->Get(Stream_General, 0, __T("VideoCount"))).toInt();
        for (int i = 0; i < m_videoCount; i++) {
            VideoInfo *vinfo = new VideoInfo;
            vinfo->ID        = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("ID"))).toInt();
            vinfo->Width     = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("Width"))).toInt();
            vinfo->Height    = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("Height"))).toInt();
            vinfo->FPSnum    = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("FrameRate_Num"))).toInt();
            vinfo->FPSden    = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("FrameRate_Den"))).toInt();
            vinfo->FPSRate   = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("FrameRate"))).toDouble();
            vinfo->BitDepth  = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("BitDepth"))).toInt();
            vinfo->Duration  = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("Duration"))).toULong();
            vinfo->CodecID   = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("CodecID")));
            vinfo->Bitrate   = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("BitRate"))).toULong();
            vinfo->StreamSize= wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("StreamSize"))).toULong();

            vinfo->CodecFormat= wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("Format")));
            vinfo->CodecFormatVersion   = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("Format_Version")));
            vinfo->HdrInfo   = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("HDR_Format")));
            vinfo->TransferCharacteristics = wstring2QString(m_mediaInfo->Get(Stream_Video, i, __T("transfer_characteristics")));
            if (vinfo->CodecFormat == "MPEG Video" && vinfo->CodecFormatVersion == "Version 2") {
                m_isMpeg2 = true;
                qInfo() << "video is mepg2";
            }

            if (vinfo->TransferCharacteristics == "PQ" || vinfo->TransferCharacteristics == "HLG") {
                m_isHdr = true;
                qInfo() << "video is hdr";
            }
            if (m_canGetPreviewPic && vinfo->CodecFormat == "RLE") {
                m_canGetPreviewPic = false;
                qInfo() << "get RLE";
            }
            m_fps = vinfo->FPSRate;
            qInfo() << "vinfo->FPSnum:" << vinfo->FPSnum << ", vinfo->FPSden:" << vinfo->FPSden << ", vinfo->FPSRate:" << vinfo->FPSRate;
            m_duration = m_duration > vinfo->Duration ? m_duration : vinfo->Duration;
            m_videoMap[i] = vinfo;
        }

        m_audioCount = wstring2QString(m_mediaInfo->Get(Stream_General, 0, __T("AudioCount"))).toInt();
        for (int i = 0; i < m_audioCount; i++) {
            AudioInfo *ainfo = new AudioInfo;
            ainfo->ID           = wstring2QString(m_mediaInfo->Get(Stream_Audio, i, __T("ID"))).toInt();
            ainfo->Channels     = wstring2QString(m_mediaInfo->Get(Stream_Audio, i, __T("Channels"))).toInt();
            ainfo->Duration     = wstring2QString(m_mediaInfo->Get(Stream_Audio, i, __T("Duration"))).toULong();
            ainfo->Bitrate      = wstring2QString(m_mediaInfo->Get(Stream_Audio, i, __T("BitRate"))).toULong();
            ainfo->StreamSize   = wstring2QString(m_mediaInfo->Get(Stream_Audio, i, __T("StreamSize"))).toULong();
            ainfo->SamplingRate = wstring2QString(m_mediaInfo->Get(Stream_Audio, i, __T("SamplingRate"))).toULong();
            ainfo->CodecID      = wstring2QString(m_mediaInfo->Get(Stream_Audio, i, __T("CodecID")));
            ainfo->ChannelLayout= wstring2QString(m_mediaInfo->Get(Stream_Audio, i, __T("ChannelLayout")));

            m_duration = m_duration > ainfo->Duration ? m_duration : ainfo->Duration;
            m_audioMap[i] = ainfo;
        }
    }

    KyInfo() << "File Size:"        << (double)m_fileSize/1024.0/1024.0     << "Mb"
             << "\tDuration:"         << (double)m_duration/1000.0            << "s"
             << "\tm_overallBitRate:"   << (double)m_overallBitRate/1000.0/100.0<< "Mb/s"
             << QString("(%1)").arg(m_filePath);
}

void MediaHandle::unsetAll()
{
    m_filePath = "";
    m_fileSize = 0;
    m_duration = 0;
    m_overallBitRate = 0;

    m_videoCount = 0;
    m_audioCount = 0;
    m_subtitleCount = 0;

    foreach (auto info, m_videoMap) {
        delete info;
        info = nullptr;
    }
    m_videoMap.clear();

    foreach (auto info, m_audioMap) {
        delete info;
        info = nullptr;
    }
    m_audioMap.clear();
}
}
