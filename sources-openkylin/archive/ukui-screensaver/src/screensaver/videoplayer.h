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
#ifndef _KYVIDEOPLAYER_H
#define _KYVIDEOPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QGraphicsView>
#include <QGraphicsVideoItem>

class KyVideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit KyVideoPlayer(QWidget *parent = 0);
    ~KyVideoPlayer();
    void setOutput(QGraphicsVideoItem *videoWidget, int duration);
    void setMediaFile(QString filePath);
    void stop();

Q_SIGNALS:
    void playerShowError(QMediaPlayer::Error error);

private slots:
    void updatePosition(qint64 position);

    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onShowError(QMediaPlayer::Error error);

private:
    QMediaPlayer *m_mediaPlayer;
    int playerFlag = 0;
    bool m_isActiveStop = false;
    int m_nVideoDuration = 0;
    QString m_filePath;
};

#endif // VIDEOPLAYER_H
