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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "dbusadapter.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <cmath>
#include "global/globalsignal.h"
#include "global/global.h"

using namespace Global;

DbusAdapter::DbusAdapter(QObject *parent) : QObject(parent)
{
    m_currentFile = "";

    connect(g_core_signal, &GlobalCoreSignal::sigStateChange, [this](){
        if(g_playstate < 0)
            m_currentFile = "";
    });
    connect(g_core_signal, &GlobalCoreSignal::sigFileInfoChange, this, [this](Mpv::FileInfo fi){
        m_currentFile = fi.file_path;
    });

    // 播放状态的改变通过检测sigStateChange来实现
    connect(g_core_signal, &GlobalCoreSignal::sigStateChange, [this]() {
            QString status = "Stopped";
            switch (g_playstate) {
            case Mpv::Playing:
                status = "Playing";
                break;
            case Mpv::Paused:
                status = "Paused";
                break;
            case Mpv::Stopped:
                status = "Stopped";
                break;
            default:
                break;
            }
            notify("org.mpris.MediaPlayer2.Player", "PlaybackStatus", status);

        });
    connect(g_core_signal, &GlobalCoreSignal::sigVolumeChange, [&](int volume){
        QVariantMap changedProperties;
        changedProperties.insert("Volume",QVariant::fromValue(volume));
        notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
    });
    QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.KylinVideo");
    QDBusConnection::sessionBus().registerService("org.mpris.MediaPlayer2.KylinVideo");
    QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", this, QDBusConnection::ExportAllContents);
    QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", this, QDBusConnection::ExportAllContents);
    initMetadata();
    connect(g_core_signal, &GlobalCoreSignal::sigCurrentTime, [&](double time){
        //获取到的时间单位是秒，转换为微秒
        m_position = time * 1000000;
    });

    connect(g_user_signal, &GlobalUserSignal::sigSpeed, [&](double rate){
        QVariantMap changedProperties;
        changedProperties.insert("Rate",QVariant::fromValue(rate));
        notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
    });

    connect(g_user_signal, &GlobalUserSignal::sigPlayOrder, [&](PlayOrder order){
        m_playOrder = order;
        QString playorder = "Track";
        switch (order) {
        case ONE_LOOP:
            playorder = "Track";
            break;
        case LIST_LOOP:
            playorder = "Playlist";
            break;
        default:
            playorder = "None";
            break;
        }
        if(playorder == "Playlist" || playorder == "Track"){
            //LoopStatus属性发生变化，发出PropertiesChanged信号
            QVariantMap changedProperties;
            changedProperties.insert("LoopStatus",QVariant::fromValue(playorder));
            notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
        }
        if(order == RANDOM){
            QVariantMap changedProperties;
            QString input = "true";
            changedProperties.insert("Shuffle",QVariant::fromValue(input));
            notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
        }else{
            QVariantMap changedProperties;
            QString input = "false";
            changedProperties.insert("Shuffle",QVariant::fromValue(input));
            notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
        }
    });
}

void DbusAdapter::initMetadata()
{
    // 表示路径
    m_metadata.insert("mpris:trackid", 0);
    m_metadata.insert("mpris:length", 0);
    m_metadata.insert("mpris:artUrl", "");
    m_metadata.insert("xesam:album", "");
    m_metadata.insert("xesam:albumArtist", "");
    m_metadata.insert("xesam:artist", "");
    m_metadata.insert("xesam:asText", "");
    m_metadata.insert("xesam:audioBPM", 0);
    m_metadata.insert("xesam:autoRating", 0.0);
    QStringList commont;
    m_metadata.insert("xesam:comment", commont);
    QStringList composer;
    m_metadata.insert("xesam:composer", composer);
    m_metadata.insert("xesam:contentCreated", "");
    m_metadata.insert("xesam:discNumber", 0);
    m_metadata.insert("xesam:firstUsed", "");
    QStringList genre;
    m_metadata.insert("xesam:genre", genre);
    m_metadata.insert("xesam:lastUsed", "");
    QStringList lyricist;
    m_metadata.insert("xesam:lyricist", lyricist);
    m_metadata.insert("xesam:title", "");
    m_metadata.insert("xesam:trackNumber", 0);
    m_metadata.insert("xesam:url", "");
    m_metadata.insert("xesam:useCount", 0);
    m_metadata.insert("xesam:userRating", 0.0);
}

void DbusAdapter::notify(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    QString objectPath = "/org/mpris/MediaPlayer2";
    QDBusMessage signalMessage= QDBusMessage::createSignal(objectPath,"org.freedesktop.DBus.Properties","PropertiesChanged");
    signalMessage << interfaceName << changedProperties << invalidatedProperties;
    connection.send(signalMessage);
}


/**
 * @brief Stop - 完全停止当前的播放，并将播放位置重置为曲目开头。
 *
 * @note
 * - 若播放器处于播放（Playing）或暂停（Paused）状态，调用此方法会停止播放，并将状态设置为停止（Stopped）。
 * - 若播放器已经处于停止（Stopped）状态，调用此方法不会产生任何效果。
 * - 调用'Play'方法后，播放器会从头开始播放右侧播放列表中的第一首曲目。
 * - 停止播放后，会发送信号以显示默认界面。
 */
void DbusAdapter::Stop()
{
    if(g_playstate == Mpv::Playing || g_playstate == Mpv::Paused){
        isStopped = true;
        g_user_signal->stop();
        // 手动停止之后需要显示默认界面
        g_user_signal->showStopFrame();
    }else{
        return;
    }
}

/**
 * @brief Next - 跳转到播放列表中的下一首曲目并播放。
 *
 * @note
 * - 若当前的播放状态为 Playing 或 Paused，且存在下一首曲目，则跳转播放下一首。
 * - 若当前的播放状态为 Playing 或 Paused，且不存在下一首曲目，则从头播放当前曲目。
 * - 若当前的播放状态为 Stopped，则调用此方法无效果。
 */
void DbusAdapter::Next() const
{
    if(g_playstate == Mpv::Playing || g_playstate == Mpv::Paused)
        g_user_signal->playNext(true);
    else
        return;
}

/**
 * @brief SetPosition - 将当前曲目的播放位置设置为指定的微秒数。
 *
 * @param TrackId 轨道标识符。当此参数为空时，表示当前播放的曲目。
 * @param Position 要设置的新位置，单位为微秒。
 *
 * @note
 * - 当`TrackId`为空时，此函数会尝试设置当前播放曲目的位置。
 * - 如果当前播放状态为`Stopped`，调用此函数不会产生任何效果。
 * - 如果`Position`小于0或大于当前轨道的长度，调用此函数同样不会产生任何效果。
 */
void DbusAdapter::SetPosition(const QString &TrackId, const qint64 Position)
{
    if(g_playstate == Mpv::Playing || g_playstate == Mpv::Paused){
        if(TrackId.isEmpty()){
            if(Position >= 0){
                g_user_signal->sigSeek(Position / 1000000);
            }else{
                qWarning() << "dbus SetPosition error, Position is :" << Position;
                return;
                }
        } //后期可以增加判断条件，若TrackID与当前曲目的TrackID一致，则调用生效；否则调用无效
    }else
        return;
}


/**
 * @brief OpenUri - 打开并播放由URI指定的媒体文件。
 *
 * @param Uri 要打开的媒体文件的资源标识符，可以是文件路径或URL。
 *
 * @note
 * - 如果`Uri`是一个空字符串，则函数不会执行任何操作并立即返回。
 * - 如果`Uri`不是以"http"开头的字符串，则认为它是一个文件路径，并将其添加到播放列表中。
 * - 如果`Uri`是以"http"开头的字符串，则认为它是一个URL，并发送信号以播放该URL指定的媒体文件。
 */
void DbusAdapter::OpenUri(const QString Uri)
{
    QStringList uriList;
    if(Uri == "")
        return;
    else{
        if (!Uri.startsWith("http")){
            uriList.append(Uri);
            g_user_signal->addFiles(uriList);
        }else{
        g_user_signal->sigDbusUrl(Uri);
        }
    }
}

bool DbusAdapter::getShuffle()
{
    switch (m_playOrder) {
    case RANDOM:
        return true;
    default:
        return false;
    }
}

void DbusAdapter::setShuffle(bool b)
{
    if(b && m_playOrder != PlayOrder::RANDOM){
        g_user_signal->setPlayOrder(RANDOM);
    }else if(!b && m_playOrder != PlayOrder::LIST_LOOP){
        //顺序播放设置为列表循环
        g_user_signal->setPlayOrder(LIST_LOOP);
    }else{
        return;
    }

}

double DbusAdapter::getRate()
{
    return g_settings->value("General/speed").toDouble();
}

void DbusAdapter::setRate(double d)
{
    qDebug() << "setRate:" << d;
    if(g_playstate != Mpv::Playing && g_playstate != Mpv::Paused)
        return;
    else{
        // 当状态为 Playing 或 Paused时，set 方法可生效
        if(d == 0.5 || d == 0.75 || d == 1.0 || d == 1.25 || d == 1.5 || d == 2.0){
            if(g_settings->value("General/speed").toDouble() == d){
                return;
            }else{
                if (d >= m_minimumRate && d <= m_maximumRate) {
                    g_user_signal->setSpeed(d);
                } else {
                    qWarning() << "dbus set rate error, rate is :" << d;
                }
            }
        }else
            return;
    }
 }

QString DbusAdapter::getLoopStatus()
{
    switch (m_playOrder) {
    case ONE_LOOP:
        return "Track";
    case LIST_LOOP:
        return "Playlist ";
    default:
        return "Playlist";
    }
}

/**
 * @brief Seek - 用于在当前播放的曲目中根据指定的微秒数进行快进或快退操作。
 *
 * @param Offset 快进或快退的时间，单位是微秒。正值表示快进，负值表示快退。
 *
 * @note
 * - 如果快退的值会使播放位置超出曲目开始位置，则将播放位置设置为 0（曲目起点）。
 * - 如果快进的值会使播放位置超出曲目结束位置，则效果等同于调用 `Next` 方法。
 * - 当前播放状态为 Stopped 时，调用此方法不会产生任何效果。
 * - 如果 `Offset` 的绝对值小于 0.1 秒，调用此方法同样不会产生任何效果。
 */
void DbusAdapter::Seek(const qint64 Offset)
{
    //只有在Playing、Paused状态下会对当前的曲目产生影响
    if(g_playstate == Mpv::Playing || g_playstate == Mpv::Paused){
        //若Offset小于0.1s，不会产生任何效果
        if (qAbs(Offset) >= 1000000){
            g_user_signal->sigDbusSeek(Offset);
        }else
            return;
    }else
        return;
}

void DbusAdapter::setVolume(double d)
{
    if(g_playstate != Mpv::Playing && g_playstate != Mpv::Paused)
        return;
    else{
        if(double(g_settings->value("General/volume").toInt()) / 100 == d){
            return;
        }else{
            //传入负值时将声音设置为0
            int volume = 0;
            if(d <= 0){
                volume = 0;
            }else if (d > 1) {
                qWarning() << "setVolume error, bigger than 1, volume is :" << d;
                return;
            }else{
                volume = round(d * 100);
            }
            //发出设置声音的信号
            g_user_signal->sigVolume(volume);
        }
    }
}


void DbusAdapter::setLoopStatus(QString s)
{
    // 当状态为 Playing 或 Paused时，set 方法可生
    if(s == "Track" && m_playOrder != ONE_LOOP)
        g_user_signal->setPlayOrder(ONE_LOOP);
    else if(s == "Playlist" && m_playOrder != LIST_LOOP)
        g_user_signal->setPlayOrder(LIST_LOOP);
    else
        return;
}

double DbusAdapter::getVolume()
{
    return double(g_settings->value("General/volume").toInt()) / 100;
}


long long DbusAdapter::getPosition()
{
    return m_position;
}

void DbusAdapter::KvPlayPause() const
{
    if(g_playstate == Mpv::Paused)
        g_user_signal->play();
    else if(g_playstate == Mpv::Playing)
        g_user_signal->pause();
    else if (g_playstate == Mpv::Stopped)
        g_user_signal->stop();
}

/**
 * @brief Play - 启动或恢复播放。
 *
 * @note
 * - 当前没有播放且存在可播放的曲目时，调用此方法会播放右侧播放列表中第一首曲目。
 * - 如果播放已暂停（Paused），调用此方法会从当前暂停的位置恢复播放。
 * - 如果已经在播放（Playing），调用此方法不会产生任何效果。
 * - 若处于停止（Stopped）状态，调用此方法会从头播放右侧播放列表中第一首曲目。
 * - 如果没有可播放的曲目，调用此方法不会产生任何效果。
 */
void DbusAdapter::Play()
{
    if(g_playstate == Mpv::Playing)
            return;
    if(isStopped){
        g_user_signal->playFirst();
        isStopped = false;
    }
    else
        g_user_signal->play();
}


/**
 * @brief Pause - 暂停播放当前曲目。
 *
 * @note
 * - 若处于播放（Playing）状态，调用此方法会暂停播放。
 * - 若处于暂停（Paused）状态，调用此方法无效果。
 * - 调用 Play 方法后，播放将从暂停的位置继续进行。
 */
void DbusAdapter::Pause()
{
    if(g_playstate == Mpv::Playing)
        g_user_signal->pause();
    else
        return;
}


/**
 * @brief PlayPause - 在暂停和播放之间切换。
 *
 * @note
 * - 若正在播放（Playing）状态，则暂停播放。
 * - 若处于暂停（Paused）状态，则开始播放。
 * - 若处于停止（Stopped）状态，且存在可播放曲目，调用此方法会播放右侧播放列表中第一个视频。
 */
void DbusAdapter::PlayPause()
{
    if(g_playstate == Mpv::Paused){
        g_user_signal->play();
    }
    else if(g_playstate == Mpv::Playing){
        g_user_signal->pause();
    }
    else{
        if(isStopped){
            g_user_signal->playFirst();
            isStopped = false;
        }
        else
            return;
    }
}

void DbusAdapter::notify(const QString &interfaceName, const QString &key, const QString &value)
{
    QVariantMap changedProperties;
    changedProperties.insert(key, QVariant::fromValue(value));
    QStringList invalidatedProperties;
    QDBusConnection connection = QDBusConnection::sessionBus();
    QString objectPath = "/org/mpris/MediaPlayer2";
    QDBusMessage signalMessage= QDBusMessage::createSignal(objectPath,"org.freedesktop.DBus.Properties","PropertiesChanged");
    signalMessage << interfaceName << changedProperties << invalidatedProperties;
    connection.send(signalMessage);
}

/**
 * @brief Previous - 跳转到播放列表中的上一首曲目并播放。
 *
 * @note
 * - 若当前的播放状态为 Playing 或 Paused，且存在上一首曲目，则跳转播放上一首。
 * - 若当前的播放状态为 Playing 或 Paused，且不存在上一首曲目，则从头播放当前曲目。
 * - 若当前的播放状态为 Stopped，则调用此方法无效果。
 */
void DbusAdapter::Previous() const
{
    if(g_playstate == Mpv::Playing || g_playstate == Mpv::Paused)
        g_user_signal->playPrev(true);
    else
        return;
}

void DbusAdapter::AddFile(QString file) const
{
    g_user_signal->addFiles(QStringList() << file);
}

void DbusAdapter::VolumeUp() const
{
    g_user_signal->setVolumeUp(10);
}

void DbusAdapter::VolumeDown() const
{
    g_user_signal->setVolumeDown(10);
}

void DbusAdapter::FullScreen() const
{
    g_user_signal->fullScreen();
}

void DbusAdapter::LoopMode() const
{
    g_user_signal->changePlayOrder();
}

void DbusAdapter::Exit() const
{
    g_user_signal->exitApp();
}

/**
 * 返回播放状态
 * @return:
 *  -1  :停止
 *  0   :暂停
 *  1   :播放
 */
int DbusAdapter::PlayState()
{
    if(g_playstate == Mpv::Playing)
        return 1;
    else if(g_playstate == Mpv::Paused)
        return 0;
    else
        return -1;
}

/**
 * @brief DbusAdapter::getState 当前音视频播放状态
 *
 * @note 当系统重启后任务栏也不生效时，需要 `killall ukui-panel` 重启任务栏验证
 *
 * @return
 *      "Playing" ： 播放
 *      "Paused" ： 暂停
 *      "Stopped" ： 停止
 */
QString DbusAdapter::getPlaybackState()
{
    if (g_playstate == Mpv::Playing) {
        return PLAY_STATUS;
    } else if (g_playstate == Mpv::Paused) {
        return PAUSED_STATUS;
    } else {
        return STOP_STATUS;
    }
}

QString DbusAdapter::CurrentFile()
{
    return m_currentFile;
}
