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

#include <QStringList>
#include "dbusadapter.h"
#include <string>
#include <string.h>
#include <ukui-log4qt.h>
DbusAdapter::DbusAdapter(QObject *parent) : QObject(parent)
{
    QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.KylinMusic");
    QDBusConnection::sessionBus().registerService("org.mpris.MediaPlayer2.KylinMusic");
//    QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", this, QDBusConnection::ExportNonScriptableSlots | QDBusConnection::ExportScriptableProperties);
    QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", this, QDBusConnection::ExportAllContents);
//    QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", this, QDBusConnection::ExportAllContents);
//    if(QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", this, QDBusConnection::ExportAllContents))
//       {
//            qDebug()<<"成功注册对象org.freedesktop.DBus.Properties";
//       }
    connect(g_user_signal,  &GlobalUserSignal::sigStatuChange, this, [&](QString status){
        QVariantMap changedProperties;
        changedProperties.insert("PlaybackStatus",QVariant::fromValue(status));
        notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
    });
    connect(g_user_signal, &GlobalUserSignal::sigLoopStatusChange, this, [&](QString loopStatus){
        if(loopStatus == "Track"){
            //LoopStatus属性发生变化，发出PropertiesChanged信号
            QVariantMap changedProperties;
            changedProperties.insert("LoopStatus",QVariant::fromValue(loopStatus));
            notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
        }else if(loopStatus == "Random"){
            QVariantMap changedProperties;
            QString input = "true";
            changedProperties.insert("Shuffle",QVariant::fromValue(input));
            notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
        }else if(loopStatus == "Playlist"){
            QVariantMap changedProperties;
            QString input = "false";
            changedProperties.insert("Shuffle",QVariant::fromValue(input));
            notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
            changedProperties.insert("LoopStatus",QVariant::fromValue(loopStatus));
            notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
        }
    });
    connect(g_user_signal, &GlobalUserSignal::sigrateChange, this, [&](double rate){
        QVariantMap changedProperties;
        changedProperties.insert("Rate",QVariant::fromValue(rate));
        notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
    });
    connect(g_user_signal, &GlobalUserSignal::sigVolumeChange, this, [&](int volume){
        QVariantMap changedProperties;
        changedProperties.insert("Volume",QVariant::fromValue(volume));
        notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
    });
}

QString DbusAdapter::getLoopStatus()
{
    QString loopStatus;
    loopStatus = Widget::mutual->getPlayMode();
    if(loopStatus == ""){
        loopStatus = "Playlist";
    }
    return loopStatus;
}

void DbusAdapter::setLoopStatus(QString s)
{
    if(s != "Track" && s != "Playlist" ){
        return;
    }else{
        if(Widget::mutual->getPlayMode() != s){
            Widget::mutual->setPlayMode(s);
        }else{
            return;
        }
    }
}

QVariantMap DbusAdapter::getMetadata()
{
    return Widget::mutual->getMetadata();
}

void DbusAdapter::updateMetadata(const QVariantMap input)
{
    qInfo() << input;
    QVariantMap changedProperties;
    changedProperties.insert("Metadata",QVariant::fromValue(input));
    notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
}

double DbusAdapter::getVolume()
{
    return double(Widget::mutual->getVolume()) / 100;
}

// mpris中volume的取值范围是[0,1]，但软件中取值范围是[0, 100]
void DbusAdapter::setVolume(double d)
{
    if(Widget::mutual->getStates() == STOP_STATUS)
        return;
    else{
        if(Widget::mutual->getVolume() == round(d * 100)){
            return;
        }else{
            int volume = 0;
            if (d < 0) {
                volume = 0;
            } else if (d > 1) {
                qWarning() << "setVolume error, bigger than 1, volume is :" << d;
                return;
            } else{
                //四舍五入
                volume = round(d * 100);
            }
            Widget::mutual->setVolume(volume);
            //调节音量条
            Widget::mutual->volumeChange(volume);
        }
    }
}

long long DbusAdapter::getPosition()
{
    return Widget::mutual->getPosition() * 1000;
}

/**
 * @brief Stop - 完全停止当前的播放，并将播放位置重置为曲目开头。
 *
 * @note
 * - 若播放器处于播放（Playing）或暂停（Paused）状态，调用此方法会停止播放，并将状态设置为停止（Stopped）。
 * - 若播放器已经处于停止（Stopped）状态，调用此方法不会产生任何效果。
 * - 调用'Play'方法后，播放器会从头开始播放播放列表中的第一首曲目。
 * - 停止播放后，会发送信号以显示默认界面。
 */
void DbusAdapter::Stop() const
{
    KyInfo() << "Stop";
    if(Widget::mutual->getStates() == PLAY_STATUS || Widget::mutual->getStates() == PAUSED_STATUS){
        g_user_signal->initPlaySongArea();
        Widget::mutual->Stop();
    }
    else
        return;
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
    KyInfo() << "Next begin";
    if(Widget::mutual->getStates() == STOP_STATUS){
        KyInfo() << "Statues is STOP_STATUS";
        return;
    }

    QString playPath = playController::getInstance().getPath();
    if(playPath != ""){
        KyInfo() << "Current PlayPath:" <<playPath;
        Widget::mutual->Next();
    }else{
        KyInfo() << "playPath is null";
    }
    KyInfo() << "Next end";
}

/**
 * @brief Play - 启动或恢复播放。
 *
 * @note
 * - 当前没有播放且存在可播放的曲目时，调用此方法会播放右侧播放列表中第一首曲目。
 * - 如果播放已暂停（Paused），调用此方法会从当前暂停的位置恢复播放。
 * - 如果已经在播放（Playing），调用此方法不会产生任何效果。
 * - 若处于停止（Stopped）状态，调用此方法会从头播放播放列表中第一首曲目。
 * - 如果没有可播放的曲目，调用此方法不会产生任何效果。
 */
void DbusAdapter::Play() const
{
    KyInfo() << "Play";
    if(Widget::mutual->getStates() == PLAY_STATUS)
        return;
    //若当前状态为Stopped，则播放播放列表中第一首曲目
    else if(Widget::mutual->getStates() == STOP_STATUS){
        g_user_signal->sigPlayFirst();
    }
    //若当前状态为Paused，则播放
    else if(Widget::mutual->getStates() == PAUSED_STATUS)
        Widget::mutual->Play();
}

/**
 * @brief Pause - 暂停播放当前曲目。
 *
 * @note
 * - 若处于播放（Playing）状态，调用此方法会暂停播放。
 * - 若处于暂停（Paused）状态，调用此方法无效果。
 * - 调用 Play 方法后，播放将从暂停的位置继续进行。
 */
void DbusAdapter::Pause() const
{
    KyInfo() << "Pause";
    if(Widget::mutual->getStates() == PLAY_STATUS)
        Widget::mutual->Pause();
    else
        return;
}

/**
 * @brief DbusAdapter::PlayPause
 * 当华为机器使用有线耳机暂停歌曲播放，其他应用会通过mpris协议调用音乐的PlayPause(播放或暂停）接口。
 *
 * 如果播放已暫停，则恢复播放；
 * 如果播放停止，则开始播放。
 * 若处于停止（Stopped）状态，且存在可播放曲目，调用此方法会播放播放列表中第一首曲目。
 *
 * 系统调用研发负责人： 孙军浩
 */
void DbusAdapter::PlayPause()
{
    KyInfo() << "PlayPause";
    //若当前状态为Stopped，则播放播放列表中第一首曲目
    if(Widget::mutual->getStates() == STOP_STATUS){
        g_user_signal->sigPlayFirst();
    }else{
        qDebug() << "into PlayPause throught mpris.";
        Widget::mutual->PlayPause();
    }

}
void DbusAdapter::notify(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    QString objectPath = "/org/mpris/MediaPlayer2";
    QDBusMessage signalMessage= QDBusMessage::createSignal(objectPath,"org.freedesktop.DBus.Properties","PropertiesChanged");
//    QVariantMap changed;
//    changed.insert("PlaybackStatus",QVariant(PlaybackStatus));
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
    KyInfo() << "Previous begin";
    if(Widget::mutual->getStates() == STOP_STATUS){
        KyInfo() << "Statues is STOP_STATUS";
        return;
    }

    QString playPath = playController::getInstance().getPath();
    if(playPath != ""){
        KyInfo() << "playPath:" <<playPath;
        Widget::mutual->Previous();
    }else{
        KyInfo() << "playPath is null";
    }
    KyInfo() << "Previous end";
}

void DbusAdapter::VolumeUp() const
{
    Widget::mutual->VolumeUp();
}

bool DbusAdapter::getShuffle()
{
   if(playController::getInstance().mode() == playController::Random)
       return true;
   else
       return false;
}

void DbusAdapter::setShuffle(bool isRandom){
    if(isRandom && playController::getInstance().mode() != playController::Random){
        playController::getInstance().getPlaylist()->setPlaybackMode(MediaPlaylist::Random);
    }else if(!isRandom && playController::getInstance().mode() != playController::Loop){
        playController::getInstance().getPlaylist()->setPlaybackMode(MediaPlaylist::Loop);
    }else{
        return;
    }
}

void DbusAdapter::VolumeDown() const
{
    Widget::mutual->VolumeDown();
}

void DbusAdapter::FullScreen() const
{
    Widget::mutual->slotShowMaximized();
}

void DbusAdapter::Exit() const
{
    Widget::mutual->slotClose();
}

/**
 * @brief Seek - 用于在当前播放的曲目中根据指定的微秒数进行快进或快退操作。
 *
 * @param Offset 快进或快退的时间，单位是微秒。正值表示快进，负值表示快退。
 *
 * @note
 * - 如果快退的值会使播放位置超出曲目开始位置，则将播放位置设置为 0（曲目起点）。
 * - 如果快进的值会使播放位置超出曲目结束位置，则效果等同于调用 'Next' 方法。
 * - 当前播放状态为 Stopped 时，调用此方法不会产生任何效果。
 * - 如果 'Offset' 的绝对值小于 0.1 秒，调用此方法同样不会产生任何效果。
 */
void DbusAdapter::Seek(const qint64 Offset)
{
    //状态为Stopped，表示当前播放曲目为空
    if(Widget::mutual->getStates() == STOP_STATUS){
        return;
    }else{
        qDebug() << "seek :" << Offset;
        // 如果小于0.1s就不进行操作
        if (qAbs(Offset) > 1000*99) {
            Widget::mutual->Seek(Offset / 1000);
        }else
            return;
    }
}

/**
 * @brief SetPosition - 将当前曲目的播放位置设置为指定的微秒数。
 *
 * @param TrackId 轨道标识符。当此参数为空时，表示当前播放的曲目。
 * @param Position 要设置的新位置，单位为微秒。
 *
 * @note
 * - 当 'TrackId' 为空时，此函数会尝试设置当前播放曲目的位置。
 * - 如果当前播放状态为 'Stopped' ，调用此函数不会产生任何效果。
 * - 如果 'Position' 小于0或大于当前轨道的长度，调用此函数同样不会产生任何效果。
 */
void DbusAdapter::SetPosition(const QString &TrackId, const qint64 Position)
{
    if(Widget::mutual->getStates() == STOP_STATUS){
        return;
    }else{
        if(TrackId.isEmpty()){
            //当Position大于等于0且小于等于当前轨道长度，调用此方法可生效
            if(Position >= 0 && Position / 1000 <= playController::getInstance().getMpvCorePointer()->duration()){
                Widget::mutual->Seek(Position / 1000 - playController::getInstance().getMpvCorePointer()->position());
            }else{
                qWarning() << "dbus SetPosition error, Position is :" << Position;
            }
        }//后期可增加判断TrackId是否与当前TrackId一致
    }
}


/**
 * @brief OpenUri - 打开由URI指定的媒体文件（添加文件至播放列表）。
 *
 * @param Uri 要打开的媒体文件的文件路径。
 *
 * @note
 * - 如果 'Uri' 是一个空字符串，则函数不会执行任何操作并立即返回。
 * - 如果 'Uri' 不为空，则添加此文件至播放列表。
 */
void DbusAdapter::OpenUri(const QString &Uri)
{
    if(Uri.isEmpty()){
        return;
    }else{
        g_user_signal->sigOpenUri(Uri);
    }
}


#if UKUI_PANEL_CAN_CONTROL
QString DbusAdapter::getState()
{
    return Widget::mutual->getStates();
}
#endif

double DbusAdapter::getRate()
{
    qDebug() << "getRate";
    return Widget::mutual->getPlayRate();
}

void DbusAdapter::setRate(double d)
{
    qDebug() << "setRate:" << d;
    if(Widget::mutual->getStates() == STOP_STATUS)
        return;
    else{
        if(Widget::mutual->getPlayRate() == d){
            return;
        }else{
            if(d == 0.5 || d == 0.75 || d == 1.0 || d == 1.25 || d == 1.5 || d == 2.0){
                if (d >= m_minimumRate && d <= m_maximumRate) {
                    Widget::mutual->setPlayRate(d);
                }else{
                    qWarning() << "dbus set rate error, rate is :" << d;
                }
            }else{
                return;
            }
        }
    }
}
