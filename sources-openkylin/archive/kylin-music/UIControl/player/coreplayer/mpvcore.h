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

#ifndef MpvCore_H
#define MpvCore_H

#include <QObject>
#include <QDebug>
#include <QTimerEvent>
#include <QTimer>
#include <QEventLoop>
#include "mpvlibload.h"
#include "qthelper.h"
#include "mediaplaylist.h"
#include "UIControl/base/musicDataBase.h"
#include "UI/globalsignal.h"

#define MPV_REPLY_COMMAND 1
#define MPV_REPLY_PROPERTY 2
#define OMX_DEC_MAX_WIDTH 4096
#define OMX_DEC_MAX_HEIGHT 4096

class MpvCore : public QObject
{
    Q_OBJECT

    friend class unitMainWindow;

public:
    //          停止播放          播放中       暂停中
    enum State{StoppedState=0,PlayingState,PausedState}; //播放状态枚举
    enum ErrorMsg{NotFound=-1,Damage=-2}; //播放状态枚举
    MpvCore(QObject *parent = nullptr);

    void setPlaylist(MediaPlaylist *playlist); //设置播放列表
    void pause(); //暂停或继续
    void pauseOnly(); // 暂停

    State state() const; //获取状态
    qint64 position() const; //获取当前播放点
    void setPosition(qint64 pos); //设置播放起始点
#ifndef WHITE_TEST
    int AsyncCommand(const char *args[]); // 异步调用mpv命令
#endif
    int time2minutes(QString timeString); // 时间格式转毫秒数
    QString minutes2time(int minutesInt); // 秒数转时间格式

    bool isAvailable() const; //暂无实际功能
    void delayMsecond(unsigned int msec); // 延时函数，不阻塞线程
    void setVolume(int vol); //设置音量
    qint64 duration() const; //获取总时长
    void setMedia(const MMediaContent &media); //设置待播放媒体
    void play(); //播放
    void setPlayRate(double s);
    double getPlayRate();

protected:
    bool event(QEvent *event);

public Q_SLOTS:
    void stop(); //停止
    void initMpvHandle(); // 初始化mpvhandle

private:
    bool truePlay(QString startTime = "0"); //实际的播放函数
    void setProperty(const QString &name, const QString &value); // 设置mpv属性
    QString getProperty(const QString &name) const; // 获得mpv属性
    void changeState(State stateNow); //改变状态

    MediaPlaylist * m_playList = nullptr; //私有播放列表
    MediaPlaylist * m_tmpPlayList = nullptr; //私有临时播放列表
    mpv::qt::Handle m_mpvHandle;//句柄
    State m_state = StoppedState;//播放状态
    QByteArray filenameBack = ""; //上次播放的媒体名
    bool m_positionChangeed = false; //播放进度被设置
    qint64 m_position = 0; //播放进度
    qint64 m_duration = 0; //总时长
    QString m_mediaTitle; // 媒体标题

    bool m_isAlsa = false;

private Q_SLOTS:
    void autoPlay(MediaPlaylist::PlaybackMode playbackMode); //自动播放

Q_SIGNALS:
    void mpvEvents(); // 触发onMpvEvents()槽函数的信号
    void stateChanged(MpvCore::State); //状态改变信号
    void notifyDurationChanged(qint64); //切换媒体时，总时长改变信号
    void positionChanged(qint64); //播放进度改变信号
    void widpositionChanged(qint64 pos); //播放进度改变信号
    void playFinish(); //媒体播放完成信号
    void playError(); //媒体播放错误信号
    void playErrorMsg(ErrorMsg errorCode);//媒体播放错误信息信号
    void sigPlaySong();
//    void signalVolume(int);
};

#endif // MpvCore_H
