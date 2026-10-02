/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 *
 */

#ifndef TEST_DESKTOPMANAGER_H
#define TEST_DESKTOPMANAGER_H

#include <QObject>

class TestDesktopManager : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    // 测试单例模式
    void testGetInstance();
    void testGetInstanceTwice();

    // 测试构造和析构
    void testConstructor();
    void testDestructor();

    // 测试基本功能
    void testGetCurrentDesktop();
    void testGetNumberOfDesktops();
    void testIsCurrentDesktopShowingDesktop();

    // 测试设置功能
    void testSetCurrentDesktop();
    void testAppendDesktop();
    void testSetDesktopName();

    // 测试移动和删除
    void testMoveDesktop();
    void testRemoveDesktop();

    // 测试几何信息
    void testGetFullScreenGeometry();

    // 测试输出UUID
    void testGetOutputUuid();

    // 测试右键菜单
    void testOpenDesktopMenu();

    // 测试边界情况
    void testInvalidDesktopIndex();

    // 测试信号连接
    void testSignalConnections();

    // 测试边缘情况
    void testEdgeCases();
};

#endif // TEST_DESKTOPMANAGER_H
