/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
 */
#ifndef MMEDIAPLAYER_H
#define MMEDIAPLAYER_H

#include <QObject>
#include <QDebug>
#include "mpvlibload.h"
#include "qthelper.h"
#include "mmediaplaylist.h"

class MMediaPlayer : public QObject
{
    Q_OBJECT
public:
    //          停止播放          播放中       暂停中
    enum State{StoppedState=0,PlayingState,PausedState}; //播放状态枚举
    enum ErrorMsg{NotFound=-1,Damage=-2}; //播放状态枚举
    MMediaPlayer(QObject *parent = nullptr);

    void setPlaylist(MMediaPlaylist *playlist); //设置播放列表
    void pause(); //暂停或继续
    void pauseOnly(); // 暂停

    State state() const; //获取状态
    qint64 position() const; //获取当前播放点
    void setPosition(qint64 pos); //设置播放起始点

    bool isAvailable() const; //暂无实际功能
    void setVolume(int vol); //设置音量
    qint64 duration() const; //获取总时长
    void setMedia(const MMediaContent &media); //设置待播放媒体
    void play(); //播放
public Q_SLOTS:
    void stop(); //停止
    void onMpvEvents(); //接收mpv事件

private:
    bool m_canPlay = true;     
    void truePlay(QString startTime = "0"); //实际的播放函数
    void handle_mpv_event(mpv_event *event); // 处理mpv事件
    void createMvpplayer(); // 创建mvpPlayer
    void setProperty(const QString &name, const QString &value); // 设置mpv属性
    QString getProperty(const QString &name) const; // 获得mpv属性
    void changeState(State stateNow); //改变状态
    MMediaPlaylist * m_playList = nullptr; //私有播放列表
    MMediaPlaylist * m_tmpPlayList = nullptr; //私有临时播放列表
    mpv_handle *m_mpvPlayer = nullptr;//句柄
    State m_state = StoppedState;//播放状态
    QByteArray filenameBack = ""; //上次播放的媒体名
    bool m_positionChangeed = false; //播放进度被设置
    qint64 m_position = 0; //播放进度
    qint64 m_duration = 0; //总时长
private Q_SLOTS:
    void autoPlay(MMediaPlaylist::PlaybackMode playbackMode); //自动播放
Q_SIGNALS:
    void mpvEvents(); // 触发onMpvEvents()槽函数的信号
    void stateChanged(MMediaPlayer::State); //状态改变信号
    void durationChanged(qint64); //切换媒体时，总时长改变信号
    void positionChanged(qint64); //播放进度改变信号
    void playFinish(); //媒体播放完成信号
    void playError(); //媒体播放错误信号
    void playErrorMsg(ErrorMsg errorCode);//媒体播放错误信息信号
//    void signalVolume(int);
};

#endif // MMEDIAPLAYER_H
