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
#include <QDBusConnection>
#include "playcontroller.h"

bool playController::play(QString playlist, int index)
{
    if (playlist.compare(m_curList)==0)
    {
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
    if (m_player->isAvailable() == false) { //存疑
        return false;
    }
    if (m_player->state() == MMediaPlayer::State::PlayingState) {
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

    return static_cast<MMediaPlaylist::PlaybackMode>(m_playlist->playbackMode());
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
    m_playlist->setPlaybackMode(static_cast<MMediaPlaylist::PlaybackMode>(mode));
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
    if (m_curList.compare(name)==0)
    {
//        qDebug() << "setCurPlaylist m_curList.compare(name)==0" << m_curList << name;
//        return ;
    }
    if (m_playlist == nullptr || m_player == nullptr) {
        return;
    }
    disconnect(m_playlist,&MMediaPlaylist::currentIndexChanged,this,&playController::slotIndexChange);
    m_curList = name;
    m_playlist->clear();

    for (auto path : songPaths) {
        m_playlist->addMedia(QUrl::fromLocalFile(path));
    }
    m_player->stop();
    m_player->setPlaylist(nullptr);
    m_player->setPlaylist(m_playlist);


//    m_playlist->setCurrentIndex(-1);
    connect(m_playlist,&MMediaPlaylist::currentIndexChanged,this,&playController::slotIndexChange);
    isInitialed = true;
}
void playController::addSongToCurList(QString name, QString songPath)
{
    if (name.compare(m_curList) != 0) {
        qDebug() << __FUNCTION__ << " the playlist to add is not Current playlist.";
        return;
    }
    if (m_playlist != nullptr) {
        m_playlist->addMedia(QUrl::fromLocalFile(songPath));
    }
}
void playController::removeSongFromCurList(QString name, int index)
{
    if (name.compare(m_curList) != 0)
    {
//        qDebug() << __FUNCTION__ << " the playlist to add is not Current playlist.";
        return;
    }
    if (m_playlist != nullptr) {
//        m_playlist->removeMedia(index);
        //判断删除后 播放歌曲的index    当前只判断了删除正在播放的歌曲    还没做删除正在播放之前的歌曲和之后的歌曲
            int count = m_playlist->mediaCount();

            int state = m_player->state();

            if(m_curIndex == index)
            {
                stop();
                if(m_curIndex == count - 1)
                {
                    m_curIndex = 0;
                    m_playlist->removeMedia(index);
                    if(m_playlist->mediaCount() == 0)
                    {
                        m_curIndex = -1;
                    }
                    setSongIndex(m_curIndex);
                }
                else
                {
                    m_playlist->removeMedia(index);
                    if(m_playlist->mediaCount() == 0)
                    {
                        m_curIndex = -1;
                    }
//                    setSongIndex(m_curIndex);
                    setSongIndex(m_curIndex);
                }
                //删除当前播放歌曲不更改播放状态  2021.09.10
                if (state == MMediaPlayer::State::PlayingState) {
                    m_player->play();
                } else {                            //设置进度条归 0
                    Q_EMIT signalSetValue();
                    m_player->pause();
                }
            }
            else if(m_curIndex > index)
            {
//                int position = 0;
//                if(m_player->state()==MMediaPlayer::PlayingState)
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
            }
            else if(m_curIndex < index)
            {
                m_playlist->removeMedia(index);
            }

            Q_EMIT curIndexChanged(m_curIndex);
            Q_EMIT currentIndexAndCurrentList(m_curIndex,m_curList);
            slotIndexChange(m_curIndex);
    }
}


void playController::removeSongFromLocalList(QString name, int index)
{
    if (name.compare(m_curList) != 0)
    {
        qDebug() << __FUNCTION__ << " the playlist to add is not Current playlist.";
        return;
    }
    if (m_playlist != nullptr)
    {
            int count = m_playlist->mediaCount();

            int state = m_player->state();

            if(m_curIndex == index)
            {
                stop();
                if(m_curIndex == count - 1)
                {
                    m_curIndex = 0;
                    m_playlist->removeMedia(index);
                    if(m_playlist->mediaCount() == 0)
                    {
                        m_curIndex = -1;
                    }
                    setSongIndex(m_curIndex);
                }
                else
                {
                    m_playlist->removeMedia(index);
                    if(m_playlist->mediaCount() == 0)
                    {
                        m_curIndex = -1;
                    }
                    setSongIndex(m_curIndex);
                }
                //删除当前播放歌曲不更改播放状态  2021.09.10
                if (state == MMediaPlayer::State::PlayingState) {
                    m_player->play();
                } else {                            //设置进度条归 0
                    Q_EMIT signalSetValue();
                    m_player->pause();
                }
            }
            else if(m_curIndex > index)
            {

                m_playlist->removeMedia(index);
                m_curIndex = m_curIndex - 1;
                setSongIndex(m_curIndex);
            }
            else if(m_curIndex < index)
            {
                m_playlist->removeMedia(index);
            }

            Q_EMIT curIndexChanged(m_curIndex);
            Q_EMIT currentIndexAndCurrentList(m_curIndex,m_curList);
            slotIndexChange(m_curIndex);
    }
}

playController::PlayState playController::getState()
{
    if(m_player->state() == MMediaPlayer::State::PlayingState)
        return PlayState::PLAY_STATE;
    else if(m_player->state() == MMediaPlayer::State::PausedState)
        return PlayState::PAUSED_STATE;
    else if(m_player->state() == MMediaPlayer::State::StoppedState)
        return PlayState::STOP_STATE;
    else
        return PlayState::STOP_STATE;
}

playController::playController()
    : m_curList(""),m_curIndex(-1)
{
    m_player = new MMediaPlayer(this);
    m_player->setObjectName("ukui-clock_MMediaPlayer_m_player");
    if (m_player == nullptr) {
        qDebug() << "failed to create player ";
        return;
    }
    m_playlist = new MMediaPlaylist(m_player);
    m_playlist->setObjectName("ukui-clock_MMediaPlaylist_m_playlist");
    if (m_playlist == nullptr) {
        qDebug() << "failed to create laylist";
        return;
    }
    m_player->setPlaylist(m_playlist);
    init();
    //绑定铃声音量
//    initDbus();
    m_playlist->setPlaybackMode(MMediaPlaylist::Loop);
//    m_playlist->setCurrentIndex(-1);
    connect(m_playlist,&MMediaPlaylist::currentIndexChanged,this,&playController::slotIndexChange);
    connect(m_player,&MMediaPlayer::stateChanged,this,&playController::slotStateChanged);
    connect(m_playlist,&MMediaPlaylist::playbackModeChanged,this,&playController::slotPlayModeChange);
//    connect(m_player,&MMediaPlayer::playErrorMsg,this,&playController::slotPlayErrorMsg);
}


void playController::init()
{
    m_volume=100;
    m_player->setVolume(100);
    m_curList = ALLMUSIC;
    m_mode = Loop;
}

void playController::initDbus()
{
    QDBusConnection::sessionBus().connect(QString(), "/", "org.ukui.media", "sinkVolumeChanged", this, SLOT(slotVolumeChange(QString,int,bool)));
}

int playController::getVolume()
{
    return m_volume;
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
    if (!m_receive) {
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
    if (m_playlist == nullptr || m_player == nullptr) {
        qDebug() << "m_playlist or m_player is nullptr";
        return;
    }
    m_playlist->next();
    m_player->play();
    curPlaylist();
    auto index = m_playlist->currentIndex();
    Q_EMIT curIndexChanged(index);
}
void playController::onPreviousSong()
{
    if (m_playlist == nullptr || m_player == nullptr) {
        qDebug() << "m_playlist or m_player is nullptr";
        return;
    }
    m_playlist->previous();
    m_player->play();
    auto index = m_playlist->currentIndex();
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

void playController::playSingleSong(QString Path, int playMode)
{
    qInfo()<<"musicPath:1"<<Path;
    QStringList filePaths;
    filePaths<<Path;
    qInfo()<<"musicPath:2"<<filePaths;
    getInstance().setCurPlaylist(ALLMUSIC,filePaths);
    getInstance().setPlaymode(playMode);
    getInstance().play(ALLMUSIC,0);
}

void playController::slotStateChanged(MMediaPlayer::State newState)
{
    if(newState == MMediaPlayer::State::PlayingState)
        Q_EMIT playerStateChange(playController::PlayState::PLAY_STATE);
    else if(newState == MMediaPlayer::State::PausedState)
        Q_EMIT playerStateChange(playController::PlayState::PAUSED_STATE);
    else if(newState == MMediaPlayer::State::StoppedState)
        Q_EMIT playerStateChange(playController::PlayState::STOP_STATE);

}

void playController::slotPlayModeChange(MMediaPlaylist::PlaybackMode mode)
{
    if(mode == MMediaPlaylist::PlaybackMode::CurrentItemInLoop)
        Q_EMIT signalPlayMode(static_cast<MMediaPlaylist::PlaybackMode>(playController::PlayMode::CurrentItemInLoop));
    else if(mode == MMediaPlaylist::PlaybackMode::Sequential)
        Q_EMIT signalPlayMode(static_cast<MMediaPlaylist::PlaybackMode>(playController::PlayMode::Sequential));
    else if(mode == MMediaPlaylist::PlaybackMode::Loop)
        Q_EMIT signalPlayMode(static_cast<MMediaPlaylist::PlaybackMode>(playController::PlayMode::Loop));
    else if(mode == MMediaPlaylist::PlaybackMode::Random)
        Q_EMIT signalPlayMode(static_cast<MMediaPlaylist::PlaybackMode>(playController::PlayMode::Random));
}

void playController::slotIndexChange(int index)
{
    if(index == -1)
    {
        Q_EMIT signalNotPlaying();
        //当index == -1时，会调用positionChanged导致时长显示错误
        Q_EMIT singalChangePath("");
        Q_EMIT currentIndexAndCurrentList(-1,m_curList);
        return;
    }
    m_curIndex = index;
    MMediaContent content = m_playlist->media(index);
    QString path = content.canonicalUrl().toLocalFile();
    QFileInfo file(path.remove("file://"));
    if(file.exists())
    {
        x = 0;
        Q_EMIT currentIndexAndCurrentList(index,m_curList);
        Q_EMIT singalChangePath(path);
    }
    else
    {
        x++;
        if(x > m_playlist->mediaCount())
        {
            x = 0;
            return;
        }
    }
}

void playController::setPosition(int position)
{
    if (qAbs(m_player->position() - position) > 99)
       m_player->setPosition(position);
}

void playController::setPlayListName(QString playListName)
{
    m_curList=playListName;
}

QString playController::getPlayListName()
{
    return m_curList;
}

QString playController::getPath()
{
    return m_path;
}

void playController::setMode(playController::PlayMode mode)
{
    m_mode = mode;
}

playController::PlayMode playController::mode() const
{
    return m_mode;
}

void playController::slotVolumeChange(QString app, int value, bool mute)
{
    if (app == "ukui-clock") {
        if (value < 0) {
            return;
        }

        if (value != m_volume) {
            m_receive = true;
            Q_EMIT signalVolume(value);
        }

        //mute = true静音  mute = false取消静音
        Q_EMIT signalMute(mute);
    }
}
