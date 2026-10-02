/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co, Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Lu Hongxu <lhongxu@outlook.com>
 *
 */

#include "previewpage/video.h"

#include <QResizeEvent>

VideoPreviewPage::VideoPreviewPage(QWidget *parent) : BasePreviewPage(parent)
{
    m_player = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    m_video_item = new QGraphicsVideoItem;
    m_video_item->setAspectRatioMode(Qt::KeepAspectRatio);
    m_video_scene = new QGraphicsScene(this);
    m_video_container = new QWidget(this);
    m_video_container->setMouseTracking(true);
    m_video_view = new QGraphicsView(m_video_scene, m_video_container);
    m_video_view->setDragMode(QGraphicsView::NoDrag);
    m_video_view->setAlignment(Qt::AlignCenter);
    m_video_view->setFrameShape(QFrame::NoFrame);
    m_video_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_video_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_video_view->setStyleSheet("padding:0px;border:0px;background:transparent");
    m_video_view->setMouseTracking(true);
    m_video_view->viewport()->setMouseTracking(true);

    m_video_scene->addItem(m_video_item);
    m_player->setVideoOutput(m_video_item);

    /* media control bar */
    m_control_bar = new MediaControlBar(m_video_container);
    m_control_bar->setPreviewWidget(m_video_container);
    m_control_bar->setFloating(true);

    /* play/pause */
    connect(m_control_bar, &MediaControlBar::playPauseClicked, this, [=]() {
        switch (m_player->state()) {
        case QMediaPlayer::PlayingState:
            m_player->pause();
            break;
        default:
            m_player->play();
            break;
        }
    });

    connect(m_player, &QMediaPlayer::stateChanged, this, [=](QMediaPlayer::State state) {
        m_control_bar->setPlaying(state == QMediaPlayer::PlayingState);
    });

    /* loop play */
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            // Reloading avoids a GStreamer "internal data stream error" on replay.
            m_player->setMedia(m_player->media());
            m_player->setPosition(0);
            m_player->play();
        }
    });

    connect(m_player, &QMediaPlayer::positionChanged, this, [=](qint64 position) {
        m_control_bar->setPosition(position);
    });

    /* position slider */
    connect(m_player, &QMediaPlayer::durationChanged, this, [=](qint64 duration) {
        m_control_bar->setDuration(duration);
        m_control_bar->setPosition(m_player->position());
    });

    connect(m_control_bar, &MediaControlBar::positionChanged, this, [=](qint64 position) {
        m_player->setPosition(position);
    });

    /* volume */
    connect(m_control_bar, &MediaControlBar::volumeChanged, this, [=](int volume) {
        m_player->setVolume(volume);
    });
    connect(m_control_bar, &MediaControlBar::muteToggled, this, [=]() {
        m_player->setMuted(!m_player->isMuted());
    });
    connect(m_player, &QMediaPlayer::volumeChanged, this, [=](int volume) {
        m_control_bar->setVolume(volume);
    });
    connect(m_player, &QMediaPlayer::mutedChanged, this, [=](bool muted) {
        m_control_bar->setMuted(muted);
    });

    /* resize event */
    connect(m_video_item, &QGraphicsVideoItem::nativeSizeChanged, this, [=]() {
        updateVideoGeometry();
    });

    /* layout */
    base_layout->setContentsMargins(0, 0, 0, 0);
    base_layout->addWidget(m_video_container);

    m_control_bar->setVolume(m_player->volume());
    m_control_bar->setMuted(m_player->isMuted());
}

VideoPreviewPage::~VideoPreviewPage() {
    stopPlayback();
}

void VideoPreviewPage::updateInfo(PreviewHubFileInfo *info) {
    stopPlayback();
    m_player->setMedia(QUrl(info->uri()));
    m_player->play();
    m_player->setMuted(false);
    m_control_bar->showBar(true);
}

void VideoPreviewPage::cancel() {
    stopPlayback();
}

void VideoPreviewPage::stopPlayback() {
    m_control_bar->setPosition(0);

    if (m_player->state() != QMediaPlayer::StoppedState)
        m_player->stop();

    m_player->setMedia(QMediaContent());
    m_control_bar->setDuration(0);
}

void VideoPreviewPage::resizeEvent(QResizeEvent *event) {
    BasePreviewPage::resizeEvent(event);
    updateVideoGeometry();
}

void VideoPreviewPage::updateVideoGeometry() {
    const QRect rect = m_video_container->rect();
    m_video_view->setGeometry(rect);
    m_video_scene->setSceneRect(rect);
    m_video_item->setPos(0, 0);
    m_video_item->setSize(rect.size());
}
