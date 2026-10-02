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

#ifndef TEST_FRIENDLIST_H
#define TEST_FRIENDLIST_H

#include <QtTest/QtTest>
#include <QObject>
#include <QApplication>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QListView>
#include <QList>
#include <QHash>

// 包含被测类
#include "../../../../../src/view/friendlist/friendlist.h"

// 测试类
class TestFriendListView : public QObject
{
    Q_OBJECT

private slots:
    // 在所有测试函数之前运行
    void initTestCase();
    
    // 在每个测试函数之前运行
    void init();
    
    // 测试单例
    void testSingleton();

    // 测试UI初始化
    void testUiInitialization();

    // 测试模型初始化
    void testModelInitialization();

    // 测试过滤功能
    void testFilterFriend();

    // 测试获取聊天框
    void testGetMsgWid();

    // 测试槽函数
    void testSlots();

    // 测试右键菜单
    void testShowContextMenu();

    // 测试获取模型
    void testGetFriendListModel();

    // 测试清除所有消息
    void testClearAllMessages();

    // 测试开始聊天
    void testStartChat();

    // 测试置顶
    void testSetTop();

    // 测试修改昵称
    void testChangeNickname();

    // 测试查看资料
    void testViewInfo();

    // 测试删除好友
    void testDelFriend();

    // 测试删除好友后的槽函数
    void testSlotDeleFriend();

    // 在每个测试函数之后运行
    void cleanup();
    
    // 在所有测试函数之后运行
    void cleanupTestCase();

private:
    FriendListView* m_view;
};

#endif // TEST_FRIENDLIST_H
