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

#ifndef TEST_CHAT_SEARCH_H
#define TEST_CHAT_SEARCH_H

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QApplication>
#include <QTimer>
#include <QDir>

// 包含被测类的头文件
#include "../../../../../../src/view/chatsearch/chat_search.h"

// 测试类
class TestChatSearch : public QObject
{
    Q_OBJECT

public:
    explicit TestChatSearch(QObject *parent = nullptr);

private slots:
    // 在所有测试函数之前运行
    void initTestCase();
    
    // 在每个测试函数之前运行
    void init();
    
    // 测试构造函数
    void testConstructor();
    
    // 测试获取单例实例
    void testGetInstance();
    
    // 测试设置UI组件
    void testSetWidgetUi();
    
    // 测试设置组件样式
    void testSetWidgetStyle();
    
    // 测试设置信号连接
    void testSetSignalConn();
    
    // 测试获取焦点
    void testGetFocus();
    
    // 测试滚动条是否在底端
    void testIsScrollOnBottom();
    
    // 测试滚动条设为底端
    void testSetScrollToBottom();
    
    // 测试获取删除状态
    void testGetDeleteState();
    
    // 测试事件过滤器
    void testEventFilter();
    
    // 测试显示所有信息
    void testShowAllInfo();
    
    // 测试显示文件信息
    void testShowFileInfo();
    
    // 测试显示图片信息
    void testShowImageInfo();
    
    // 测试显示链接信息
    void testShowLinkInfo();
    
    // 测试过滤好友
    void testFilterFriendByReg();
    
    // 测试改变按钮样式
    void testChangeBtnStyle();
    
    // 测试改变主题
    void testChangeTheme();
    
    // 测试初始化GSetting
    void testInitGsetting();
    
    // 测试创建删除时复选框
    void testShowDeleMsgCheckbox();
    
    // 测试清除复选框图标
    void testClearCheckBoxIcon();
    
    // 测试记录选择删除消息
    void testRecordChooseDeleteMsg();
    
    // 测试点击消息改变状态
    void testClickMsgToChangeState();
    
    // 测试取消点击
    void testSlotCancelClick();
    
    // 测试确定点击
    void testSlotSureClick();
    
    // 测试选择消息
    void testSlotChooseMsg();
    
    // 测试删除菜单
    void testShowDeleMenu();
    
    void testShowContextMenu_TextMsg();
    void testShowContextMenu_TimeMsg();
    void testShowContextMenu_OtherMsg();
    
    // 测试检查文件存在
    void testCheckFileExist();
    
    // 测试打开文件
    void testSlotOpenFile();
    
    // 测试打开目录
    void testSlotOpenDir();
    
    // 测试删除消息
    void testSlotDeleteMsg();
    
    // 测试清空消息
    void testSlotClearMsg();
    
    // 测试显示选择消息
    void testShowChooseMsg();
    
    // 测试改变按钮样式（重载版本）
    void testChangeBtnStyleOverload();
    
    // 测试改变尺寸H屏幕模式
    void testChangeSizeHScreenMode();
    
    // 测试改变尺寸V屏幕模式
    void testChangeSizeVScreenMode();
    
    // 测试改变尺寸PC模式
    void testChangeSizePCMode();
    
    // 测试按键事件
    void testKeyPressEvent();
    
    // 测试关闭事件
    void testCloseEvent();
    
    // 测试滚动条移动
    void testOnSliderMoved();
    
    // 测试字体变化
    void testFontChange();
    
    // 在每个测试函数之后运行
    void cleanup();
    
    // 在所有测试函数之后运行
    void cleanupTestCase();

private:
    ChatSearch *m_chatSearch;
    ChatMsg *m_mockChatMsg;
};

#endif // TEST_CHAT_SEARCH_H