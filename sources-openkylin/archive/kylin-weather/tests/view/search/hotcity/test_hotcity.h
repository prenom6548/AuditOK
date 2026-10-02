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
#ifndef TEST_HOTCITY_H
#define TEST_HOTCITY_H

#include <QObject>
#include <QApplication>
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include "view/search/hotcity.h"

// 测试HotCity类
class TestHotCity : public QObject
{
    Q_OBJECT

public:
    TestHotCity(QObject *parent = nullptr);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试构造函数
    void testConstructor();
    
    // 测试initPostion方法
    void testInitPostion();
    
    // 测试initConnect方法
    void testInitConnect();
    
    // 测试信号
    void testSignals();

private:
    HotCity *m_hotCity;
};

#endif // TEST_HOTCITY_H