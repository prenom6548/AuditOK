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
#ifndef TEST_ALLOWLOCATION_H
#define TEST_ALLOWLOCATION_H

#include <QObject>
#include <QtTest/QtTest>
#include "view/horscreen/allowlocation.h"

class TestAllowLocation : public QObject
{
    Q_OBJECT

public:
    TestAllowLocation();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // 测试构造函数
    void testConstructor();
    
    // 测试布局初始化
    void testInitLayout();
    
    // 测试信号连接
    void testInitConnect();
    
    // 测试拒绝定位功能
    void testRejectLocate();
    
    // 测试允许定位功能
    void testAllowLocate();
    
    // 测试信号发射
    void testSignals();

private:
    AllowLocation* m_allowLocation;
};

#endif // TEST_ALLOWLOCATION_H