/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2026, KylinSoft Co, Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#include "components/media-control-bar.h"

#include <QCursor>
#include <QEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QSignalBlocker>
#include <QStyle>
#include <QVBoxLayout>

#include <limits>

MediaControlBar::MediaControlBar(QWidget *parent)
    : PreviewControlBar(parent)
{
    /* play button */
    m_play_button = new QPushButton(this);
    m_play_button->setIcon(mediaIcon(QStyle::SP_MediaPlay));
    m_play_button->setFixedSize(28, 28);
    m_play_button->setToolTip(tr("Play"));

    connect(m_play_button, &QPushButton::clicked, this, &MediaControlBar::playPauseClicked);

    /* position slider */
    m_position_slider = new Slider(this);
    m_position_slider->setProperty("handleVisible", false);

    m_current_time_label = new QLabel(formatTime(0), this);
    m_current_time_label->setAlignment(Qt::AlignCenter);
    m_current_time_label->setMinimumWidth(42);

    connect(m_position_slider, &Slider::valueChanged, this, [=](int position) {
        m_current_time_label->setText(formatTime(position));
    });
    connect(m_position_slider, &Slider::sliderReleased, this, [=]() {
        Q_EMIT positionChanged(m_position_slider->value());
    });

    m_duration_time_label = new QLabel(formatTime(0), this);
    m_duration_time_label->setAlignment(Qt::AlignCenter);
    m_duration_time_label->setMinimumWidth(42);

    m_mute_button = new QPushButton(this);
    m_mute_button->setIcon(mediaIcon(QStyle::SP_MediaVolume));
    m_mute_button->setFixedSize(28, 28);
    m_mute_button->setToolTip(tr("Volume"));
    connect(m_mute_button, &QPushButton::clicked, this, [=]() {
        if (m_volume_popup->isHidden()) {
            showVolumePopup(true);
        } else {
            Q_EMIT muteToggled();
        }
    });

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 7, 14, 7);
    layout->setSpacing(8);
    layout->addWidget(m_play_button);
    layout->addWidget(m_current_time_label);
    layout->addWidget(m_position_slider, 1);
    layout->addWidget(m_duration_time_label);
    layout->addWidget(m_mute_button);

    m_volume_popup = new QWidget(parent ? parent : this);
    m_volume_popup->setObjectName("videoVolumePopup");
    m_volume_popup->setAttribute(Qt::WA_StyledBackground, true);
    m_volume_popup->hide();

    m_volume_label = new QLabel(m_volume_popup);
    m_volume_label->setAlignment(Qt::AlignCenter);
    m_volume_label->setMinimumWidth(36);

    m_volume_slider = new QSlider(Qt::Vertical, m_volume_popup);
    m_volume_slider->setRange(0, 100);
    m_volume_slider->setValue(m_volume);
    m_volume_slider->setFixedWidth(26);
    m_volume_slider->setFocusPolicy(Qt::NoFocus);
    connect(m_volume_slider, &QSlider::valueChanged, this, [=](int value) {
        setVolume(value);
        if (value > 0 && m_is_muted) {
            m_is_muted = false;
            Q_EMIT muteToggled();
        }
        Q_EMIT volumeChanged(value);
    });

    auto *volumeLayout = new QVBoxLayout(m_volume_popup);
    volumeLayout->setContentsMargins(8, 8, 8, 10);
    volumeLayout->setSpacing(4);
    volumeLayout->addWidget(m_volume_label);
    volumeLayout->addWidget(m_volume_slider, 1, Qt::AlignHCenter);

    /* install event filters for mouse hover handling */
    m_position_slider->installEventFilter(this);
    m_volume_popup->installEventFilter(this);

    /* watch parent for click-outside-to-hide-volume-popup */
    if (parent)
        parent->installEventFilter(this);
}

MediaControlBar::~MediaControlBar()
{
    delete m_volume_popup;
}

void MediaControlBar::setDuration(qint64 duration) {
    const int sliderMaximum = static_cast<int>(qBound<qint64>(
        0, duration, std::numeric_limits<int>::max()));
    m_position_slider->setRange(0, sliderMaximum);
    m_duration_time_label->setText(formatTime(duration));
}

void MediaControlBar::setPosition(qint64 position) {
    if (m_position_slider->isSliderDown())
        return;

    const int sliderPosition = static_cast<int>(qBound<qint64>(
        m_position_slider->minimum(), position, m_position_slider->maximum()));
    m_position_slider->setValue(sliderPosition);
}

void MediaControlBar::setPlaying(bool playing) {
    m_is_playing = playing;
    m_play_button->setIcon(mediaIcon(playing ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
    m_play_button->setToolTip(playing ? tr("Pause") : tr("Play"));
}

void MediaControlBar::setVolume(int volume) {
    m_volume = volume;
    if (!m_is_muted && m_volume_slider->value() != volume) {
        const QSignalBlocker blocker(m_volume_slider);
        m_volume_slider->setValue(volume);
    }
    updateVolumeControls();
}

void MediaControlBar::setMuted(bool muted) {
    m_is_muted = muted;
    const QSignalBlocker blocker(m_volume_slider);
    m_volume_slider->setValue(muted ? 0 : m_volume);
    updateVolumeControls();
}

void MediaControlBar::showBar(bool visible) {
    if (!visible)
        showVolumePopup(false);
    PreviewControlBar::showBar(visible);
}

bool MediaControlBar::eventFilter(QObject *watched, QEvent *event) {
    /* handle click-outside to hide volume popup */
    if (isPreviewObject(watched) && event->type() == QEvent::MouseButtonPress &&
        !m_volume_popup->isHidden()) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        QPoint globalPos = me->globalPos();
        QPoint localInMute = m_mute_button->mapFromGlobal(globalPos);
        QPoint localInPopup = m_volume_popup->mapFromGlobal(globalPos);
        if (!m_mute_button->rect().contains(localInMute) &&
            !m_volume_popup->rect().contains(localInPopup)) {
            showVolumePopup(false);
        }
    }

    /* handle events on our own sub-widgets */
    if (watched == m_position_slider || watched == m_volume_popup) {
        switch (event->type()) {
        case QEvent::Enter:
            if (watched == m_position_slider)
                m_position_slider->setProperty("handleVisible", true);
            break;
        case QEvent::Leave:
            if (watched == m_position_slider)
                m_position_slider->setProperty("handleVisible", false);
            break;
        default:
            break;
        }
        /* refresh the custom-painted handle after property change */
        if ((watched == m_position_slider) &&
            (event->type() == QEvent::Enter || event->type() == QEvent::Leave)) {
            m_position_slider->update();
        }
    }

    return PreviewControlBar::eventFilter(watched, event);
}

void MediaControlBar::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    positionVolumePopup();
}

bool MediaControlBar::isInteractionActive() const {
    if (PreviewControlBar::isInteractionActive() ||
        m_position_slider->isSliderDown() || m_volume_slider->isSliderDown()) {
        return true;
    }

    return m_volume_popup && m_volume_popup->isVisible() &&
           m_volume_popup->rect().contains(
               m_volume_popup->mapFromGlobal(QCursor::pos()));
}

QString MediaControlBar::themeStyleSheet(bool dark) const {
    const QString text = dark ? "#f1f1f1" : "#2d2d2d";
    const QString hover = dark ? "rgba(255, 255, 255, 28)" : "rgba(0, 0, 0, 18)";
    return PreviewControlBar::themeStyleSheet(dark) + QString(
        "#previewControlBar QPushButton {"
        "border: 0px;"
        "background: transparent;"
        "padding: 4px;"
        "}"
        "#previewControlBar QPushButton:hover {"
        "background: %1;"
        "border-radius: 4px;"
        "}"
        "#previewControlBar QLabel {"
        "color: %2;"
        "font-size: 12px;"
        "background: transparent;"
        "border: none;"
        "}")
        .arg(hover, text);
}

void MediaControlBar::themeChanged(bool dark) {
    const QString popupBackground = dark ? "rgba(42, 42, 42, 235)" : "rgba(255, 255, 255, 235)";
    const QString border = dark ? "rgba(255, 255, 255, 32)" : "rgba(0, 0, 0, 18)";
    const QString text = dark ? "#f1f1f1" : "#2d2d2d";
    const QString sliderBg = dark ? "rgba(255, 255, 255, 42)" : "rgba(0, 0, 0, 25)";
    const QString handle = dark ? "#f4f4f4" : "#ffffff";
    const QColor sliderColor = dark ? QColor(255, 255, 255, 42) : QColor(0, 0, 0, 25);
    const QString positionSliderStyleSheet = QString(
        "QSlider {"
        "min-height: 18px;"
        "background: transparent;"
        "border: none;"
        "}");
    const QString popupStyleSheet = QString(
        "#videoVolumePopup {"
        "background-color: %1;"
        "border: 1px solid %2;"
        "border-radius: 8px;"
        "}"
        "#videoVolumePopup QLabel {"
        "color: %3;"
        "font-size: 12px;"
        "}")
        .arg(popupBackground, border, text);
    const QString volumeSliderStyleSheet = QString(
        "QSlider {"
        "background: transparent;"
        "border: none;"
        "padding: 0px;"
        "}"
        "QSlider::groove:vertical {"
        "width: 4px;"
        "border-radius: 2px;"
        "background: %1;"
        "border: none;"
        "}"
        "QSlider::sub-page:vertical {"
        "width: 4px;"
        "border-radius: 2px;"
        "background: %1;"
        "border: none;"
        "}"
        "QSlider::add-page:vertical {"
        "width: 4px;"
        "border-radius: 2px;"
        "background: #2f88ff;"
        "border: none;"
        "}"
        "QSlider::handle:vertical {"
        "width: 12px;"
        "height: 12px;"
        "margin: 0px -4px;"
        "border-radius: 6px;"
        "background: %2;"
        "border: 2px solid #2f88ff;"
        "}")
        .arg(sliderBg, handle);
    if (m_position_slider->styleSheet() != positionSliderStyleSheet)
        m_position_slider->setStyleSheet(positionSliderStyleSheet);
    m_position_slider->setProgressColors(QColor("#2f88ff"), sliderColor, QColor(handle));
    if (m_volume_popup->styleSheet() != popupStyleSheet)
        m_volume_popup->setStyleSheet(popupStyleSheet);
    if (m_volume_slider->styleSheet() != volumeSliderStyleSheet)
        m_volume_slider->setStyleSheet(volumeSliderStyleSheet);

    m_play_button->setIcon(mediaIcon(m_is_playing ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
    updateVolumeControls();
}

void MediaControlBar::updateVolumeControls() {
    const int vol = m_is_muted ? 0 : m_volume;
    m_volume_label->setText(QString("%1%").arg(vol));
    m_mute_button->setIcon(mediaIcon(vol == 0 ? QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume));
}

void MediaControlBar::positionVolumePopup() {
    if (!parentWidget())
        return;

    const QPoint buttonCenter = m_mute_button->mapTo(parentWidget(), QPoint(m_mute_button->width() / 2, 0));
    const int containerWidth = parentWidget()->width();
    const int popupWidth = 48;
    const int maxHeight = qMax(96, buttonCenter.y() - 18);
    const int popupHeight = qMin(150, maxHeight);
    int popupX = buttonCenter.x() - popupWidth / 2;
    popupX = qBound(8, popupX, qMax(8, containerWidth - popupWidth - 8));
    int popupY = buttonCenter.y() - popupHeight - 10;
    popupY = qMax(8, popupY);
    m_volume_popup->setGeometry(popupX, popupY, popupWidth, popupHeight);
}

void MediaControlBar::showVolumePopup(bool visible) {
    if (visible) {
        positionVolumePopup();
        m_volume_popup->show();
        m_volume_popup->raise();
        return;
    }
    m_volume_popup->hide();
}

QIcon MediaControlBar::mediaIcon(QStyle::StandardPixmap standardPixmap) const {
    QPixmap pixmap = style()->standardIcon(standardPixmap).pixmap(18, 18);
    if (pixmap.isNull())
        return QIcon();

    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), isDarkTheme() ? QColor("#f1f1f1") : QColor("#202020"));
    painter.end();
    return QIcon(pixmap);
}

QString MediaControlBar::formatTime(qint64 milliseconds) const {
    const qint64 seconds = qMax<qint64>(0, milliseconds / 1000);
    const qint64 hours = seconds / 3600;
    const qint64 minutes = seconds % 3600 / 60;
    const qint64 secs = seconds % 60;

    if (hours > 0)
        return QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QLatin1Char('0')).arg(secs, 2, 10, QLatin1Char('0'));
    return QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(secs, 2, 10, QLatin1Char('0'));
}
