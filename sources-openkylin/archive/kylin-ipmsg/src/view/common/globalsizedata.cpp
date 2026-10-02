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

/* SDK gsetting */
#include <QDebug>
#include "buriedpoint.hpp"

#include "globalsizedata.h"

// 主题色
int GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUILight;

// 毛玻璃透明度
double GlobalSizeData::BLUR_TRANSPARENCY = 150;

// 主界面
const int GlobalSizeData::WINDOW_WIDTH = 350;
const int GlobalSizeData::WINDOW_HEIGHT = 686;

// 标题栏
const int GlobalSizeData::TITLEBAR_HEIGHT = 40;

// 本机信息
const int GlobalSizeData::LOCALINFO_HEIGHT = 140;
const QSize GlobalSizeData::AVATAR_LAB_SIZE = QSize(60, 60);
const QSize GlobalSizeData::USER_NAME_LAB_SIZE = QSize(90, 30);
const QSize GlobalSizeData::USER_IP_LAB_SIZE = QSize(170, 20);
const QSize GlobalSizeData::CHANGE_NAME_BTN_SIZE = QSize(25, 25);
const QSize GlobalSizeData::CHANGE_NAME_BTN_ICON = QSize(16, 16);
const QSize GlobalSizeData::OPEN_FOLDER_BTN_SIZE = QSize(25, 25);
const QSize GlobalSizeData::OPEN_FOLDER_BTN_ICON = QSize(16, 16);
const QSize GlobalSizeData::SEARCH_EDIT_SIZE = QSize(334, 36);

//xc-平板尺寸改变
const int GlobalSizeData::TITLEBAR_MAX_HEIGHT = 64;
const int GlobalSizeData::LOCALINFO_MAX_HEIGHT = 169;
const int GlobalSizeData::LOCALINFO_SEARCH_MAX_H_WIDEH = 503;  // 搜索横屏宽度
const int GlobalSizeData::LOCALINFO_SEARCH_MAX_V_WIDEH = 1040; // 搜索竖屏宽度
const int GlobalSizeData::LOCALINFO_SEARCH_MAX_HEIGHT = 40;    // 搜索竖屏宽度
const QSize GlobalSizeData::LOCALINFO_BTN_MAX_SIZE = QSize(48, 48);

// 好友列表
const int GlobalSizeData::FRIEND_LIST_HEIGHT = 506;

// 所选消息框坐标及尺寸
QPoint GlobalSizeData::BORDER_POINT = QPoint(0, 0);
QSize GlobalSizeData::BORDER_SIZE = QSize(0, 0);

// 主题
QString GlobalSizeData::LIGHT_THEME = "ukui-default";
QString GlobalSizeData::BLACK_THEME = "ukui-dark";


void GlobalSizeData::SDKPointBatchDelete()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgBatchDelete)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointChangeDir()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgChangeDir)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointCleanCache()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgCleanCache)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointClearChatRecord()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgClearChatRecord)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointClearSingleChatRecord()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgClearSingleChatRecord)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointCopy()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgCopy)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointDeleteRecord()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgDeleteRecord)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointHistorySearch()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgHistorySearch)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointMainSearch()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgMainSearch)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointModifyFriendNotes()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgModifyFriendNotes)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointNicknameModify()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgNicknameModify)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointOpen()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgOpen)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointOpenDir()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgOpenDir)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointOpenSaveDir()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgOpenSaveDir)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointResend()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgResend)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointSaveAs()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgSaveAs)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointSendDir()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgSendDir)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointSendFiles()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgSendFiles)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointSendMessage()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgSendMessage)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointSendScreenshot()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgSendScreenshot)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointSetTop()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgSetTop)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointTray()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgTray)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}

void GlobalSizeData::SDKPointViewInformation()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(::kabase::AppName::KylinIpmsg , ::kabase::BuriedPoint::PT::KylinIpmsgViewInformation)) {
        qCritical() << "Error : buried point fail !";
    };
    
    return;
}
