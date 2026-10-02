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

#ifndef TEST_FRIENDINFOWID_H
#define TEST_FRIENDINFOWID_H

#include <QtTest>
#include <QObject>
#include <QApplication>
#include <QWidget>

// 包含被测类
#include "../../../../../src/view/friendlist/friendinfowid.h"

// 测试类
class TestFriendInfoWid : public QObject
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

    // 测试填充好友信息
    void testFillFriendInfo();

    // 测试字体改变
    void testChangeFontSize();

    // 测试槽函数
    void testSlots();

    // 测试透明度变化
    void testChangeTransparency();

    // 测试绘制事件
    void testPaintEvent();

    // 测试长名字填充
    void testFillFriendInfoLongNames();

    // 测试空昵称填充
    void testFillFriendInfoEmptyNickname();

    // 测试关闭事件
    void testCloseEvent();

    // 测试修改昵称
    void testChangeNickname();

    // 测试按键事件
    void testKeyPressEvent();

    // 在每个测试函数之后运行
    void cleanup();
    
    // 在所有测试函数之后运行
    void cleanupTestCase();

private:
    FriendInfoWid* m_wid;
};

#endif // TEST_FRIENDINFOWID_H
