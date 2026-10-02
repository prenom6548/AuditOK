/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#ifndef TEST_TITLE_H
#define TEST_TITLE_H

#include <QObject>
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QPushButton>
#include "global/corevar.h"

// 前向声明title类
class title;

// 添加缺失的类声明
namespace kdk {
    class KSearchLineEdit;
}

class ViewVar;
class horscreen;
class menuModule;
class Interaction;
class CoreVar;

class TestTitle : public QObject
{
    Q_OBJECT

public:
    TestTitle();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试构造函数
    void testConstructor();

    // 测试槽函数
    void testSlotSearchLineEditClear();
    void testSlotChangeLineEditStyle();
    void testSlotChangeHScreenMode();
    void testSlotChangeVScreenMode();
    void testSlotChangePCMode();

    // 测试获取搜索行位置的方法
    void testGetSearchLinePositionX();
    void testGetSearchLinePositionY();

    // 测试事件过滤器
    void testEventFilter();

    // 测试键盘事件
    void testKeyPressEvent();

    // 测试发送搜索输入
    void testSendSearchInput();

    // 测试按钮点击连接
    void testButtonConnections();

private:
    title* m_title;
};

#endif // TEST_TITLE_H