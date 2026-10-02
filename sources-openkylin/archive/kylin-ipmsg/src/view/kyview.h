/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef KYVIEW_H
#define KYVIEW_H

#define WINDOWW 350      //窗口宽度
#define WINDOWH 686      //窗口高度
#define WINDOWW_MAX_H_SCREEN 551  //窗口最大宽度
#define WINDOWW_MAX_V_SCREEN 1080  //窗口最大宽度
#define WINDOWH_MAX_H_SCREEN 1080  //窗口最大高度
#define WINDOWH_MAX_V_SCREEN 1620  //窗口最大高度

#define TITLEH 40        //标题栏高度
#define TITLEH_MAX 64    //标题栏高度

#define LOCALINFOH 140     //本机信息高度
#define LOCALINFOH_MAX 169 //本机信息高度

#define FRIENDSLISTH 506               //好友列表高度
#define FRIENDSLISTH_MAX_H_SCREEN 847  //好友列表高度
#define FRIENDSLISTH_MAX_V_SCREEN 1387 //好友列表高度

#include <QApplication>
#include <QVBoxLayout>
#include <KWindowSystem>
#include <QScreen>
#include <QSystemTrayIcon>
#include <QStackedWidget>
#include <windowmanager/windowmanager.h>

#include "ukuiwindowhelper.h"

#include "view/titlebar/titlebar.h"
#include "view/localinfo/localinfo.h"
#include "view/common/globalsizedata.h"
#include "view/friendlist/friendlist.h"
#include "view/chatmsg/chatmsg.h"
#include "view/trayicon/trayiconwid.h"
#include "view/friendlist/homepagesearch/searchpage.h"
#include "view/friendlist/homepagesearch/blankpage.h"

class KyView : public QWidget
{
    Q_OBJECT
    friend class TestKyView;

public:
    explicit KyView(QWidget *parent = nullptr);
    ~KyView();

    // 标题栏
    TitleBar *m_titleBar;

    // 生成静态实例
    static KyView *getInstance();

    void showList();
    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle();

    // 初始化并监听gsetting
    void initGsetting();

    // 显示主界面后检查本机昵称
    void checkLocalName();

    // 窗口关闭事件
    void closeEvent(QCloseEvent *event);

//    QString getWindowId(void);
    QString m_windowId = "";
    WindowId m_mainwindowId;

private:
    // 整体界面窗体
    QWidget *m_mainWid;

    //堆栈窗体
    QStackedWidget *m_stackedWid;

    // 空白页面
    BlankPage *m_blankWid;

    // 本机信息
    LocalInfo *m_localInfo;

    // 好友列表
    FriendListView *m_friendView;

    // 搜索列表
    SearchPage *m_searchPage;

    // 聊天消息框
    ChatMsg *m_chatMsg;

    // 整体界面布局
    QVBoxLayout *m_mainLayout;

    // 系统托盘
    TrayIconWid *m_trayIconWid;

    UkuiWindowHelper *m_windowHelper;

    QColor m_backColor;

    // 透明度变化
    void transChange();

    // 绘制事件
    void paintEvent(QPaintEvent *event);

    void keyPressEvent(QKeyEvent *event);

    bool event(QEvent *event);

    // 事件过滤器
    bool eventFilter(QObject *watch, QEvent *e);

public slots:
    void slotSearchInterface();
    void slotShowBlankPage();
    void slotFriendInterface();
    void slotChangeStatck();
    void slotShowSearchBlankPage();
    void slotChangeHScreenMode();   // 平板横屏
    void slotChangeVScreenMode();   // 平板竖屏
    void slotChangePCMode();   // 平板竖屏
signals:
    void sigChangeSearch();
    void sigChangeBlank();
    void sigChangeFriend();
    void sigChangeSearchBlank();
};




#endif // KYVIEW_H
