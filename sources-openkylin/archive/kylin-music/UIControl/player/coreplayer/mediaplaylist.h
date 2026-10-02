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

#ifndef MediaPlaylist_H
#define MediaPlaylist_H

#include <QObject>
#include <QDebug>
#include <QFileInfo>
#include <QUrl>
#include <QDateTime>
#include "UI/globalsignal.h"

class MMediaContent
{
public:
    MMediaContent(QUrl url);
    QUrl canonicalUrl() const; //获取Qurl内容

private:
    QUrl m_url; //媒体url路径
};


class MediaPlaylist : public QObject
{
    Q_OBJECT

public:
    MediaPlaylist(QObject *parent = nullptr);

    //播放循环模式枚举
    enum PlaybackMode {
        CurrentItemOnce=0, // 只播放一次
        CurrentItemInLoop, // 单曲循环
        Sequential, // 列表播放
        Loop, // 列表循环
        Random // 随机播放
    };

    QString getPlayFileName(); //获取待播放媒体的文件名
    int currentIndex() const; //获取当前播放的歌曲在播放列表中的位置
    bool addMedia(const QUrl &items); //添加媒体到播放列表
    void next(); //切换下一首
    void previous(); //切换上一首
    void setCurrentIndex(int index); //设置选中播放列表中的位置
    PlaybackMode playbackMode() const; //获取播放循环模式
    void setPlaybackMode(PlaybackMode mode); //设置循环播放模式
    int mediaCount() const; //列表中总媒体的数量
    MMediaContent media(int index) const; //获取列表中特定位置的媒体
    bool clear(); //清空列表
    bool removeMedia(int pos); //移出特定位置的歌曲

public Q_SLOTS:
    void playFinish(); //播放完成槽函数
    void playError(); //播放异常槽函数
    void playErrorMsg(int Damage); //播放错误异常

Q_SIGNALS:
    void currentIndexChanged(int); //媒体切换信号
    void playbackModeChanged(MediaPlaylist::PlaybackMode mode); //播放循环模式切换信号
    void autoPlay(MediaPlaylist::PlaybackMode playbackMode); //自动播放下一首信号
    void stop(); //停止播放信号

private:
    int randomIndex(); //生成随机数
    QList<QUrl> m_playerList; //实际的播放队列
    int m_index = 0; //当前播放的歌曲位置
    int notExistFileNumber = 0; // 播放列表中不存在的文件数目，用于判断当前歌单列表在“循环”或“随机”模式下是否需要继续播放
    PlaybackMode m_playbackMode = Loop; //当前列表的循环模式
};

#endif // MediaPlaylist_H
