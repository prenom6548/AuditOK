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

#ifndef AUDIOPREVIEWPAGE_H
#define AUDIOPREVIEWPAGE_H

#include <QMediaPlayer>
#include <QLabel>

#include "previewpage/base.h"
#include "components/media-control-bar.h"

class AudioPreviewPage : public BasePreviewPage
{
    Q_OBJECT
public:
    explicit AudioPreviewPage(QWidget *parent = nullptr);

private:
    void stopPlayback();
    void updateDuration(qint64 duration);

    QMediaPlayer *m_player;
    QLabel *m_icon;
    QLabel *m_file_name_label;
    QLabel *m_duration_label;
    MediaControlBar *m_control_bar;

    // BasePreviewPage interface
public Q_SLOTS:
    void updateInfo(PreviewHubFileInfo *info) override;
public:
    void cancel() override;
};

#endif // AUDIOPREVIEWPAGE_H
