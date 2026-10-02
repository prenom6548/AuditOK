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

#ifndef VIDEOPREVIEWPAGE_H
#define VIDEOPREVIEWPAGE_H

#include <QMediaPlayer>
#include <QGraphicsVideoItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>

#include "previewpage/base.h"
#include "components/slider.h"

class VideoPreviewPage : public BasePreviewPage
{
    Q_OBJECT
public:
    explicit VideoPreviewPage(QWidget *parent = nullptr);
    ~VideoPreviewPage();

private:
    QMediaPlayer *m_player;
    QGraphicsVideoItem *m_video_item;
    QGraphicsScene *m_video_scene;
    QGraphicsView *m_video_view;
    QPushButton *m_play_button;
    Slider *m_position_slider;
    QTimer *timer;

    // BasePreviewPage interface
public:
    void cancel() override;

public Q_SLOTS:
    void updateInfo(PreviewHubFileInfo *info) override;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // VIDEOPREVIEWPAGE_H
