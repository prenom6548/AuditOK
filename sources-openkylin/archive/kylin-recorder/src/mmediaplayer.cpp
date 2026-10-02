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
 */
#include "mmediaplayer.h"
#include "mpvlib/qthelper.h"

MMediaPlayer::MMediaPlayer(QObject *parent) : QObject(parent)
{
    createMvpplayer();
    connect(this, &MMediaPlayer::playFinish, this, &MMediaPlayer::stop, Qt::UniqueConnection);
}

MMediaPlayer::~MMediaPlayer()
{
    if (m_mpvPlayer) {
        mpv::qt::command_variant(m_mpvPlayer, QStringList() << "stop");
        MpvLibLoad::getInstance()->mpv_terminate_destroy(m_mpvPlayer);
        m_mpvPlayer = nullptr;
    }
}

void MMediaPlayer::truePlay(QString startTime)
{
    qInfo() << "truePlay:" << startTime;
    //异常情况：入参不合法
    if (startTime.isEmpty()) {
        return;
    }
    m_positionChangeed = false;
    m_position = 0;
    qInfo() << "truePlay: " << startTime;
    setProperty("start", startTime);

    changeState(PlayingState);
}

void MMediaPlayer::play(qint64 pos)
{
    qInfo() << "play: " << QString::number(pos);
    double postmp = (double)pos / 1000;
    truePlay(QString::number(postmp));
}

void MMediaPlayer::playPause(qint64 pos)
{
    qInfo() << "need restart play m_state:" << m_state;

    // 获得mpv播放器的"暂停"状态
    QString pasued = getProperty("pause");
    // 根据"暂停"状态来选择暂停还是播放
    if (pasued == "no") {
        setProperty("pause", "yes");
        changeState(PausedState);
    } else if (pasued == "yes") {
        setProperty("pause", "no");
        changeState(PlayingState);
    }
}

void MMediaPlayer::stop()
{
    filenameBack = "";


    mpv::qt::command_variant(m_mpvPlayer, QStringList() << "stop");
    qInfo() << "stop";

    changeState(StoppedState);
}

MMediaPlayer::State MMediaPlayer::state() const
{
    return m_state;
}

qint64 MMediaPlayer::position() const
{
    return m_position;
}

void MMediaPlayer::setPosition(qint64 pos)
{
    double sec = (double)pos / 1000;
    m_positionChangeed = true;
    qInfo() << "setPosition:" << sec;
//    truePlay(QString::number(sec));
    const char *args[] = {"seek", QString::number(sec).toStdString().c_str(), "absolute", NULL};;
    MpvLibLoad::getInstance()->mpv_command_async(m_mpvPlayer, 0, args);
}

void MMediaPlayer::setVolume(int vol)
{
    setProperty("volume", QString::number(vol));
}

qint64 MMediaPlayer::duration() const
{
    return m_duration;
}

void MMediaPlayer::setMedia(QString media)
{
    m_duration = 0;
    m_position = 0;


    //结合录音实际将下列7行代码加上尤为重要，防止了音乐切换概率性需要再点击才能播放的问题
    QByteArray c_filename = media.toUtf8();
    if (c_filename == "") {
        changeState(StoppedState);
        return;
    }

    m_lastFileName = media;
    setProperty("pause", "no");
    const char *args[] = {"loadfile", c_filename, NULL};
    MpvLibLoad::getInstance()->mpv_command_async(m_mpvPlayer, 0, args);

    changeState(PlayingState);

}

bool MMediaPlayer::isAvailable() const
{
    return true;
}


void MMediaPlayer::onMpvEvents()
{
    // 处理所有事件，直到事件队列为空
    while (m_mpvPlayer) {
        mpv_event *event = MpvLibLoad::getInstance()->mpv_wait_event(m_mpvPlayer, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;
        handle_mpv_event(event);
    }
}

void MMediaPlayer::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
    // 属性改变事件发生
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
               if (m_state == StoppedState) {
                   changeState(PlayingState);
               }
                // 获得播放时间
                double time = *(double *)prop->data;
                m_position = time * 1000; //单位换算为毫秒
//                qInfo() << "mpv pos:" << m_position;
//                emit positionChanged(m_position);
            } else if (prop->format == MPV_FORMAT_NONE) {
                //当前时长距离总时长不超过500毫秒判断播放结束
                if (m_duration != 0 && (m_duration - m_position < 500)) {
                    m_duration = 0;
                    m_position = 0;
                    //播放结束
                    qInfo() << "播放结束";

                    emit playFinish();
                }
            }
        }
    } break;
    case MPV_EVENT_PLAYBACK_RESTART: {
        m_duration = getProperty("duration").toDouble() * 1000; //单位换算为毫秒
        qInfo() << "get duration:" << m_duration;
        emit durationChanged(m_duration);
        if (m_duration == 0) {
            qInfo() << "file:" << m_lastFileName << ", duration is 0!";
            emit playFinish();
        }
        //初始化完成事件
    } break;

    default:;
    }
}

// 回调函数
static void wakeup(void *ctx)
{
    // 此回调可从任何mpv线程调用（但也可以从调用mpv API的线程递归地返回）
    // 只是需要通知要唤醒的Qt GUI线程（以便它可以使用mpv_wait_event（）），并尽快返回
    MMediaPlayer *mvpPlayer = (MMediaPlayer *)ctx;
    emit mvpPlayer->mpvEvents();
}

void MMediaPlayer::createMvpplayer()
{
    // 创建mpv实例
    setlocale(LC_NUMERIC, "C");
    m_mpvPlayer = MpvLibLoad::getInstance()->mpv_create();
    if (m_mpvPlayer == nullptr) {
        qDebug() << "创建播放模块失败！";
        this->deleteLater();
        return;
    }
    //禁用视频流
    setProperty("vid", "no");
    //接收事件
    connect(this, &MMediaPlayer::mpvEvents, this, &MMediaPlayer::onMpvEvents, Qt::QueuedConnection);
    MpvLibLoad::getInstance()->mpv_set_wakeup_callback(m_mpvPlayer, wakeup, this);
    //绑定事件
    MpvLibLoad::getInstance()->mpv_observe_property(m_mpvPlayer, 0, "time-pos", MPV_FORMAT_DOUBLE);
    // 判断mpv实例是否成功初始化
    if (MpvLibLoad::getInstance()->mpv_initialize(m_mpvPlayer) < 0) {
        qDebug() << "初始化失败！";
        this->deleteLater();
    }
}

void MMediaPlayer::setProperty(const QString &name, const QString &value)
{
    MpvLibLoad::getInstance()->mpv_set_option_string(m_mpvPlayer, name.toLatin1().data(), value.toLatin1().data());
}

QString MMediaPlayer::getProperty(const QString &name) const
{
    return (QString)MpvLibLoad::getInstance()->mpv_get_property_string(m_mpvPlayer, name.toLatin1().data());
}

void MMediaPlayer::changeState(MMediaPlayer::State stateNow)
{
    m_state = stateNow;
    QMediaPlayer::State tmp;
    if (m_state == MMediaPlayer::State::StoppedState){
        tmp = QMediaPlayer::State::StoppedState;
        m_position = 0;
    } else if (m_state == MMediaPlayer::State::PlayingState){
        tmp = QMediaPlayer::State::PlayingState;
    } else if (m_state == MMediaPlayer::State::PausedState){
        tmp = QMediaPlayer::State::PausedState;
    }
    emit stateChanged(tmp);
}
