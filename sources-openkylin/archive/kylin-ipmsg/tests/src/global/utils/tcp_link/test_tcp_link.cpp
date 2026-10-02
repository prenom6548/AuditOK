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

#include "test_tcp_link.h"
#include <QDebug>
#include <QSignalSpy>

void TestTcpLink::initTestCase()
{
}

void TestTcpLink::init()
{
    m_tcpLink = new TcpLink();
}

void TestTcpLink::cleanup()
{
    delete m_tcpLink;
}

void TestTcpLink::cleanupTestCase()
{
}

void TestTcpLink::testConstructorAndDestructor()
{
    TcpLink* link = new TcpLink();
    QVERIFY(link != nullptr);
    
    // 覆盖析构函数中的循环 (29-41行)
    g_tcpItem item;
    item.uuid = "destructor-test";
    item.module = nullptr; // simple_tcp_link 使用 void*
    link->tcpMaintainAdd(item);
    
    delete link; // 触发析构函数
}

void TestTcpLink::testInit()
{
    // init() is called in constructor
    QCOMPARE(m_tcpLink->init(), 0);
}

void TestTcpLink::testTcpMaintainAdd()
{
    g_tcpItem item;
    item.uuid = "test-uuid-1";
    item.selfIp = "127.0.0.1";
    item.selfPort = "12345";
    item.peerIp = "192.168.1.1";
    item.peerPort = "54321";
    item.module = nullptr;

    // Test adding new item
    QCOMPARE(m_tcpLink->tcpMaintainAdd(item), 0);

    // Test updating existing item
    item.selfPort = "11111";
    QCOMPARE(m_tcpLink->tcpMaintainAdd(item), 0);
    
    g_tcpMaintain result = m_tcpLink->select("test-uuid-1");
    QCOMPARE(result.selfPort, QString("11111"));
}

void TestTcpLink::testTcpMaintainDelete()
{
    g_tcpItem item;
    item.uuid = "test-uuid-delete";
    m_tcpLink->tcpMaintainAdd(item);

    // Test deleting existing item
    QCOMPARE(m_tcpLink->tcpMaintainDelete("test-uuid-delete"), 0);

    // Test deleting non-existing item
    QCOMPARE(m_tcpLink->tcpMaintainDelete("non-existing"), 0);
}

void TestTcpLink::testUdpMaintainAdd()
{
    g_udpItem item;
    item.uuid = "udp-uuid-1";
    item.peerListenIp = "10.0.0.1";
    item.peerListenPort = "8888";

    // Test adding new item (returns 1 for new)
    QCOMPARE(m_tcpLink->udpMaintainAdd(item), 1);

    // Test updating existing item (returns 0 for existing)
    // 覆盖 151-156 行 (更新已有项的 peerListenIp 和 peerListenPort)
    item.peerListenIp = "10.0.0.2";
    item.peerListenPort = "9999";
    QCOMPARE(m_tcpLink->udpMaintainAdd(item), 0);
    
    g_tcpMaintain result = m_tcpLink->select("udp-uuid-1");
    QCOMPARE(result.peerListenIp, QString("10.0.0.2"));
    QCOMPARE(result.peerListenPort, QString("9999"));
}

void TestTcpLink::testUdpMaintainDelete()
{
    g_udpItem item;
    item.uuid = "udp-uuid-delete";
    m_tcpLink->udpMaintainAdd(item);

    // Test deleting existing item
    QCOMPARE(m_tcpLink->udpMaintainDelete("udp-uuid-delete"), 0);

    // Test deleting non-existing item
    QCOMPARE(m_tcpLink->udpMaintainDelete("non-existing"), 0);
}

void TestTcpLink::testSelect()
{
    g_tcpItem item;
    item.uuid = "select-uuid";
    item.selfIp = "1.1.1.1";
    m_tcpLink->tcpMaintainAdd(item);

    // Test selecting existing
    g_tcpMaintain result = m_tcpLink->select("select-uuid");
    QCOMPARE(result.uuid, QString("select-uuid"));
    QCOMPARE(result.selfIp, QString("1.1.1.1"));

    // Test selecting non-existing
    g_tcpMaintain result2 = m_tcpLink->select("none");
    QVERIFY(result2.uuid.isEmpty());
}

void TestTcpLink::testErgodic()
{
    // Just call it to cover the code
    m_tcpLink->ergodic();
}
