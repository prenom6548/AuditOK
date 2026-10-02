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
#ifndef PLAYCONTROLLER_H
#define PLAYCONTROLLER_H

#include <QObject>
#include <QGSettings>
#include "mmediaplayer.h"
#include "mmediaplaylist.h"


#define KYLINMUSIC "org.kylin-music-data.settings"
const QString ALLMUSIC = "LocalMusic";                    //本地总表
class playController : public QObject {
    Q_OBJECT
public:
    enum PlayState {
        STOP_STATE = 0,
        PLAY_STATE = 1,
        PAUSED_STATE = 2
    };

    enum PlayMode {
        CurrentItemInLoop = 1,
        Sequential = 2,
        Loop = 3,
        Random = 4,
    };
public:
    static playController& getInstance()
    {
        static playController instance;
        return instance;
    }

private:
    playController(/* args */);
    playController(playController const&);
    playController& operator=(playController const&);
    ~playController();
    void init();
    void initDbus();
public:
    //新增接口 fyf
    void playSingleSong(QString Path, int playMode);

    bool play(QString playlist, int index);

    bool play();
    bool pause();
    bool stop();

    int volume();

    void setPlayPosition(int pos);
    int playPosition();

    void setSongIndex(int index);
    int songIndex();

    int playmode();
    void setPlaymode(int mode);

    void curPlaylist();
    void setCurPlaylist(QString name, QStringList songPaths);
    void addSongToCurList(QString name, QString songPath);
    //从歌单删除
    void removeSongFromCurList(QString name, int index);
    //从本地删除
    void removeSongFromLocalList(QString name, int index);
//    void removeMedia(QString name, int index);
    PlayState getState();
    PlayMode mode() const;
    MMediaPlayer* getPlayer()
    {
        return m_player;
    }
    MMediaPlaylist* getPlaylist()
    {
        return m_playlist;
    }

    void setPosition(int position);
    //获取音量
    int getVolume();
    //设置音量
    void setVolume(int volume);
    //获取歌单名
    QString getPlayListName();
    //获取歌曲路径
    QString getPath();
Q_SIGNALS:
    void curPositionChanged(qint64);
    void curDurationChanged(qint64);
    void curIndexChanged(int index);
    void playerError(int error, QString errMsg);
    void playerStateChange(playController::PlayState newState);
    void singalChangePath(QString path);
    void currentIndexAndCurrentList(int index,QString listname);
    void signalPlayMode(int playModel);

    void signalNotPlaying();
//    void playErrorMsg(int errorCode);//媒体播放错误信息信号

    //进度条归 0
    void signalSetValue();
    //系统音乐音量改变
    void signalVolume(int value);
    //系统音乐音量静音
    void signalMute(bool mute);
public Q_SLOTS:
    void onCurrentIndexChanged();
    void onPositionChanged(double value);
    void onNextSong();
    void onPreviousSong();
    void setCurList(QString renameList);
    void setMode(playController::PlayMode mode);
    void setPlayListName(QString playListName);
private Q_SLOTS:
    void onError();
    void onMediaStatusChanged();
    //状态改变
    void slotStateChanged(MMediaPlayer::State newState);
    //播放模式改变
    void slotPlayModeChange(MMediaPlaylist::PlaybackMode mode);
    //获得当前播放的index
    void slotIndexChange(int index);
    //媒体播放错误信息槽函数
//    void slotPlayErrorMsg(MMediaPlayer::ErrorMsg msg);
    //接收系统应用音量变化
    void slotVolumeChange(QString app, int value, bool mute);
private:
    //当前播放列表名
    QString m_curList;
    //当前播放的索引
    int m_curIndex;
    MMediaPlayer* m_player  = nullptr;
    MMediaPlaylist* m_playlist  = nullptr;
    bool isInitialed = false;
    //在列表里歌曲（判断本地歌曲是否存在）没有播放的情况下，当前函数掉了多少次，要是歌曲在播放（找到本地路径存在，x重新计数
    int x = 0;
    int m_volume = 100;
    QGSettings *playSetting = nullptr;
    QString m_path;
    PlayMode m_mode = playController::Loop;
//    int m_msg = 0;
    //标记系统音乐接收状态
    bool m_receive = false;
};


#endif // PLAYCONTROLLER_H
