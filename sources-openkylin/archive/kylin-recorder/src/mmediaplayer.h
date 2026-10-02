/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Xin Li <lixin@kylinos.cn>
 */
#ifndef MMEDIAPLAYER_H
#define MMEDIAPLAYER_H

#include <QObject>
#include <QUrl>
#include <QDebug>
//#include <client.h> // MPV库头文件
#include <QMediaPlayer>
#include "mpvlib/mpvlibload.h"

class MMediaPlayer : public QObject
{
    Q_OBJECT
public:
    enum State { StoppedState = 0, PlayingState, PausedState };
    MMediaPlayer(QObject *parent = nullptr);
    ~MMediaPlayer();

    void playPause(qint64 pos = 0);

    State state() const;
    qint64 position() const;
    void setPosition(qint64 pos);

    bool isAvailable() const;
    void setVolume(int vol);
    qint64 duration() const;
    void setMedia(QString media);
    void play(qint64 pos = 0);
public slots:
    void stop();
    void onMpvEvents(); //接收mpv事件

private:
    void truePlay(QString startTime = "0");
    void handle_mpv_event(mpv_event *event);                     // 处理mpv事件
    void createMvpplayer();                                      // 创建mvpPlayer
    void setProperty(const QString &name, const QString &value); // 设置mpv属性
    QString getProperty(const QString &name) const;              // 获得mpv属性
    void changeState(State stateNow);

    mpv_handle *m_mpvPlayer = nullptr; //句柄
    State m_state = StoppedState;      //播放状态
    QByteArray filenameBack = "";
    bool m_positionChangeed = false;
    qint64 m_position = 0;
    qint64 m_duration = 0;
    QString m_lastFileName { "" };

signals:
    void mpvEvents(); // 触发onMpvEvents()槽函数的信号
    void stateChanged(QMediaPlayer::State);
    void durationChanged(qint64);
    void positionChanged(qint64);
    void playFinish();
};

#endif // MMEDIAPLAYER_H
