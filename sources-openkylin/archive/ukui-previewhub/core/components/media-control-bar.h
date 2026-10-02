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

#ifndef MEDIACONTROLBAR_H
#define MEDIACONTROLBAR_H

#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QSlider>
#include <QStyle>

#include "components/preview-control-bar.h"
#include "components/slider.h"

class MediaControlBar : public PreviewControlBar
{
    Q_OBJECT
public:
    explicit MediaControlBar(QWidget *parent = nullptr);
    ~MediaControlBar() override;

    void setDuration(qint64 duration);
    void setPosition(qint64 position);
    void setPlaying(bool playing);
    void setVolume(int volume);
    void setMuted(bool muted);
    void showBar(bool visible) override;

Q_SIGNALS:
    void playPauseClicked();
    void positionChanged(qint64 position);
    void volumeChanged(int volume);
    void muteToggled();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool isInteractionActive() const override;
    QString themeStyleSheet(bool dark) const override;
    void themeChanged(bool dark) override;

private:
    void updateVolumeControls();
    void positionVolumePopup();
    void showVolumePopup(bool visible);
    QIcon mediaIcon(QStyle::StandardPixmap standardPixmap) const;
    QString formatTime(qint64 milliseconds) const;

    QPushButton *m_play_button;
    QPushButton *m_mute_button;
    QLabel *m_current_time_label;
    QLabel *m_duration_time_label;
    Slider *m_position_slider;
    QPointer<QWidget> m_volume_popup;
    QLabel *m_volume_label;
    QSlider *m_volume_slider;

    bool m_is_playing = false;
    bool m_is_muted = false;
    int m_volume = 100;
};

#endif // MEDIACONTROLBAR_H
