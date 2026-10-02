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

#include "mpvcore.h"

#include <QTimer>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QCoreApplication>
#include <QThread>
#include <ukui-log4qt.h>

const double EPS = 1e-6;
MpvCore::MpvCore(QObject *parent)
    : QObject(parent)
{
    initMpvHandle();
}

void MpvCore::setPlaylist(MediaPlaylist *playlist)
{
    //异常情况：空指针
    if (playlist == nullptr) {
        return;
    }

    m_playList = playlist;

    connect(this,&MpvCore::playFinish,m_playList,&MediaPlaylist::playFinish,Qt::UniqueConnection);
    connect(this,&MpvCore::playError,m_playList,&MediaPlaylist::playError,Qt::UniqueConnection);
    connect(m_playList,&MediaPlaylist::autoPlay,this,&MpvCore::autoPlay,Qt::UniqueConnection);
    connect(m_playList,&MediaPlaylist::stop,this,&MpvCore::stop,Qt::UniqueConnection);
    connect(this,&MpvCore::playErrorMsg,m_playList,&MediaPlaylist::playErrorMsg,Qt::UniqueConnection);
}

/**
 * @brief MpvCore::truePlay
 * @param startTime 开始播放时间，单位为秒
 *      0: 从头开始播放
 *      其他时间： 从拖动完成的位置开始播放
 */
bool MpvCore::truePlay(QString startTime)
{
    if (startTime.isEmpty()) {
        return false;
    }

    if (m_playList == nullptr) {
        return false;
    }
    qDebug() << "startTime: " << startTime;

    QString filePath = m_playList->getPlayFileName();
    qDebug() << "play filePath : " << filePath << QUrl(filePath).toLocalFile();;

    //异常情况：本地文件不存在
    if (!QFileInfo::exists(QUrl(filePath).toLocalFile())) {
        qDebug() << "当前播放文件不存在！" << filePath << QUrl(filePath).toLocalFile();
        Q_EMIT playErrorMsg(NotFound);
        Q_EMIT playError();
        return false;
    }


    qDebug() << "当前播放文件存在： " << filePath;
    const QByteArray c_filename = filePath.toUtf8();
    //如果文件名和上次一样，且不是因为拖动进度条播放，说明上次是暂停
    if (c_filename == filenameBack && !m_positionChangeed) {
        if (filenameBack != "") {
            //切换播放状态为播放
            pause();
        }

        return true;
    }


    musicDataStruct musicStruct;
    QString tmpFilepath = QUrl(filePath).toLocalFile();
    if (m_duration <= 0) {
        g_db->getSongInfoFromDB(tmpFilepath, musicStruct);
        QString durationString = musicStruct.time;
        qDebug() << "数据库总时长： " << durationString;
    }
    g_db->getSongInfoFromDB(filePath, musicStruct);
    QString durationString = musicStruct.time;
    qDebug() << "数据库总时长： " << durationString;

    int posDuration = time2minutes(durationString);
//    if (posDuration > m_duration) {
//        m_duration = posDuration;
//    }
    qDebug() << "总时长： " << m_duration << duration();

    if (!m_positionChangeed) {
        qInfo() << "new song";
        emit sigPlaySong();
    }
    //重置参数
    m_position = 0;
    m_positionChangeed = false;
    setProperty("start",startTime);
    setProperty("force-seekable", "yes");
    if (c_filename.endsWith(".ape")) {
        if (!m_isAlsa) {
            m_isAlsa = true;
            setProperty("ao", "alsa");
            qInfo() << "audio output is alsa";
        }
    } else if (m_isAlsa) {
        m_isAlsa = false;
        setProperty("ao", "pulse");
        qInfo() << "audio output is not alsa";
    }
    const char *args[] = {"loadfile",c_filename, NULL};
    MpvLibLoad::getInstance()->mpv_command_async(m_mpvHandle, 0, args);
//    const char *args[] = {"osd-msg", "seek", QString::number(posDuration).toStdString().c_str(), "absolute", NULL};
//    AsyncCommand(args);

    //如果不播放任何媒体，则切换状态为停止
    if (c_filename == "") {
        changeState(StoppedState);
        return true;
    }
    //记录到上次播放变量中
    filenameBack = c_filename;
    //切换播放状态为正在播放
    // todo: 检测当前状态，当为播放时，考虑不更改状态
    changeState(PlayingState);
    return true;
}

void MpvCore::play()
{
    Q_EMIT widpositionChanged(0);
    truePlay("0");
}

/**
 * @brief MpvCore::event  处理所有MPV事件，直到事件队列为空
 * @param event
 * @return
 */
bool MpvCore::event(QEvent *event)
{
    if(event->type() == QEvent::User)
    {
        while (m_mpvHandle)
        {
            mpv_event *event = MpvLibLoad::getInstance()->mpv_wait_event(m_mpvHandle, 0);
            if (event ==nullptr || event->event_id == MPV_EVENT_NONE) {
                break;
            }

            switch (event->event_id)
            {
            case MPV_EVENT_PROPERTY_CHANGE:
            {
                //属性改变事件
                mpv_event_property *prop = (mpv_event_property *)event->data;
                //播放时，时间改变事件
            //    printf("prop->name: %s\n", prop->name);
                if (strcmp(prop->name, "time-pos") == 0) {
                    if (prop->format == MPV_FORMAT_DOUBLE) {
                        //将播放状态设置为播放中
                        //qDebug() << "************* 播放进度更改时，状态为: " << m_state;
                        if (m_state == StoppedState) {
                            //qDebug() << "播放进度更改时，状态为: " << m_state;
                            changeState(PlayingState);
                        }
                        // 获得播放时间
                        double time = *(double *)prop->data;
                        //将单位换算为毫秒
                        m_position = time * 1000;
                        Q_EMIT positionChanged(m_position);
                    } else if (prop->format == MPV_FORMAT_NONE) {
                        //当前时长距离总时长不超过500毫秒判断播放结束
                        if ( m_duration!=0 && (m_duration - m_position < 500)) {
                            m_duration = 0;
                            m_position = 0;
                            //播放结束
                            Q_EMIT playFinish();
                        } else {
                            // 播放进度拖拉改变或者切歌时，是否更改当前状态？
//                            m_duration = 0;
//                            m_position = 0;
                            //qDebug() << "xxxxxxxxxxxxxxxxxx 播放进度更改时，状态为: " << m_state;
                            changeState(StoppedState);
                        }
                    }
                }

                if (QString(prop->name) == "duration")  {
                    printf("format: %d\n", prop->format);
                    // playback-time does the same thing as time-pos but works for streaming media
                    if(prop->format == MPV_FORMAT_DOUBLE) {
                        QString filepath = m_playList->getPlayFileName();
                        qDebug() << "play filePath : " << filepath << QUrl(filepath).toLocalFile();;

                        double tmp_duration = *(double*)prop->data * 1000;
                        qDebug() << "1 duration: " << m_duration << "tmp_duraion: " << tmp_duration;

                        musicDataStruct musicStruct;
                        filepath = QUrl(filepath).toLocalFile();
                        if (m_duration <= 0) {
                            // 对于本地flac格式音乐，获取的m_duration都是0，但tmp_duration都是正常
                            m_duration = tmp_duration;
                            g_db->getSongInfoFromDB(filepath, musicStruct);
                            QString durationString = musicStruct.time;
                            qDebug() << "数据库总时长： " << durationString;
                        }
                        g_db->getSongInfoFromDB(filepath, musicStruct);
                        QString durationString = musicStruct.time;
                        qDebug() << "数据库总时长： " << durationString;

                        int posDuration = time2minutes(durationString);

                        qDebug() << "2 duration: " << m_duration << "tmp_duraion: " << tmp_duration
                                 << "posDuration: " << posDuration
                                 << "musicStruct.time: " << musicStruct.time;

//                        if (tmp_duration.toin >= posDuration) {
                        if (tmp_duration > posDuration + EPS) {
                            m_duration = tmp_duration;

                            QString str_time = minutes2time(tmp_duration);

                            qDebug() << "更新总时长字段： " << str_time;
                            g_db->updateDuration(filepath, str_time);
//                            g_db->updateDuration(filepath, m_duration);
                            Q_EMIT notifyDurationChanged(m_duration);
                        } else {
                            // 对于ape格式本地音乐，需要从数据库中直接获取时长
                            m_duration = posDuration;
                            Q_EMIT notifyDurationChanged(m_duration);
                        }
                        qDebug() << "3 duration: " << m_duration << "tmp_duraion: " << tmp_duration; // 151301 => 02:31
                    }
                }
            }
                break;
//            case MPV_EVENT_PLAYBACK_RESTART:
            case MPV_EVENT_FILE_LOADED:
            {
                //初始化完成事件
                qDebug() << "[mpv event] MPV_EVENT_FILE_LOADED";

                //获取总时长
                QString m_durationMpv = MpvLibLoad::getInstance()->mpv_get_property_string(m_mpvHandle, "duration");//单位换算为毫秒
                qDebug() << "durationMpv: " << m_durationMpv;

                double tmpDuration = getProperty("duration").toDouble() *1000;//单位换算为毫秒
                m_mediaTitle = getProperty("media-title"); // 媒体标题
                QString filepath = m_playList->getPlayFileName();
                qDebug() << "play filePath : " << filepath << QUrl(filepath).toLocalFile();;
                qDebug() << "duration: " << duration()
                         << "media title: " << m_mediaTitle;

                musicDataStruct musicStruct;
                filepath = QUrl(filepath).toLocalFile();
                if (m_duration <= 0) {
                    g_db->getSongInfoFromDB(filepath, musicStruct);
                    QString durationString = musicStruct.time;
                    qDebug() << "数据库总时长： " << durationString;
                }
//                filepath = filepath.remove("file://");
                g_db->getSongInfoFromDB(filepath, musicStruct);
                QString durationString = musicStruct.time;
                qDebug() << "数据库总时长： " << durationString;

                int posDuration = time2minutes(durationString);
                if (tmpDuration > posDuration) {
                    qDebug() << "tmpDuration: " << tmpDuration << posDuration;
                    Q_EMIT notifyDurationChanged(m_duration);
                }
                    Q_EMIT notifyDurationChanged(m_duration);
            }
                break;
            case MPV_EVENT_IDLE:
            {
                //播放器空闲事件，只有刚启动时、播放完成时、歌曲异常时会进入此分支
                QString playlist = getProperty("playlist");
                //qDebug() << "playlist: " << playlist;
                if (!playlist.contains(',')) { //排除播放完成
                    if (playlist.length() > 2) { //排除刚启动
                        //歌曲播放异常
                        QString pasued = getProperty("pause");
                        if (pasued == "yes")
                        {
                            Q_EMIT playErrorMsg(Damage);
                        }
                        else
                        {
                            Q_EMIT playError();
                        }
                    }
                }
            }
                break;
            default:
                break;
            }
        }
        return false;
    }

    return QObject::event(event);
}

void MpvCore::setPlayRate(double s)
{
    setProperty("speed", QString::number(s));
    g_user_signal->sigrateChange(s);
}

double MpvCore::getPlayRate()
{
    return getProperty("speed").toDouble();
}

void MpvCore::pause()
{
    // 获得mpv播放器的"暂停"状态
    QString pasued = getProperty("pause");

    KyInfo() << "pauseState = " << pasued;

    // 根据"暂停"状态来选择暂停还是播放
    if(pasued == "no") {
        KyInfo() << "set pause yes";

        setProperty("pause", "yes");
        changeState(PausedState);
    } else if(pasued == "yes") {
        KyInfo() << "set pause no";

        setProperty("pause", "no");
        changeState(PlayingState);
    }
}

void MpvCore::pauseOnly()
{
    // 获得mpv播放器的"暂停"状态
    QString pasued = getProperty("pause");
    KyInfo() << "pauseStated = " << pasued;

    // 根据"暂停"状态来选择暂停还是播放
    if(pasued == "no") {
        KyInfo() << "begin pause.";
        setProperty("pause", "yes");
        changeState(PausedState);
    }
}

void MpvCore::stop()
{    
    filenameBack = "";
    setProperty("pause", "no");
    const char *args[] = {"stop", NULL};
    MpvLibLoad::getInstance()->mpv_command_async(m_mpvHandle, 0, args);
    changeState(StoppedState);    
}

MpvCore::State MpvCore::state() const
{
    return m_state;
}

qint64 MpvCore::position() const
{
    return m_position;
}

void MpvCore::setPosition(qint64 pos)
{
    if (pos < 0) {
        pos = 0;
    }
    Q_EMIT widpositionChanged(pos);
    double sec = double(pos)/1000;
    m_positionChangeed = true;

    //记录拖动进度条之前播放状态是否为暂停
    bool restartPlay = false;
    if (m_state == PausedState) {
        restartPlay = true;
    }

    if (!truePlay(QString::number(sec))){
        return;
    }

    if (restartPlay) {
        //切换播放状态为播放
        pause();
    }
}

#ifndef WHITE_TEST
int MpvCore::AsyncCommand(const char *args[])
{
    if (m_mpvHandle) {
        return MpvLibLoad::getInstance()->mpv_command_async(m_mpvHandle, MPV_REPLY_COMMAND, args);
    }
    return -1;
}
#endif
/**
 * @brief MpvCore::time2minutes 时间转毫秒数
 * @param timeString LocalMusic数据库表获取的时间格式
 * @return 毫秒数
 */
int MpvCore::time2minutes(QString timeString)
{
    if (timeString.isEmpty()) {
        return 0;
    }

    qDebug() << "timeString: " << timeString;
    QStringList s =timeString.split(":");

    int posDuration = 0;
    if(s.size() == 3) {
        posDuration = s.at(0).toInt() * 3600 + s.at(1).toInt() * 60 + s.at(2).toInt();
    } else {
        posDuration = s.at(0).toInt() * 60 + s.at(1).toInt();
    }
    return posDuration * 1000;
}

/**
 * @brief MpvCore::minutes2time 秒数转时间
 * @param minutesInt mpv获取的秒数
 * @return 时间格式字符串
 */
QString MpvCore::minutes2time(int minutesInt)
{
    int pos = minutesInt / 1000;
    int hour = static_cast<int>(pos / 3600);
    int minutes = static_cast<int>(pos % 3600 / 60);
    int seconds = static_cast<int>(pos % 60);

    QTime duration(hour, minutes, seconds);

    QString str_time;
    if(hour > 0) {
        str_time = duration.toString("hh:mm:ss");
    } else {
        str_time = duration.toString("mm:ss");
    }

    return str_time;
}

/**
 * @brief MpvCore::setVolume 设置音量
 *
 * 设置音量，此音量和系统同步，不单独设置mpv音量
 *
 * @param vol 用户在音量滑动条设置的音量
 */
void MpvCore::setVolume(int vol)
{
    QDBusMessage message = QDBusMessage::createSignal("/", "org.kylin.music", "sinkInputVolumeChanged");
    message << "kylin-music" << vol << false;

    KyInfo() << "createSignal: volume = " << vol;
    QDBusConnection::sessionBus().send(message);
}

qint64 MpvCore::duration() const
{
    return m_duration;
}

void MpvCore::setMedia(const MMediaContent &media)
{
    QUrl url =media.canonicalUrl();

    //防止内存泄漏
    if (m_tmpPlayList != nullptr) {
        m_tmpPlayList->deleteLater();
    }

    //创建新的播放列表并将歌曲录入
    m_tmpPlayList = new MediaPlaylist(this);
    m_tmpPlayList->setObjectName("kylin-music_mpvcore_MediaPlaylist_m_tmpPlayList");
    m_tmpPlayList->addMedia(url);
    setPlaylist(m_tmpPlayList);

    //以暂停状态从头开始播放
    setProperty("pause", "yes");
    play();
}

bool MpvCore::isAvailable() const
{
    return true;
}

void MpvCore::delayMsecond(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

/**
 * @brief wakeup 回调函数
 *
 * 此回调可从任何mpv线程调用（但也可以从调用mpv API的线程递归地返回）,
 * 只是需要通知要唤醒的Qt GUI线程（以便它可以使用mpv_wait_event（）），并尽快返回
 *
 * @param ctx
 */
static void wakeup(void *ctx)
{
    MpvCore *mpvHandler = (MpvCore *)ctx;
    QCoreApplication::postEvent(mpvHandler, new QEvent(QEvent::User));
}

/**
 * @brief MpvCore::initMpvHandle 创建mpv实例
 */
void MpvCore::initMpvHandle()
{
    setlocale(LC_NUMERIC,"C");
    m_mpvHandle = mpv::qt::Handle::FromRawHandle(MpvLibLoad::getInstance()->mpv_create());

    if (m_mpvHandle == nullptr) {
        qDebug()<<"创建播放模块失败！";
        this->deleteLater();
        return;
    }

    // 判断mpv实例是否成功初始化
    if (MpvLibLoad::getInstance()->mpv_initialize(m_mpvHandle) < 0) {
        qDebug()<<"初始化失败！";
        this->deleteLater();
    }

    //禁用视频流
    setProperty("vid", "no");
    MpvLibLoad::getInstance()->mpv_set_option_string(m_mpvHandle, "audio-client-name", "kylin-music");

    //绑定event事件,如监听"duration"音频时长事件
    MpvLibLoad::getInstance()->mpv_observe_property(m_mpvHandle, 0, "time-pos", MPV_FORMAT_DOUBLE);
    MpvLibLoad::getInstance()->mpv_observe_property(m_mpvHandle, 0, "duration", MPV_FORMAT_DOUBLE);

    //接收事件
    MpvLibLoad::getInstance()->mpv_set_wakeup_callback(m_mpvHandle, wakeup, this);
}

/**
 * @brief MpvCore::setProperty 设置mpv属性
 * @param name 设置的mpv属性名称
 * @param value 设置的mpv属性值
 */
void MpvCore::setProperty(const QString &name, const QString &value)
{
    MpvLibLoad::getInstance()->mpv_set_option_string(m_mpvHandle, name.toLatin1().data(), value.toLatin1().data());
}

/**
 * @brief MpvCore::getProperty 设置mpv属性
 * @param name 获取的mpv属性名称
 */
QString MpvCore::getProperty(const QString &name) const
{
    return (QString)MpvLibLoad::getInstance()->mpv_get_property_string(m_mpvHandle, name.toLatin1().data());
}

void MpvCore::changeState(MpvCore::State stateNow)
{
    qDebug() << "============= m_state: " << m_state << "stateNow: " << stateNow;
    //待设置的循环模式和设置之前一致则不处理
    if (m_state == stateNow ) {
        return;
    }
    qDebug() << "============== stateChanged m_state: " << m_state << "stateNow: " << stateNow;

    m_state = stateNow;

    Q_EMIT stateChanged(m_state);
}

void MpvCore::autoPlay(MediaPlaylist::PlaybackMode playbackMode)
{
    //如果是单曲循环模式
    if (playbackMode == MediaPlaylist::PlaybackMode::CurrentItemInLoop) {
        //播放完毕自动切歌（借用播放点改变时间逻辑循环）
        m_positionChangeed = true;
    }
    Q_EMIT widpositionChanged(0);
    truePlay("0");
}
