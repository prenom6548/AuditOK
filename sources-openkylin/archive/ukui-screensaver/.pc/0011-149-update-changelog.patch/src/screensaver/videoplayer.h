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
#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QGraphicsView>
#include <QGraphicsVideoItem>

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = 0);
    ~VideoPlayer();
    void setOutput(QGraphicsVideoItem *videoWidget, int duration);
    void setMediaFile(QString filePath);

private slots:
    void updatePosition(qint64 position);

    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void showError(QMediaPlayer::Error error);

private:
    QMediaPlayer *m_player;
    int video_Duration = 0;
    QString m_filePath;
};

#endif // VIDEOPLAYER_H
