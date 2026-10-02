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

#ifndef GLOBALSIZEDATA_H
#define GLOBALSIZEDATA_H

#ifndef UKUI_THEME_GSETTING_PATH
#define UKUI_THEME_GSETTING_PATH "org.ukui.style"
#endif

#include <QString>
#include <QSize>
#include <QColor>
#include <QPoint>

class GlobalSizeData
{
public:
    // 主题色
    static int THEME_COLOR;

    enum ThemeColor {
        UKUILight = 0,
        UKUIDark,
    };

    // 毛玻璃透明度
    static double BLUR_TRANSPARENCY;

    // 主界面
    static const int WINDOW_WIDTH;  //主界面宽度
    static const int WINDOW_HEIGHT; //主界面高度

    // 标题栏
    static const int TITLEBAR_HEIGHT; //标题栏高度

    // 本机信息
    static const int LOCALINFO_HEIGHT;       //本机信息高度
    static const QSize AVATAR_LAB_SIZE;      // 本机头像尺寸
    static const QSize USER_NAME_LAB_SIZE;   // 本机昵称尺寸
    static const QSize USER_IP_LAB_SIZE;     // 本机IP尺寸
    static const QSize CHANGE_NAME_BTN_SIZE; // 修改本机昵称按钮
    static const QSize CHANGE_NAME_BTN_ICON; // 修改本机昵称按钮图标
    static const QSize OPEN_FOLDER_BTN_SIZE; // 打开文件保存目录按钮
    static const QSize OPEN_FOLDER_BTN_ICON; // 打开文件保存目录按钮图标
    static const QSize SEARCH_EDIT_SIZE;     // 打开文件保存目录按钮
    static const QSize SEARCH_EDIT_ICON;     // 打开文件保存目录按钮图标

    //xc-平板尺寸改变
    static const int TITLEBAR_MAX_HEIGHT; //标题栏高度
    static const int LOCALINFO_MAX_HEIGHT;   // 本机信息最大高度
    static const int LOCALINFO_SEARCH_MAX_H_WIDEH;  // 搜索横屏宽度
    static const int LOCALINFO_SEARCH_MAX_V_WIDEH;  // 搜索竖屏宽度
    static const int LOCALINFO_SEARCH_MAX_HEIGHT;   // 搜索宽度
    static const QSize LOCALINFO_BTN_MAX_SIZE;   // 首页按钮大小

    // 好友列表
    static const int FRIEND_LIST_HEIGHT; //好友列表高度

    // 所选消息框坐标及尺寸
    static QPoint BORDER_POINT;
    static QSize BORDER_SIZE;

    // 主题
    static QString LIGHT_THEME;
    static QString BLACK_THEME;

    /* SDK功能打点 */
    static void SDKPointNicknameModify();
    static void SDKPointOpenSaveDir();
    static void SDKPointMainSearch();
    static void SDKPointHistorySearch();
    static void SDKPointSetTop();
    static void SDKPointModifyFriendNotes();
    static void SDKPointViewInformation();
    static void SDKPointChangeDir();
    static void SDKPointCleanCache();
    static void SDKPointClearChatRecord();
    static void SDKPointClearSingleChatRecord();
    static void SDKPointDeleteRecord();
    static void SDKPointBatchDelete();
    static void SDKPointSendMessage();
    static void SDKPointSendFiles();
    static void SDKPointSendDir();
    static void SDKPointSendScreenshot();
    static void SDKPointResend();
    static void SDKPointCopy();
    static void SDKPointOpen();
    static void SDKPointOpenDir();
    static void SDKPointSaveAs();
    static void SDKPointTray();

    friend class TestGlobalSizeData;

};

#endif // GLOBALSIZEDATA_H
