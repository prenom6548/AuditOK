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

#ifndef TEST_TCP_LINK_H
#define TEST_TCP_LINK_H

#include <QtTest/QtTest>
#include <QObject>
#include <QApplication>
#include <QSignalSpy>
#include <QString>
#include <QList>

#include "simple_tcp_link.h"

// 测试类
class TestTcpLink : public QObject
{
    Q_OBJECT

private slots:
    // 在所有测试函数之前运行
    void initTestCase();
    
    // 在每个测试函数之前运行
    void init();
    
    // 测试构造函数和析构函数
    void testConstructorAndDestructor();
    
    // 测试init函数
    void testInit();
    
    // 测试tcpMaintainAdd函数
    void testTcpMaintainAdd();
    
    // 测试tcpMaintainDelete函数
    void testTcpMaintainDelete();
    
    // 测试udpMaintainAdd函数
    void testUdpMaintainAdd();
    
    // 测试udpMaintainDelete函数
    void testUdpMaintainDelete();
    
    // 测试select函数
    void testSelect();
    
    // 测试ergodic函数
    void testErgodic();
    
    // 在每个测试函数之后运行
    void cleanup();
    
    // 在所有测试函数之后运行
    void cleanupTestCase();

private:
    TcpLink* m_tcpLink;
};

#endif // TEST_TCP_LINK_H