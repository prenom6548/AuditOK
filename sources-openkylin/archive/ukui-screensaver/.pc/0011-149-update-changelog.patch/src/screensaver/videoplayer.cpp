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

VideoPlayer::VideoPlayer(QWidget *parent) : QWidget(parent)
{
    m_player = new QMediaPlayer(this);
    connect(m_player, &QMediaPlayer::positionChanged, this, &VideoPlayer::updatePosition);

    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &VideoPlayer::mediaStatusChanged);
    connect(
        m_player,
        static_cast<void (QMediaPlayer::*)(QMediaPlayer::Error)>(&QMediaPlayer::error),
        this,
        &VideoPlayer::showError);
}

VideoPlayer::~VideoPlayer()
{
    if (m_player != nullptr) {
        m_player->stop();
        m_player->deleteLater();
        m_player = nullptr;
    }
}

void VideoPlayer::setOutput(QGraphicsVideoItem *videoWidget, int duration)
{
    video_Duration = duration;
    m_player->setVideoOutput(videoWidget);
    m_player->setMuted(true); //视频静音
    m_player->setPosition(1); //避免首次打开视频是黑屏
    m_player->play();
}

void VideoPlayer::setMediaFile(QString filePath)
{
    m_filePath = filePath;
    m_player->setMedia(QMediaContent(QUrl::fromLocalFile(filePath)));
}

void VideoPlayer::updatePosition(qint64 position)
{
    //因为用wps制作的视频用qmediaplayer去获取视频时长有问题，会在视频最后一帧停留很长时间，所以用ffmpeg获取视频时长，手动循环播放视频
    qDebug() << "position =" << position << "player->duration() = " << m_player->duration()
             << "duration = " << video_Duration;
    if (video_Duration || m_player->duration()) {
        if (position && position > (video_Duration > 0 ? video_Duration : m_player->duration())) {
            m_player->setMedia(QMediaContent(QUrl::fromLocalFile(m_filePath)));
            m_player->setPosition(1);
            m_player->play();
        } else if (position == 0 && m_player->duration()) { //有些wps做的视频播放到最后一帧会自己跳到0
            m_player->setMedia(QMediaContent(QUrl::fromLocalFile(m_filePath)));
            m_player->setPosition(1);
            m_player->play();
        } else if (position && (position == video_Duration || position == m_player->duration())) { //视频当前播放帧数 = 视频总时长
            m_player->setMedia(QMediaContent(QUrl::fromLocalFile(m_filePath)));
            m_player->setPosition(1);
            m_player->play();
        }
    }
}

void VideoPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status)
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

void VideoPlayer::showError(QMediaPlayer::Error error)
{
    switch (error) {
        case QMediaPlayer::NoError:
            qDebug() << "没有错误！";
            break;
        case QMediaPlayer::ResourceError:
            qDebug() << "媒体资源无法被解析！";
            break;
        case QMediaPlayer::FormatError:
            qDebug() << "不支持该媒体格式！";
            break;
        case QMediaPlayer::NetworkError:
            qDebug() << "发生了一个网络错误！";
            break;
        case QMediaPlayer::AccessDeniedError:
            qDebug() << "没有播放权限！";
            break;
        case QMediaPlayer::ServiceMissingError:
            qDebug() << "没有发现有效的播放服务！";
            break;
        default:
            break;
    }
}
