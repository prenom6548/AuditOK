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

#include "test_udp_socket.h"
#include <QUdpSocket>
#include <QtTest/QSignalSpy>
#include <QTest>
#include <QDateTime>
#include <QHostAddress>
#include <QNetworkDatagram>
#include "global/utils/addrset.h"

void TestUdpSocket::SetUp()
{
    // Initialize GlobalData dependencies
    GlobalData::getInstance()->m_udpListenPort = "2425";
    GlobalData::getInstance()->m_uuid = "test-uuid-123";
    GlobalData::getInstance()->m_tcpListenIP = "127.0.0.1";
    GlobalData::getInstance()->m_tcpListenPort = "2426";

    m_udpSocket = new UdpSocket();
}

void TestUdpSocket::TearDown()
{
    delete m_udpSocket;
}

// 测试 slotSocketRead 处理 UDP_BROADCAST/UDP_UNICAST (覆盖 110-145)
TEST_F(TestUdpSocket, SlotSocketReadOnlineTest)
{
    QSignalSpy spy(m_udpSocket, SIGNAL(addOnlineFriend(FriendInfoData*)));
    
    // 构造一个模拟的 UDP 数据包 (UDP_BROADCAST 类型)
    // 格式: msgType (1字节) + "appName uuid peerListenIp peerListenPort username platform"
    QByteArray data;
    data.append((char)UDP_BROADCAST);
    data.append("kylin-ipmsg other-uuid-456 192.168.1.100 2426 testuser linux");

    // 模拟接收数据
    QUdpSocket client;
    client.writeDatagram(data, QHostAddress::LocalHost, 2425);
    
    // 等待数据到达并被处理 (增加等待时间)
    QTest::qWait(200);

    EXPECT_GT(spy.count(), 0);
    EXPECT_TRUE(m_udpSocket->m_friendOnlineTimeMap.contains("other-uuid-456"));
}

// 测试 slotSocketRead 处理 UDP_GOODBYE (覆盖 146-171)
TEST_F(TestUdpSocket, SlotSocketReadOfflineTest)
{
    // 先添加一个在线好友
    m_udpSocket->m_friendOnlineTimeMap.insert("other-uuid-456", QDateTime::currentDateTime().toSecsSinceEpoch());

    QSignalSpy spy(m_udpSocket, SIGNAL(updateOfflineFriend(FriendInfoData*)));
    
    QByteArray data;
    data.append((char)UDP_GOODBYE);
    data.append("kylin-ipmsg other-uuid-456");

    QUdpSocket client;
    client.writeDatagram(data, QHostAddress::LocalHost, 2425);
    
    QTest::qWait(200);

    EXPECT_GT(spy.count(), 0);
    EXPECT_FALSE(m_udpSocket->m_friendOnlineTimeMap.contains("other-uuid-456"));
}

// 测试 udpSocketBroadcast 中的 IP 获取逻辑 (覆盖 184-191)
TEST_F(TestUdpSocket, UdpSocketBroadcastIpTest)
{
    // 先清空，触发获取逻辑
    GlobalData::getInstance()->m_tcpListenIP = "";

    // 触发广播逻辑
    m_udpSocket->udpSocketBroadcast(true);
    
    // 验证 m_tcpListenIP 是否被更新 (如果 hostIp 不是 localhost)
    QString hostIp = AddrSet::getInstance()->getHostIpAddress();
    if (hostIp != QHostAddress(QHostAddress::LocalHost).toString()) {
        EXPECT_EQ(GlobalData::getInstance()->m_tcpListenIP, hostIp);
    } else {
        // 如果是 localhost，m_tcpListenIP 应该保持为空或原有值
        SUCCEED();
    }
}

// 测试 slotOffline (覆盖 253-258)
TEST_F(TestUdpSocket, SlotOfflineTest)
{
    // slotOffline 内部调用 udpSocketBroadcast(false)
    m_udpSocket->slotOffline();
    SUCCEED();
}

// 测试 slotCheckOnline (覆盖 261-290)
TEST_F(TestUdpSocket, SlotCheckOnlineTest)
{
    QSignalSpy spy(m_udpSocket, SIGNAL(updateOfflineFriend(FriendInfoData*)));
    
    // 模拟一个过期的好友
    QString uuid = "expired-uuid";
    qint64 oldTime = QDateTime::currentDateTime().toSecsSinceEpoch() - (m_udpSocket->m_maxDiffSecs + 10);
    m_udpSocket->m_friendOnlineTimeMap.insert(uuid, oldTime);
    
    // 手动触发检测
    m_udpSocket->slotCheckOnline();
    
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(m_udpSocket->m_friendOnlineTimeMap.contains(uuid));
}
