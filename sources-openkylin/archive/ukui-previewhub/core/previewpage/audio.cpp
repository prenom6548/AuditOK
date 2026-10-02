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

#include "previewpage/audio.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

static QString formatDuration(qint64 milliseconds)
{
    const qint64 seconds = qMax<qint64>(0, milliseconds / 1000);
    const qint64 hours = seconds / 3600;
    const qint64 minutes = seconds % 3600 / 60;
    const qint64 secs = seconds % 60;

    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(secs, 2, 10, QLatin1Char('0'));
    }
    return QString("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(secs, 2, 10, QLatin1Char('0'));
}

AudioPreviewPage::AudioPreviewPage(QWidget *parent) : BasePreviewPage(parent)
{
    m_player = new QMediaPlayer(this, QMediaPlayer::LowLatency);

    /* file information */
    m_icon = new QLabel(this);
    m_icon->setFixedSize(112, 112);
    m_icon->setAlignment(Qt::AlignCenter);

    m_file_name_label = new QLabel(this);
    m_file_name_label->setWordWrap(true);
    m_file_name_label->setStyleSheet("font-size: 24px;");

    m_duration_label = new QLabel(this);
    m_duration_label->setStyleSheet("font-size: 14px;");

    auto *details_layout = new QVBoxLayout;
    details_layout->setContentsMargins(0, 0, 0, 0);
    details_layout->setSpacing(10);
    details_layout->addStretch();
    details_layout->addWidget(m_file_name_label);
    details_layout->addWidget(m_duration_label);
    details_layout->addStretch();

    auto *information_layout = new QHBoxLayout;
    information_layout->setContentsMargins(0, 0, 0, 0);
    information_layout->setSpacing(36);
    information_layout->addWidget(m_icon);
    information_layout->addLayout(details_layout);

    /* media control bar */
    m_control_bar = new MediaControlBar(this);
    m_control_bar->setPreviewWidget(this);
    m_control_bar->setFloating(true);

    connect(m_control_bar, &MediaControlBar::playPauseClicked, this, [this]() {
        switch (m_player->state()) {
        case QMediaPlayer::PlayingState:
            m_player->pause();
            break;
        default:
            m_player->play();
            break;
        }
    });

    connect(m_player, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
        m_control_bar->setPlaying(state == QMediaPlayer::PlayingState);
    });

    /* loop play */
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            m_player->setPosition(0);
            m_player->play();
        }
    });

    connect(m_player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        m_control_bar->setPosition(position);
    });

    connect(m_player, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
        updateDuration(duration);
        m_control_bar->setDuration(duration);
        m_control_bar->setPosition(m_player->position());
    });

    connect(m_control_bar, &MediaControlBar::positionChanged, this, [this](qint64 position) {
        m_player->setPosition(position);
    });

    /* volume */
    connect(m_control_bar, &MediaControlBar::volumeChanged, m_player, &QMediaPlayer::setVolume);
    connect(m_control_bar, &MediaControlBar::muteToggled, this, [this]() {
        m_player->setMuted(!m_player->isMuted());
    });
    connect(m_player, &QMediaPlayer::volumeChanged, m_control_bar, &MediaControlBar::setVolume);
    connect(m_player, &QMediaPlayer::mutedChanged, m_control_bar, &MediaControlBar::setMuted);

    base_layout->setContentsMargins(14, 0, 14, 18);
    base_layout->addStretch();
    base_layout->addLayout(information_layout);
    base_layout->setAlignment(information_layout, Qt::AlignHCenter);
    base_layout->addStretch();

    m_control_bar->setVolume(m_player->volume());
    m_control_bar->setMuted(m_player->isMuted());
}

void AudioPreviewPage::updateInfo(PreviewHubFileInfo *info)
{
    stopPlayback();
    const QIcon icon = QIcon::fromTheme(info->iconName(), QIcon::fromTheme("audio-x-generic"));
    m_icon->setPixmap(icon.pixmap(96, 96));
    m_file_name_label->setText(info->desktopName());
    updateDuration(0);

    m_player->setMedia(QUrl(info->uri()));
    m_player->play();
    m_player->setMuted(false);
    m_control_bar->showBar(true);
}

void AudioPreviewPage::cancel()
{
    qDebug() << "AudioPreviewPage::cancel";
    stopPlayback();
}

void AudioPreviewPage::stopPlayback()
{
    m_control_bar->setPosition(0);

    if (m_player->state() != QMediaPlayer::StoppedState)
        m_player->stop();

    m_player->setMedia(QMediaContent());
    m_control_bar->setDuration(0);
}

void AudioPreviewPage::updateDuration(qint64 duration)
{
    m_duration_label->setText(tr("Duration:") + QLatin1Char(' ') + formatDuration(duration));
}
