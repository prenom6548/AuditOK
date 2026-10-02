/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "videoplayer.h"
#include <QDir>
#include <QTimer>

KyVideoPlayer::KyVideoPlayer(QWidget *parent) : QWidget(parent)
{
    m_mediaPlayer = new QMediaPlayer(this);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &KyVideoPlayer::updatePosition);

    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &KyVideoPlayer::mediaStatusChanged);
    connect(
        m_mediaPlayer,
        static_cast<void (QMediaPlayer::*)(QMediaPlayer::Error)>(&QMediaPlayer::error),
        this,
        &KyVideoPlayer::onShowError);
}

KyVideoPlayer::~KyVideoPlayer()
{
    if (m_mediaPlayer != nullptr) {
        m_mediaPlayer->stop();
        m_mediaPlayer->deleteLater();
        m_mediaPlayer = nullptr;
    }
}

void KyVideoPlayer::setOutput(QGraphicsVideoItem *videoWidget, int duration)
{
    m_nVideoDuration = duration;
    m_mediaPlayer->setVideoOutput(videoWidget);
    m_mediaPlayer->setMuted(true); //视频静音
    m_mediaPlayer->setPosition(1); //避免首次打开视频是黑屏
    m_mediaPlayer->play();
}

void KyVideoPlayer::setMediaFile(QString filePath)
{
    m_filePath = filePath;
    m_mediaPlayer->setMedia(QMediaContent(QUrl::fromLocalFile(filePath)));
}

void KyVideoPlayer::stop()
{
    qDebug() << "m_mediaPlayer->state() = " << m_mediaPlayer->state();
    if (m_mediaPlayer && m_mediaPlayer->state() == QMediaPlayer::PlayingState) {
        m_mediaPlayer->stop();
        m_isActiveStop = true;
    }
}

void KyVideoPlayer::updatePosition(qint64 position)
{
    if (m_isActiveStop)
        return ;
    //因为用wps制作的视频用qmediaplayer去获取视频时长有问题，会在视频最后一帧停留很长时间，所以用ffmpeg获取视频时长，手动循环播放视频
    qDebug() << "position =" << position << "player->duration() = " << m_mediaPlayer->duration()
             << "duration = " << m_nVideoDuration;
    if (m_nVideoDuration || m_mediaPlayer->duration()) {
        if (position && position > (m_nVideoDuration > 0 ? m_nVideoDuration : m_mediaPlayer->duration())) {
            m_mediaPlayer->setMedia(QMediaContent(QUrl::fromLocalFile(m_filePath)));
            m_mediaPlayer->setPosition(1);
            m_mediaPlayer->play();
        } else if (position == 0 && m_mediaPlayer->duration()) { //有些wps做的视频播放到最后一帧会自己跳到0
            m_mediaPlayer->setMedia(QMediaContent(QUrl::fromLocalFile(m_filePath)));
            m_mediaPlayer->setPosition(1);
            m_mediaPlayer->play();
        } else if (position && (position == m_nVideoDuration || position == m_mediaPlayer->duration())) { //视频当前播放帧数 = 视频总时长
            m_mediaPlayer->setMedia(QMediaContent(QUrl::fromLocalFile(m_filePath)));
            m_mediaPlayer->setPosition(1);
            m_mediaPlayer->play();
        }
    }
}

void KyVideoPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status) {
        case QMediaPlayer::UnknownMediaStatus:
            qDebug() << "UnknownMediaStatus!!!";
            break;
        case QMediaPlayer::NoMedia:
            qDebug() << "NoMedia!!!";
            break;
        case QMediaPlayer::BufferingMedia:
            qDebug() << "BufferingMedia!!!";
            break;
        case QMediaPlayer::BufferedMedia:
            qDebug() << "The player has fully buffered the current media!!!";
            break;
        case QMediaPlayer::LoadingMedia:
            qDebug() << "LoadingMedia!!!";
            break;
        case QMediaPlayer::StalledMedia:
            qDebug() << "StalledMedia!!!";
            break;
        case QMediaPlayer::EndOfMedia:
            qDebug() << "EndOfMedia!!!";
            break;
        case QMediaPlayer::LoadedMedia:
            qDebug() << "LoadedMedia!!!";
            break;
        case QMediaPlayer::InvalidMedia:
            qDebug() << "InvalidMedia!!!";
            break;
        default:
            break;
    }
}

void KyVideoPlayer::onShowError(QMediaPlayer::Error error)
{
    switch (error) {
    case QMediaPlayer::NoError:
        qDebug() << "没有错误！" ;
        break;
    case QMediaPlayer::ResourceError:
        qDebug() << "媒体资源无法被解析！";
        stop();
        Q_EMIT playerShowError(error);
        break;
    case QMediaPlayer::FormatError:
        qDebug() << "不支持该媒体格式！";
        stop();
        Q_EMIT playerShowError(error);
        break;
    case QMediaPlayer::NetworkError:
        qDebug() << "发生了一个网络错误！";
        stop();
        Q_EMIT playerShowError(error);
        break;
    case QMediaPlayer::AccessDeniedError:
        qDebug() << "没有播放权限！";
        stop();
        Q_EMIT playerShowError(error);
        break;
    case QMediaPlayer::ServiceMissingError:
        qDebug() << "没有发现有效的播放服务！";
        stop();
        Q_EMIT playerShowError(error);
        break;
    default: break;
    }
}
