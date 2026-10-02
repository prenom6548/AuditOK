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
#include "mmediaplaylist.h"

MMediaPlaylist::MMediaPlaylist(QObject *parent)
    : QObject(parent)
{

}

QString MMediaPlaylist::getPlayFileName()
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

int MMediaPlaylist::currentIndex() const
{
    return m_index;
}

bool MMediaPlaylist::addMedia(const QUrl &items)
{
    m_playerList.append(items);
    return true;
}

void MMediaPlaylist::next()
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
        break;
    case Sequential:
        m_index++;
        if (m_index >= m_playerList.length()) {
           m_index = m_playerList.length() - 1;
        }
        break;
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

void MMediaPlaylist::previous()
{
    //异常情况：播放列表为空
    if (m_playerList.isEmpty()) {
        return;
    }

    switch (m_playbackMode) {
    case Random:
        m_index=randomIndex();
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

void MMediaPlaylist::setCurrentIndex(int index)
{
    //待设置的数量和设置之前一致则不处理，默认播放第一首除外
//    if (index == m_index && index != 0) {
//        return;
//    }
    //异常情况：要设置的媒体位置超过列表总长度
    if (index >= m_playerList.length()) {
        qDebug()<<"指定位置超过列表元素数量";
        return;
    }
    //统一所有非正常情况
    if (index < 0) {
        index = -1;
    }
    m_index = index;
    Q_EMIT currentIndexChanged(m_index);
}

void MMediaPlaylist::setPlaybackMode(MMediaPlaylist::PlaybackMode mode)
{
    //待设置的循环模式和设置之前一致则不处理
    if (mode == m_playbackMode) {
        return;
    }
    m_playbackMode = mode;
    Q_EMIT playbackModeChanged(mode);
}

int MMediaPlaylist::mediaCount() const
{
    return m_playerList.length();
}

MMediaContent MMediaPlaylist::media(int index) const
{
    //异常情况：要设置的媒体位置在列表中不存在
    if (index >= m_playerList.length() || index < 0) {
        return MMediaContent(QUrl(""));
    }
    return MMediaContent(m_playerList.at(index));
}

bool MMediaPlaylist::clear()
{
    m_playerList.clear();
    return true;
}

bool MMediaPlaylist::removeMedia(int pos)
{
    //异常情况：要移出的媒体位置在列表中不存在
    if (pos >= m_playerList.length() || pos < 0) {
        return false;
    }
    m_playerList.removeAt(pos);
    return true;
}

void MMediaPlaylist::playError()
{
    //当前仅在存在播放列表中的媒体本地文件被删除时触发
    //播放异常时，轮询所有列表中的媒体文件是否存在
    for (auto url : m_playerList) {
        //如果发现列表中有媒体文件没被删除
        if (QFileInfo::exists(url.toLocalFile())) {
            //如果是单曲循环则切换下一首
            if (m_playbackMode == CurrentItemInLoop) {
                next();
            }
            //按播放完成处理
            palyFinish();
            return;
        }
    }
    //列表中所有媒体的本地文件全部被删除了
    Q_EMIT currentIndexChanged(-1);
}

void MMediaPlaylist::playErrorMsg(int Damage)
{
    if (Damage == -2) {
        //如果是列表循环则切换下一首
        if (m_playbackMode == Loop) {
            next();
        } else if(m_playbackMode == Random) {
            m_index = randomIndex();
            Q_EMIT currentIndexChanged(m_index);
            Q_EMIT stop();
        }
        //不知为什么要加自动播放信号
//        Q_EMIT autoPlay(m_playbackMode);
    }
}

void MMediaPlaylist::palyFinish()
{
    //如果没有待播放的媒体则不处理
    if (m_index < 0) {
        return;
    }
    if (m_playbackMode == CurrentItemOnce) {
        return;
    }
    //如果循环模式不是单曲循环则切换下一首
    if (m_playbackMode != CurrentItemInLoop) {
        next();
        Q_EMIT currentIndexChanged(m_index);
    }
    Q_EMIT autoPlay(m_playbackMode);
}

MMediaPlaylist::PlaybackMode MMediaPlaylist::playbackMode() const
{
    return m_playbackMode;
}

int MMediaPlaylist::randomIndex()
{
    qsrand(QDateTime::currentDateTime().toMSecsSinceEpoch());
    return qrand()%(m_playerList.length());
}


MMediaContent::MMediaContent(QUrl url)
{
    m_url = url;
}

QUrl MMediaContent::canonicalUrl() const
{
    return m_url;
}
