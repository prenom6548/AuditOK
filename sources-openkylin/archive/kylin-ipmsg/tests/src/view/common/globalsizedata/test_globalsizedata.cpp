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

#include "buriedpoint.hpp"
#include "test_globalsizedata.h"
#include <QtTest>
#include <QObject>
#include <QApplication>
#include <QDebug>

// 测试初始化
void TestGlobalSizeData::initTestCase()
{
    // 在所有测试开始前执行一次
    qDebug() << "Starting GlobalSizeData tests...";
}

// 每个测试前的初始化
void TestGlobalSizeData::init()
{
    // GlobalSizeData 是一个静态类，不需要实例化
}

// 测试静态常量
void TestGlobalSizeData::testStaticConstants()
{
    // 测试主题色
    QVERIFY(GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight || 
            GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUIDark);
    
    // 测试毛玻璃透明度
    QCOMPARE(GlobalSizeData::BLUR_TRANSPARENCY, 150.0);
    
    // 测试主界面尺寸
    QCOMPARE(GlobalSizeData::WINDOW_WIDTH, 350);
    QCOMPARE(GlobalSizeData::WINDOW_HEIGHT, 686);
    
    // 测试标题栏高度
    QCOMPARE(GlobalSizeData::TITLEBAR_HEIGHT, 40);
    
    // 测试本机信息高度
    QCOMPARE(GlobalSizeData::LOCALINFO_HEIGHT, 140);
    
    // 测试平板尺寸
    QCOMPARE(GlobalSizeData::TITLEBAR_MAX_HEIGHT, 64);
    QCOMPARE(GlobalSizeData::LOCALINFO_MAX_HEIGHT, 169);
    
    // 测试好友列表高度
    QCOMPARE(GlobalSizeData::FRIEND_LIST_HEIGHT, 506);
    
    // 测试主题字符串
    QCOMPARE(GlobalSizeData::LIGHT_THEME, QString("ukui-default"));
    QCOMPARE(GlobalSizeData::BLACK_THEME, QString("ukui-dark"));

    // 测试其他静态常量
    QCOMPARE(GlobalSizeData::AVATAR_LAB_SIZE, QSize(60, 60));
    QCOMPARE(GlobalSizeData::USER_NAME_LAB_SIZE, QSize(90, 30));
    QCOMPARE(GlobalSizeData::USER_IP_LAB_SIZE, QSize(170, 20));
    QCOMPARE(GlobalSizeData::CHANGE_NAME_BTN_SIZE, QSize(25, 25));
    QCOMPARE(GlobalSizeData::CHANGE_NAME_BTN_ICON, QSize(16, 16));
    QCOMPARE(GlobalSizeData::OPEN_FOLDER_BTN_SIZE, QSize(25, 25));
    QCOMPARE(GlobalSizeData::OPEN_FOLDER_BTN_ICON, QSize(16, 16));
    QCOMPARE(GlobalSizeData::SEARCH_EDIT_SIZE, QSize(334, 36));
    QCOMPARE(GlobalSizeData::TITLEBAR_MAX_HEIGHT, 64);
    QCOMPARE(GlobalSizeData::LOCALINFO_MAX_HEIGHT, 169);
    QCOMPARE(GlobalSizeData::LOCALINFO_SEARCH_MAX_H_WIDEH, 503);
    QCOMPARE(GlobalSizeData::LOCALINFO_SEARCH_MAX_V_WIDEH, 1040);
    QCOMPARE(GlobalSizeData::LOCALINFO_SEARCH_MAX_HEIGHT, 40);
    QCOMPARE(GlobalSizeData::LOCALINFO_BTN_MAX_SIZE, QSize(48, 48));
    QCOMPARE(GlobalSizeData::BORDER_POINT, QPoint(0, 0));
    QCOMPARE(GlobalSizeData::BORDER_SIZE, QSize(0, 0));
}

// 测试主题颜色枚举
void TestGlobalSizeData::testThemeColorEnum()
{
    // 验证枚举值
    QCOMPARE(static_cast<int>(GlobalSizeData::UKUILight), 0);
    QCOMPARE(static_cast<int>(GlobalSizeData::UKUIDark), 1);
}

// 测试静态成员的可修改性（为了覆盖率，虽然是常量或静态变量）
void TestGlobalSizeData::testStaticMemberModification()
{
    int originalColor = GlobalSizeData::THEME_COLOR;
    GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUIDark;
    QCOMPARE(GlobalSizeData::THEME_COLOR, static_cast<int>(GlobalSizeData::UKUIDark));
    GlobalSizeData::THEME_COLOR = originalColor;

    double originalBlur = GlobalSizeData::BLUR_TRANSPARENCY;
    GlobalSizeData::BLUR_TRANSPARENCY = 200.0;
    QCOMPARE(GlobalSizeData::BLUR_TRANSPARENCY, 200.0);
    GlobalSizeData::BLUR_TRANSPARENCY = originalBlur;

    QPoint originalPoint = GlobalSizeData::BORDER_POINT;
    GlobalSizeData::BORDER_POINT = QPoint(10, 10);
    QCOMPARE(GlobalSizeData::BORDER_POINT, QPoint(10, 10));
    GlobalSizeData::BORDER_POINT = originalPoint;

    QSize originalSize = GlobalSizeData::BORDER_SIZE;
    GlobalSizeData::BORDER_SIZE = QSize(100, 100);
    QCOMPARE(GlobalSizeData::BORDER_SIZE, QSize(100, 100));
    GlobalSizeData::BORDER_SIZE = originalSize;
}

// 测试SDK打点函数
void TestGlobalSizeData::testSDKPointFunctions()
{
    // 测试所有SDK打点函数是否能正常调用而不崩溃
    // 这些函数内部调用了 kabase::BuriedPoint，在测试环境下应该能正常运行
    
    GlobalSizeData::SDKPointBatchDelete();
    GlobalSizeData::SDKPointChangeDir();
    GlobalSizeData::SDKPointCleanCache();
    GlobalSizeData::SDKPointClearChatRecord();
    GlobalSizeData::SDKPointClearSingleChatRecord();
    GlobalSizeData::SDKPointCopy();
    GlobalSizeData::SDKPointDeleteRecord();
    GlobalSizeData::SDKPointHistorySearch();
    GlobalSizeData::SDKPointMainSearch();
    GlobalSizeData::SDKPointModifyFriendNotes();
    GlobalSizeData::SDKPointNicknameModify();
    GlobalSizeData::SDKPointOpen();
    GlobalSizeData::SDKPointOpenDir();
    GlobalSizeData::SDKPointOpenSaveDir();
    GlobalSizeData::SDKPointResend();
    GlobalSizeData::SDKPointSaveAs();
    GlobalSizeData::SDKPointSendDir();
    GlobalSizeData::SDKPointSendFiles();
    GlobalSizeData::SDKPointSendMessage();
    GlobalSizeData::SDKPointSendScreenshot();
    GlobalSizeData::SDKPointSetTop();
    GlobalSizeData::SDKPointTray();
    GlobalSizeData::SDKPointViewInformation();
    
    QVERIFY(true);
}

// 每个测试后的清理
void TestGlobalSizeData::cleanup()
{
    // 不需要特殊清理
}

// 测试结束后的清理
void TestGlobalSizeData::cleanupTestCase()
{
    // 在所有测试结束后执行一次
    qDebug() << "Finished GlobalSizeData tests.";
}
