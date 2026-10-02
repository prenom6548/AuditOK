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

#include <QDebug>
#include <QFileInfo>
#include <QDBusConnection>
#include <ukui-log4qt.h>
#include "player.h"
#include "UIControl/base/musicDataBase.h"
#include "UIControl/base/musicfileinformation.h"
#include "UI/player/playsongarea.h"
#include "UI/base/widgetstyle.h"

/**
 * @brief playController::play 播放指定索引的音乐
 * @param playlist
 * @param index
 * @return
 */
bool playController::play(QString playlist, int index)
{
    if (playlist.compare(m_curList)==0) {
//        if (index == m_curIndex) {
//            if (m_player->state() == MpvCore::State::PlayingState){
//                play();
//            }else {
//                pause();
//            }
//        }
//        else {
            stop();
            setSongIndex(index);
            play();
//        }
        return true;
    }

    return false;
}

/**
 * @brief playController::play 歌曲列表指定播放
 * @param playlist 播放列表名
 * @param index 页面传来的索引值
 * @param playfile 页面播放的文件名
 * @param pathfileList 当前歌单列表所有的歌曲列表
 * @return
 */
bool playController::play(QString playlist, int index, QString playfile, QStringList pathfileList)
{
    if (playlist.compare(m_curList)==0) {
        stop();
        setSongIndex(index);
        play();

        return true;
    }

    return false;
}

bool playController::play()
{
    if (m_player == nullptr) {
        return false;
    }

    if (!m_player->isAvailable()) { //存疑
        return false;
    }

    if (m_player->state() == MpvCore::State::PlayingState) {
        m_player->pause();
    } else {
        m_player->play();
        Q_EMIT curIndexChanged(m_playlist->currentIndex());
    }

    return true;
}

bool playController::pause()
{
    if (m_player == nullptr) {
        return false;
    }

    m_player->pause();

    return true;
}

bool playController::pauseOnly()
{
    if (m_player == nullptr) {
        return false;
    }

    m_player->pauseOnly();

    return true;
}

bool playController::stop()
{
    if (m_player == nullptr) {
        return false;
    }

    m_player->stop();

    return true;
}

void playController::setSongIndex(int index)
{
    if (m_playlist == nullptr) {
        qDebug() << "m_playlist is null";
        return;
    }

    if(index < 0)
    {
        return;
    }

    if (index > m_playlist->mediaCount()) {
        return;
    }

    m_playlist->setCurrentIndex(index);

    Q_EMIT curIndexChanged(index);
}
int playController::songIndex()
{
    if (m_playlist == nullptr) {
        qDebug() << "m_playlist is null";
        return -1;
    }

    return m_playlist->currentIndex();
}

int playController::playmode()
{
    if (m_playlist == nullptr) {
        qDebug() << "m_playlist is null";
        return -1;
    }

    return static_cast<MediaPlaylist::PlaybackMode>(m_playlist->playbackMode());
}

void playController::setPlaymode(int mode)
{
    if (m_playlist == nullptr || m_player == nullptr) {
        return;
    }
    if (m_playlist == nullptr) {
        qDebug() << "m_playlist is null";
        return;
    }

//    if (mode < 0 || mode > 5) {
//        m_playlist->setPlaybackMode(MediaPlaylist::PlaybackMode::CurrentItemOnce);
//    } else
        m_playlist->setPlaybackMode(static_cast<MediaPlaylist::PlaybackMode>(mode));
}

void playController::curPlaylist()
{
    // Print Playlist information
    if (m_playlist == nullptr) {
        return;
    }

    for (auto i = 0; i < m_playlist->mediaCount(); i++) {
        MMediaContent content = m_playlist->media(i);
//        qDebug() << "   "
//                 << "media[" << i << "] is:" << content.canonicalUrl();
    }
}
void playController::setCurPlaylist(QString name, QStringList songPaths)
{
    if (m_curList.compare(name)==0) {
//        qDebug() << "setCurPlaylist m_curList.compare(name)==0" << m_curList << name;
//        return ;
    }

    if (m_playlist == nullptr || m_player == nullptr) {
        return;
    }

    int  notExistFileNumber = 0;

    disconnect(m_playlist,&MediaPlaylist::currentIndexChanged,this,&playController::slotIndexChange);
    m_curList = name;
    m_playlist->clear();

    for (auto path : songPaths) {
        bool isExists = QFileInfo::exists(QUrl::fromLocalFile(path).toLocalFile());
        if (! isExists) {
            notExistFileNumber++;
        }
        m_playlist->addMedia(QUrl::fromLocalFile(path));
    }

    qDebug() << "notExistFileNumber = " << notExistFileNumber << "songPaths.length: " << songPaths.length();
    if (notExistFileNumber == songPaths.length()) {
        // 统计所有的不存在文件数目，当和歌单列表数目一致时，则播放列表为空，防止卡住界面
        m_playlist->clear();
    }

    m_player->stop();

    m_player->setPlaylist(nullptr);
    if (notExistFileNumber != songPaths.length()) {
        m_player->setPlaylist(m_playlist);
    }


//    m_playlist->setCurrentIndex(-1);
    connect(m_playlist,&MediaPlaylist::currentIndexChanged,this,&playController::slotIndexChange);
    isInitialed = true;
}
void playController::addSongToCurList(QString name, QString songPath)
{
    if (name.compare(m_curList) != 0) {
        qDebug() << __FUNCTION__ << " the playlist to add is not Current playlist.";
        return;
    }

    int  notExistFileNumber = 0;

    if (m_playlist != nullptr) {

        bool isExists = QFileInfo::exists(QUrl::fromLocalFile(songPath).toLocalFile());
        if (! isExists) {
            notExistFileNumber++;
        }
        m_playlist->addMedia(QUrl::fromLocalFile(songPath));
    }
}
void playController::removeSongFromCurList(QString name, int index)
{
    if (name.compare(m_curList) != 0) {
//        qDebug() << __FUNCTION__ << " the playlist to add is not Current playlist.";
        return;
    }

    if (m_playlist != nullptr) {
//        m_playlist->removeMedia(index);
        //判断删除后 播放歌曲的index    当前只判断了删除正在播放的歌曲    还没做删除正在播放之前的歌曲和之后的歌曲
            int count = m_playlist->mediaCount();

            int state = m_player->state();

            if(m_curIndex == index) {
                stop();

                if(m_curIndex == count - 1) {
                    m_curIndex = 0;
                    m_playlist->removeMedia(index);

                    if(m_playlist->mediaCount() == 0)
                    {
                        m_curIndex = -1;
                    }
                    setSongIndex(m_curIndex);
                } else {
                    m_playlist->removeMedia(index);

                    if(m_playlist->mediaCount() == 0) {
                        m_curIndex = -1;
                    }
//                    setSongIndex(m_curIndex);
                    setSongIndex(m_curIndex);
                }

                if (state == MpvCore::State::PlayingState) {
                    m_player->play();
                } else {                            //设置进度条归 0
                    Q_EMIT signalSetValue();
                    m_player->stop();
                    Q_EMIT playController::getInstance().getMpvCorePointer()->positionChanged(0);
                }
                Q_EMIT curIndexChanged(m_curIndex);
                Q_EMIT currentIndexAndCurrentList(m_curIndex,m_curList);
                slotIndexChange(m_curIndex);
            } else if(m_curIndex > index) {
//                int position = 0;
//                if(m_player->state()==MpvCore::PlayingState)
//                {
//                    position = m_player->position();
//                }
//                m_player->stop();
                m_playlist->removeMedia(index);
                m_curIndex = m_curIndex - 1;
                //只负责高亮歌曲
                setSongIndex(m_curIndex);
//                m_player->setPosition(position);
                //歌曲删除重新播放(跨函数调用)
//                PlaySongArea::getInstance().hSlider->setValue(position);
                //MPV setPosition已经设置播放，不用再次设置播放
//                m_player->play();
                Q_EMIT curIndexChanged(m_curIndex);
                Q_EMIT currentIndexAndCurrentList(m_curIndex,m_curList);
                slotIndexChange(m_curIndex);
            } else if(m_curIndex < index) {
                m_playlist->removeMedia(index);
            }


//        auto cr_index = m_playlist->currentIndex();
//        Q_EMIT curIndexChanged(cr_index);
//        Q_EMIT currentIndexAndCurrentList(cr_index,m_curList);
        //删除正在播放的歌曲时，正在播放的歌曲名和时长实时更新
//        slotIndexChange(cr_index);
//        if(name == ALLMUSIC)
//        {

//        }
    }
}


void playController::removeSongFromLocalList(QString name, int index)
{
    if (name.compare(m_curList) != 0) {
        qDebug() << __FUNCTION__ << " the playlist to add is not Current playlist.";
        return;
    }

    if (m_playlist != nullptr) {
            int count = m_playlist->mediaCount();

            int state = m_player->state();

            if(m_curIndex == index) {
                stop();

                if(m_curIndex == count - 1) {
                    m_curIndex = 0;
                    m_playlist->removeMedia(index);

                    if(m_playlist->mediaCount() == 0) {
                        m_curIndex = -1;
                    }
                    setSongIndex(m_curIndex);
                } else {
                    m_playlist->removeMedia(index);

                    if(m_playlist->mediaCount() == 0) {
                        m_curIndex = -1;
                    }
                    setSongIndex(m_curIndex);
                }

                //删除当前播放歌曲不更改播放状态  2021.09.10
                if (state == MpvCore::State::PlayingState) {
                    m_player->play();
                } else {                            //设置进度条归 0
                    Q_EMIT signalSetValue();
                    m_player->pause();
                }
            } else if(m_curIndex > index) {
//                int position = 0;
//                if(m_player->state()==MpvCore::PlayingState)
//                {
//                    position = m_player->position();
//                }
//                m_player->stop();
                m_playlist->removeMedia(index);
                m_curIndex = m_curIndex - 1;
                setSongIndex(m_curIndex);
//                m_player->setPosition(position);
//                m_player->play();
            } else if(m_curIndex < index) {
                m_playlist->removeMedia(index);
            }

            Q_EMIT curIndexChanged(m_curIndex);

            Q_EMIT currentIndexAndCurrentList(m_curIndex,m_curList);

            slotIndexChange(m_curIndex);
    }
}

playController::PlayState playController::getState()
{
    if(m_player->state() == MpvCore::State::PlayingState)
        return PlayState::PLAY_STATE;
    else if(m_player->state() == MpvCore::State::PausedState)
        return PlayState::PAUSED_STATE;
    else if(m_player->state() == MpvCore::State::StoppedState)
        return PlayState::STOP_STATE;
}

playController::playController()
    : m_curList(""),m_curIndex(-1)
{
    m_player = new MpvCore(this);
    m_player->setObjectName("kylin-music_playcontroller_MpvCore_m_player");
    if (m_player == nullptr) {
        qDebug() << "failed to create player ";
        return;
    }
    m_playlist = new MediaPlaylist(m_player);
    m_playlist->setObjectName("kylin-music_playcontroller_MediaPlaylist_m_playlist");
    if (m_playlist == nullptr) {
        qDebug() << "failed to create laylist";
        return;
    }
    m_player->setPlaylist(m_playlist);
    init();
    initDbus();
    m_playlist->setPlaybackMode(MediaPlaylist::Loop);
//    m_playlist->setCurrentIndex(-1);
    connect(m_playlist,&MediaPlaylist::currentIndexChanged,this,&playController::slotIndexChange);
    connect(m_player,&MpvCore::stateChanged,this,&playController::slotStateChanged);
    connect(m_playlist,&MediaPlaylist::playbackModeChanged,this,&playController::slotPlayModeChange);
//    connect(m_player,&MpvCore::playErrorMsg,this,&playController::slotPlayErrorMsg);
    m_volume = volumeSetting->get("volume").toInt();
//    delayMsecond(100);
    m_player->setVolume(m_volume);
}

void playController::init()
{
    volumeSetting = new QGSettings(KYLINMUSIC);
    m_volume = volumeSetting->get("volume").toInt();
//    delayMsecond(100);
    m_player->setVolume(m_volume);
    playSetting = new QGSettings(KYLINMUSIC);
    m_playListName = playSetting->get("playlistname").toString();

    //如果保存的歌单是 歌曲列表 或者 我喜欢（我喜欢与歌曲列表必存在不做判断），那么保存的歌曲路径不做处理
    if(m_playListName == ALLMUSIC) {
        m_curList = ALLMUSIC;
    } else if(m_playListName == FAV) {
        m_curList = FAV;
    } else {
        int ret = g_db->checkPlayListExist(m_playListName);
        if(ret == DB_OP_SUCC) {
            if (m_playListName == SEARCH) {
                playSetting->set("playlistname", ALLMUSIC);
                m_curList = ALLMUSIC;
                playSetting->set("path", "");
            } else {
                m_curList = m_playListName;
                QString pat = playSetting->get("path").toString();
                int ref = g_db->checkIfSongExistsInPlayList(pat, m_curList);
                if(ref != DB_OP_SUCC) {
                    playSetting->set("path", "");
                }
            }
        } else {
            playSetting->set("playlistname", ALLMUSIC);
            m_curList = ALLMUSIC;
            playSetting->set("path", "");
        }
    }

    playModeSetting = new QGSettings(KYLINMUSIC);
    m_mode = static_cast<PlayMode>(playModeSetting->get("playbackmode").toInt());
}

void playController::initDbus()
{
    QDBusConnection::sessionBus().connect(QString(), "/", "org.ukui.media", "sinkVolumeChanged", this, SLOT(slotVolumeChange(QString,int,bool)));
}

int playController::getVolume()
{
    return m_volume;
}

void playController::delayMsecond(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

void playController::delayMsecondSetVolume()
{
    int currentVolume = getVolume();
    KyInfo() << "currentVolume = " << currentVolume
             << "playState = " << getMpvCorePointer()->state();

    delayMsecond(100);
    setVolume(currentVolume);
}

void playController::setVolume(int volume)
{
    if(volume > 100) {
        volume = 100;
    }

    if(volume < 0) {
        volume = 0;
    }

    m_volume = volume;
    volumeSetting->set("volume", volume);
    g_user_signal->sigVolumeChange(volume);
    KyInfo() << "m_receive = " << m_receive;
    if (!m_receive) {
        KyInfo() << "begin setVolume";
        m_player->setVolume(volume);
    } else {
        m_receive = false;
    }
}

void playController::onCurrentIndexChanged()
{
    qDebug() << "onCurrentIndexChanged";
}
void playController::onPositionChanged(double value)
{
    qDebug() << "onPositionChanged";
    if (m_player == nullptr) {
        return;
    }
    m_player->setPosition(m_player->duration() * value);
}
void playController::onNextSong()
{
//    DbusAdapter *m_adapter = new DbusAdapter;
    auto index = m_playlist->currentIndex();
    if (m_playlist == nullptr || m_player == nullptr) {
        qDebug() << "m_playlist or m_player is nullptr";
        return;
    }

    m_playlist->next();

    QString filePath = m_playlist->getPlayFileName();
    if (!QFileInfo::exists(QUrl(filePath).toLocalFile())){
        m_playlist->next();
        m_player->play();
        return;
    }else{
        m_player->play();
        // auto index = m_playlist->currentIndex();
        Q_EMIT curIndexChanged(index);
    }
//    m_player->play();
//    playController::getInstance().slotStateChanged(MpvCore::State::PlayingState);
    curPlaylist();
//    auto index = m_playlist->currentIndex();

//    QVariantMap changedProperties;
//    changedProperties.insert("PlaybackStatus",QVariant::fromValue(QDBusVariant(m_playbackStatus)));
//    m_adapter->notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
    Q_EMIT curIndexChanged(index);
}
void playController::onPreviousSong()
{
//    DbusAdapter *m_adapter = new DbusAdapter;
    auto index = m_playlist->currentIndex();
    if (m_playlist == nullptr || m_player == nullptr) {
        qDebug() << "m_playlist or m_player is nullptr";
        return;
    }

    m_playlist->previous();
//    playController::getInstance().slotStateChanged(MpvCore::State::PlayingState);
    QString filePath = m_playlist->getPlayFileName();
    if (!QFileInfo::exists(QUrl(filePath).toLocalFile())){
        m_playlist->previous();
        m_player->play();
        return;
    }else{
        m_player->play();
       // auto index = m_playlist->currentIndex();
        Q_EMIT curIndexChanged(index);
    }
//    QVariantMap changedProperties;
//    changedProperties.insert("PlaybackStatus",QVariant::fromValue(QDBusVariant(m_playbackStatus)));
//    m_adapter->notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
    Q_EMIT curIndexChanged(index);
}

void playController::setCurList(QString renameList)
{
    m_curList = renameList;
}

void playController::onError()
{
    qDebug() << "onError";
}

void playController::onMediaStatusChanged()
{
    qDebug() << "onMediaStatusChanged";
}

playController::~playController(/* args */)
{
    if (m_playlist != nullptr) {
        m_playlist->deleteLater();
    }

    if (m_player != nullptr) {
        m_player->deleteLater();
    }
}

bool playController::playSingleSong(QString Path, bool isPlayNowOrNext)
{
}

void playController::slotStateChanged(MpvCore::State newState)
{
    QString status;
    if (newState == MpvCore::State::PlayingState) {
        status = "Playing";
        Q_EMIT playerStateChange(playController::PlayState::PLAY_STATE);
    } else if (newState == MpvCore::State::PausedState) {
        status = "Paused";
        Q_EMIT playerStateChange(playController::PlayState::PAUSED_STATE);
    } else if (newState == MpvCore::State::StoppedState) {
        status = "Stopped";
        Q_EMIT playerStateChange(playController::PlayState::STOP_STATE);
    }
        g_user_signal->sigStatuChange(status);
}

void playController::slotPlayModeChange(MediaPlaylist::PlaybackMode mode)
{
    if(mode == MediaPlaylist::PlaybackMode::CurrentItemInLoop) {
        Q_EMIT signalPlayMode(static_cast<MediaPlaylist::PlaybackMode>(playController::PlayMode::CurrentItemInLoop));
    } else if(mode == MediaPlaylist::PlaybackMode::Sequential) {
        Q_EMIT signalPlayMode(static_cast<MediaPlaylist::PlaybackMode>(playController::PlayMode::Sequential));
    } else if(mode == MediaPlaylist::PlaybackMode::Loop) {
        Q_EMIT signalPlayMode(static_cast<MediaPlaylist::PlaybackMode>(playController::PlayMode::Loop));
    } else if(mode == MediaPlaylist::PlaybackMode::Random) {
        Q_EMIT signalPlayMode(static_cast<MediaPlaylist::PlaybackMode>(playController::PlayMode::Random));
    }
}

void playController::slotIndexChange(int index)
{
    if(index == -1) {
        Q_EMIT signalNotPlaying();
        //当index == -1时，会调用positionChanged导致时长显示错误
        Q_EMIT singalChangePath("");
        Q_EMIT currentIndexAndCurrentList(-1,m_curList);
        playSetting->set("playlistname", m_curList);
        playSetting->set("path", "");

        return;
    }

    m_curIndex = index;
    MMediaContent content = m_playlist->media(index);
    QString path = content.canonicalUrl().toLocalFile();
    QFileInfo file(path.remove("file://"));

    if(file.exists()) {
        x = 0;
        Q_EMIT currentIndexAndCurrentList(index,m_curList);
        Q_EMIT singalChangePath(path);

        if(m_curList == HISTORY) {
            playSetting->set("playlistname", ALLMUSIC);
        } else {
            playSetting->set("playlistname", m_curList);
        }
        playSetting->set("path", path);
    } else {
        x++;

        if(x > m_playlist->mediaCount()) {
            x = 0;
//            index = -1;
//            m_playlist->setCurrentIndex(index);

            return;
        }
    }
}

//void playController::slotPlayErrorMsg(MpvCore::ErrorMsg msg)
//{
////    m_msg = msg;
//    Q_EMIT playErrorMsg(msg);
//}

void playController::setPosition(int position)
{
    if (qAbs(m_player->position() - position) > 99) {
       m_player->setPosition(position);
    }
}

void playController::setPlayListName(QString playListName)
{
    playSetting->set("playlistname", playListName);
}

QString playController::getPlayListName()
{
    return playSetting->get("playlistname").toString();
}

QString playController::getPath()
{
    return playSetting->get("path").toString();
}

void playController::setMode(playController::PlayMode mode)
{
    m_mode = mode;
    playModeSetting->set("playbackmode", m_mode);
}

playController::PlayMode playController::mode() const
{
    return m_mode;
}

void playController::slotVolumeChange(QString app, int value, bool mute)
{
    if (app == "kylin-music") {
        if (value < 0) {
            return;
        }

        if (value != m_volume) {
            KyInfo() << "value != m_volume."
                     << "value = " << value;

            m_receive = true;
            Q_EMIT signalVolume(value);
        }

        //mute = true静音  mute = false取消静音
        Q_EMIT signalMute(mute);
    }
}
