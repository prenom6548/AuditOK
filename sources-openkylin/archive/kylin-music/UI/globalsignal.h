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

#ifndef GLOBALSIGNAL_H
#define GLOBALSIGNAL_H

#include <QObject>


/** ******************************
 * 用户全局信号，主要用来发送主动信号
 *********************************/
class GlobalUserSignal : public QObject
{
    Q_OBJECT

public:
    static GlobalUserSignal* getInstance();

    bool historyButtonIsShow = false;
    bool lyricsButtonIsShow = false;
    bool isRTLLang = false;
    void exitApp(); // 退出
    void deleteAllHistoryPlaylistMessageBox(); // 显示历史列表删除对话框
    void hideSearchResult(); // 隐藏搜索列表
    void repaintLeftSidebarWidget(); // 刷新左侧歌单

//signals:
Q_SIGNALS:
    void sigExit();
    void sigDeleteAllHistoryPlaylistMessageBox();
    void sigHideSearchResult();
    void sigRepaintLeftSidebarWidget();
    void initPlaySongArea();            //将进度条设置为00:00，将进度条上方歌曲名字设置为“音乐”
    void sigPlayFirst();                //播放播放列表中的第一个
    void sigOpenUri(QString);                  //调用了DBus接口OpenUri
    void sigStatuChange(QString);              //播放状态发生改变
    void sigLoopStatusChange(QString);         //当前循环/重复状态发生切换
    void sigrateChange(double);                //当前播放速率发生改变
    void sigVolumeChange(int);              //音量发生改变
private:
    explicit GlobalUserSignal(QObject *parent = nullptr);
    static GlobalUserSignal* instance;
};


#define g_user_signal GlobalUserSignal::getInstance()

#endif // GLOBALSIGNAL_H
