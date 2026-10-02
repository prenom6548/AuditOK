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

#include <QStyle>

AudioPreviewPage::AudioPreviewPage(QWidget *parent) : BasePreviewPage(parent)
{
    m_player = new QMediaPlayer(this, QMediaPlayer::LowLatency);

    /* play button */
    m_button = new QPushButton(this);
    m_button->setIcon(style()->standardIcon(QStyle::SP_MediaPause));

    connect(m_button, &QPushButton::clicked, this, [=]() {
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
        switch(state) {
        case QMediaPlayer::PlayingState:
            m_button->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            timer->start();
            break;
        default:
            m_button->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            timer->stop();
            break;
        }
    });

    /* loop play */
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            m_player->setPosition(0);
            m_player->play();
        }
    });

    /* time event */
    timer = new QTimer();
    timer->setInterval(1000 / 4);
    connect(timer, &QTimer::timeout, this, [=](){
        m_progress->setValue(m_player->position());
        qDebug() << m_player->position() << " " << m_player->duration() ;
    });

    /* position slider */
    m_progress = new Slider(this);
    connect(m_player, &QMediaPlayer::durationChanged, this, [=]() {
        m_progress->setRange(0, m_player->duration());
    });
    m_progress->setValue(0);
    connect(m_progress, &Slider::sliderMoved, this, [=]() {
        timer->stop();
    });
    connect(m_progress, &Slider::sliderReleased, this, [=]() {
        m_player->setPosition(m_progress->value());
        timer->start();
    });
    connect(m_progress, &Slider::SliderClicked, this, [=]() {
        m_player->setPosition(m_progress->value());
    });

    /* icon */
    m_icon = new IconButton(this);
    m_icon->setIconSize(QSize(96, 96));
    connect(m_player, QOverload<>::of(&QMediaPlayer::metaDataChanged), this, &AudioPreviewPage::updateMeta);

    /* set layout */
    m_form = new QFormLayout;
    m_title_label = new QLabel(this);
    m_title_label->setWordWrap(true);
    m_form->addRow(tr("Title:"), m_title_label);
    m_author_label = new QLabel(this);
    m_form->addRow(tr("Author:"), m_author_label);

    m_form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    m_form->setFormAlignment(Qt::AlignHCenter);
    m_form->setLabelAlignment(Qt::AlignRight);

    base_layout->addWidget(m_icon);
    QWidget *form = new QWidget(this);
    form->setLayout(m_form);
    form->setMaximumHeight(100);
    base_layout->addWidget(form);
    base_layout->addWidget(m_progress);
    base_layout->addWidget(m_button);
}

void AudioPreviewPage::updateInfo(PreviewHubFileInfo *info) {
    m_player->setMedia(QUrl(info->uri()));
    m_player->play();
    timer->start();
    m_base_icon = QIcon::fromTheme(info->iconName(), QIcon::fromTheme("text-x-generic"));
    // default value
    m_title_label->setText(info->desktopName());
    m_author_label->setText(tr("unknown"));
}

void AudioPreviewPage::cancel() {
    qDebug() << "AudioPreviewPage::cancel";
    m_player->stop();
}

void AudioPreviewPage::updateMeta() {
    QVariant cover_val = m_player->metaData("CoverArtImage");
    const QIcon &icon = QIcon(QPixmap::fromImage(cover_val.value<QImage>()));
    m_icon->setIcon(icon.isNull() ? m_base_icon : icon);

    QVariant title = m_player->metaData("Title");
    if (title.isValid())
        m_title_label->setText(title.toString());

    QVariant autor = m_player->metaData("ContributingArtist");
    if (autor.isValid())
        m_author_label->setText(autor.toString());
    else {
        QVariant composer = m_player->metaData("Composer");
        if(composer.isValid())
            m_author_label->setText(composer.toString());
    }
    // qDebug() << m_player->availableMetaData() << "\n";
}


void AudioPreviewPage::resizeEvent(QResizeEvent* event) {
    int width = event->size().width() - 50;
    width = qMax(width, 96);
    width = qMin(width, 256);
    m_icon->setIconSize(QSize(width, width * 2/3));
}
