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

#include "mediaplaylist.h"

MediaPlaylist::MediaPlaylist(QObject *parent)
    : QObject(parent)
{
}

QString MediaPlaylist::getPlayFileName()
{
    //异常情况：播放列表为空
    if (m_playerList.isEmpty()) {
        return "";
    }
    //异常情况：当前播放的媒体在列表中的位置超过列表中媒体总数量
    if (m_index >= m_playerList.length()) {
        m_index = m_playerList.length();
        return m_playerList.last().toString();
    }

    return m_playerList.at(m_index).toString();
}

int MediaPlaylist::currentIndex() const
{
    return m_index;
}

/**
 * @brief MediaPlaylist::addMedia 添加要播放的歌曲文件
 * @param items 要添加的歌曲文件
 * @return
 */
bool MediaPlaylist::addMedia(const QUrl &items)
{
    bool isExists = QFileInfo::exists(items.toLocalFile());
    if (isExists) {
        // 单修改此处，会导致界面点击和实际播放不一致问题
//        m_playerList.append(items);
        notExistFileNumber++;
    }

    m_playerList.append(items);

    return true;
}

void MediaPlaylist::next()
{
    //异常情况：播放列表为空
    if (m_playerList.isEmpty()) {
        return;
    }

    //异常情况：无当前播放媒体
    if (m_index < 0) {
        return;
    }

    switch (m_playbackMode) {
    case Random:
        m_index=randomIndex();
        //异常情况：无当前播放媒体
        if (m_index < 0) {
            return;
        }

        break;
    case Sequential:
        m_index++;
        if (m_index >= m_playerList.length()) {
           m_index = m_playerList.length() - 1;
        }
        break;

//    case CurrentItemInLoop:
//        if(!QFileInfo::exists(QUrl(getPlayFileName()).toLocalFile())){
//            m_index++;
//        }else{
//            m_index=currentIndex();
//        }
//        break;

    default:
        m_index++;
        if (m_index >= m_playerList.length()) {
           m_index = 0;
        }
        break;
    }
    Q_EMIT currentIndexChanged(m_index);
    Q_EMIT stop();
}

void MediaPlaylist::previous()
{
    //异常情况：播放列表为空
    if (m_playerList.isEmpty()) {
        return;
    }

    switch (m_playbackMode) {
    case Random:
        m_index=randomIndex();
        //异常情况：无当前播放媒体，或当前播放列表为空
        if (m_index < 0) {
            return;
        }
        break;
    case Sequential:
        m_index--;
        if (m_index < 0) {
            m_index = 0;
        }
        break;
    default:
        m_index--;
        if (m_index < 0) {
           m_index = m_playerList.length() - 1;
        }
        break;
    }

    Q_EMIT currentIndexChanged(m_index);

    Q_EMIT stop();
}

void MediaPlaylist::setCurrentIndex(int index)
{
    //待设置的数量和设置之前一致则不处理，默认播放第一首除外
//    if (index == m_index && index != 0) {
//        return;
//    }
    //异常情况：要设置的媒体位置超过列表总长度
//    if (index >= m_playerList.length()) {
//        qDebug()<<"指定位置超过列表元素数量";
//        return;
//    }
    //统一所有非正常情况
    if (index < 0) {
        index = -1;
    }

    m_index = index;

    Q_EMIT currentIndexChanged(m_index);
}

void MediaPlaylist::setPlaybackMode(MediaPlaylist::PlaybackMode mode)
{
    //待设置的循环模式和设置之前一致则不处理
    if (mode == m_playbackMode) {
        return;
    }

    m_playbackMode = mode;

    QString loopStatus;
    switch (mode) {
    case MediaPlaylist::CurrentItemInLoop:
        loopStatus = "Track";
        break;
    case MediaPlaylist::Loop:
        loopStatus = "Playlist";
        break;
    case MediaPlaylist::Random:
        loopStatus = "Random";
    default:
        break;
    }
    g_user_signal->sigLoopStatusChange(loopStatus);
    Q_EMIT playbackModeChanged(mode);
}

int MediaPlaylist::mediaCount() const
{
    return m_playerList.length();
}

MMediaContent MediaPlaylist::media(int index) const
{
    //异常情况：要设置的媒体位置在列表中不存在
    if (index >= m_playerList.length() || index < 0) {
        return MMediaContent(QUrl(""));
    }

    return MMediaContent(m_playerList.at(index));
}

bool MediaPlaylist::clear()
{
    m_playerList.clear();

    return true;
}

bool MediaPlaylist::removeMedia(int pos)
{
    //异常情况：要移出的媒体位置在列表中不存在
    if (pos >= m_playerList.length() || pos < 0) {
        return false;
    }

    m_playerList.removeAt(pos);

    return true;
}


/**
 * @brief MediaPlaylist::playError 当前歌曲播放失败时触发的接口
 *
 * 播放当前歌曲失败情况:
 *  1. 当前歌曲损坏;
 *  2. 当前歌曲在系统中不存在，这种情况比如播放U盘中的音乐，但拔出U盘后，此时歌曲文件在列表中置灰，播放会出问题
 */
void MediaPlaylist::playError()
{
    //当前仅在存在播放列表中的媒体本地文件被删除时触发
    //播放异常时，轮询所有列表中的媒体文件是否存在
    for (auto url : m_playerList) {
        //如果发现列表中有媒体文件没被删除
        bool isExists = QFileInfo::exists(url.toLocalFile());
        qDebug() << "当前歌曲是否存在： " << url.toLocalFile() << isExists;

        if (isExists) {
            //如果是单曲循环则切换下一首
//            if (m_playbackMode == CurrentItemInLoop) {
                //m_index++;
//                next();

//            }

            //按播放完成处理
            playFinish();

            return;
        }
    }

    //列表中所有媒体的本地文件全部被删除了
    Q_EMIT currentIndexChanged(-1);
}

void MediaPlaylist::playErrorMsg(int Damage)
{
    if (Damage == -2) {
        if (mediaCount() == 0) {
               return;
        }
        if (m_playerList.isEmpty()) {
               return;
           }
        if (m_index < 0) {
               return;
           }
        if (m_index >= m_playerList.length()) {
               m_index = 0;
           }
    }
}

void MediaPlaylist::playFinish()
{
    //如果没有待播放的媒体则不处理
    if (m_index < 0) {
        return;
    }

    //如果循环模式不是单曲循环则切换下一首
    if (m_playbackMode != CurrentItemInLoop) {
        next();
        Q_EMIT currentIndexChanged(m_index);
    }

    if(m_playbackMode == CurrentItemInLoop){
        qDebug() << "单曲循环模式检测文件是否存在";
        if(!QFileInfo::exists(QUrl(getPlayFileName()).toLocalFile()))
            next();
    }

    Q_EMIT autoPlay(m_playbackMode);
}

MediaPlaylist::PlaybackMode MediaPlaylist::playbackMode() const
{
    return m_playbackMode;
}


/**
 * @brief MediaPlaylist::randomIndex 随机播放时的列表索引
 * @return 随机播放时的列表索引, 当为-1，代表当前歌曲列表的歌曲都不存在，不用播放
 */
int MediaPlaylist::randomIndex()
{
    qsrand(QDateTime::currentDateTime().toMSecsSinceEpoch());

    int lenPlaylist = m_playerList.length();
    if (lenPlaylist == 0) {
        // 播放列表沒有歌曲，此时不应有可用值
        return -1;
    }

    return qrand()%(lenPlaylist);
}


MMediaContent::MMediaContent(QUrl url)
{
    m_url = url;
}

QUrl MMediaContent::canonicalUrl() const
{
    return m_url;
}
