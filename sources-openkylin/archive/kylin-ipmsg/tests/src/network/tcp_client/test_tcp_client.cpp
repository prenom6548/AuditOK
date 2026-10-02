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

#include "test_tcp_client.h"
#include <QtTest/QSignalSpy>

void TestTcpClient::SetUp()
{
    m_tcpClient = TcpClient::getInstance();
}

void TestTcpClient::TearDown()
{
    // Singleton, don't delete
}

TEST_F(TestTcpClient, SingletonInstance)
{
    ASSERT_NE(m_tcpClient, nullptr);
    ASSERT_EQ(m_tcpClient, TcpClient::getInstance());
}

// 测试 tran 函数 (链接已建立，发送文本消息)
TEST_F(TestTcpClient, TranExistingLinkTextMsg)
{
    ChatMsgInfo send;
    send.m_friendUuid = "test-uuid-1";
    send.m_msgType = ChatMsgModel::MessageType::TextMsg;

    // 预先在 GlobalData 中建立链接
    // 注意：tcpMaintainAdd 只能更新已存在的条目，所以先用 udpMaintainAdd 创建
    g_udpItem udpItem;
    udpItem.uuid = "test-uuid-1";
    udpItem.peerListenIp = "127.0.0.1";
    udpItem.peerListenPort = "12345";
    GlobalData::getInstance()->m_tcpLink.udpMaintainAdd(udpItem);

    g_tcpItem item;
    item.uuid = "test-uuid-1";
    item.module = new TcpModule("127.0.0.1", "12345", "test-uuid-1");
    GlobalData::getInstance()->m_tcpLink.tcpMaintainAdd(item);

    int result = m_tcpClient->tran(&send);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(item.module->m_msgTaskQueue.size(), 1);
    
    // 清理
    GlobalData::getInstance()->m_tcpLink.udpMaintainDelete("test-uuid-1");
}

// 测试 tran 函数 (链接已建立，发送文件消息)
TEST_F(TestTcpClient, TranExistingLinkFileMsg)
{
    ChatMsgInfo send;
    send.m_friendUuid = "test-uuid-2";
    send.m_msgType = ChatMsgModel::MessageType::FileMsg;

    g_udpItem udpItem;
    udpItem.uuid = "test-uuid-2";
    udpItem.peerListenIp = "127.0.0.1";
    udpItem.peerListenPort = "12345";
    GlobalData::getInstance()->m_tcpLink.udpMaintainAdd(udpItem);

    g_tcpItem item;
    item.uuid = "test-uuid-2";
    item.module = new TcpModule("127.0.0.1", "12345", "test-uuid-2");
    GlobalData::getInstance()->m_tcpLink.tcpMaintainAdd(item);

    int result = m_tcpClient->tran(&send);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(item.module->m_fileTaskQueue.size(), 1);

    GlobalData::getInstance()->m_tcpLink.udpMaintainDelete("test-uuid-2");
}

// 测试 tran 函数 (链接未建立，新建链接并启动线程)
TEST_F(TestTcpClient, TranNewLink)
{
    ChatMsgInfo send;
    send.m_friendUuid = "test-uuid-3";
    send.m_msgType = ChatMsgModel::MessageType::TextMsg;

    // 确保没有现有链接，但需要有对端 IP/Port 信息才能触发新建
    GlobalData::getInstance()->m_tcpLink.udpMaintainDelete("test-uuid-3");
    g_udpItem udpItem;
    udpItem.uuid = "test-uuid-3";
    udpItem.peerListenIp = "127.0.0.1";
    udpItem.peerListenPort = "12345";
    GlobalData::getInstance()->m_tcpLink.udpMaintainAdd(udpItem);

    int result = m_tcpClient->tran(&send);
    EXPECT_EQ(result, 0);

    // 验证是否已添加到维护表
    g_tcpMaintain maintained = GlobalData::getInstance()->m_tcpLink.select("test-uuid-3");
    EXPECT_NE(maintained.module, nullptr);
    
    // 清理
    GlobalData::getInstance()->m_tcpLink.udpMaintainDelete("test-uuid-3");
}

// 测试取消发送功能
TEST_F(TestTcpClient, SendMsgCancelTest)
{
    QString uuid = "test-uuid-cancel";
    int msgId = 1001;

    g_udpItem udpItem;
    udpItem.uuid = uuid;
    udpItem.peerListenIp = "127.0.0.1";
    udpItem.peerListenPort = "12345";
    GlobalData::getInstance()->m_tcpLink.udpMaintainAdd(udpItem);

    g_tcpItem item;
    item.uuid = uuid;
    item.module = new TcpModule("127.0.0.1", "12345", uuid);
    GlobalData::getInstance()->m_tcpLink.tcpMaintainAdd(item);

    m_tcpClient->sendMsgCancel(uuid, msgId);
    EXPECT_TRUE(item.module->m_cancelTask.contains(msgId));

    GlobalData::getInstance()->m_tcpLink.udpMaintainDelete(uuid);
}

// 测试各种槽函数触发信号
TEST_F(TestTcpClient, SlotSignalsTest)
{
    ChatMsgInfo info;
    // 注册元类型以支持 QSignalSpy 处理指针
    qRegisterMetaType<ChatMsgInfo*>("ChatMsgInfo*");

    QSignalSpy spySuccess(m_tcpClient, SIGNAL(sigTcpClientSendMsgSuccess(ChatMsgInfo*)));
    QSignalSpy spyRecv(m_tcpClient, SIGNAL(sigTcpClientRecvMsgSuccess(ChatMsgInfo*)));
    QSignalSpy spyProgress(m_tcpClient, SIGNAL(sigTcpClientSendFileProgress(ChatMsgInfo*)));
    QSignalSpy spyFileSuccess(m_tcpClient, SIGNAL(sigTcpClientSendFileSuccess(ChatMsgInfo*)));
    QSignalSpy spyRecvFile(m_tcpClient, SIGNAL(sigTcpClientRecvFileSuccess(ChatMsgInfo*)));

    // 手动调用槽函数，验证信号发射
    QMetaObject::invokeMethod(m_tcpClient, "slotSendMsgSuccess", Q_ARG(ChatMsgInfo*, &info));
    QMetaObject::invokeMethod(m_tcpClient, "slotRecvMsgSuccess", Q_ARG(ChatMsgInfo*, &info));
    QMetaObject::invokeMethod(m_tcpClient, "slotSendFileProgress", Q_ARG(ChatMsgInfo*, &info));
    QMetaObject::invokeMethod(m_tcpClient, "slotSendFileSuccess", Q_ARG(ChatMsgInfo*, &info));
    QMetaObject::invokeMethod(m_tcpClient, "slotRecvFileSuccess", Q_ARG(ChatMsgInfo*, &info));

    EXPECT_EQ(spySuccess.count(), 1);
    EXPECT_EQ(spyRecv.count(), 1);
    EXPECT_EQ(spyProgress.count(), 1);
    EXPECT_EQ(spyFileSuccess.count(), 1);
    EXPECT_EQ(spyRecvFile.count(), 1);
}

// 测试 slotMaintainTcpLinkDelete
TEST_F(TestTcpClient, SlotMaintainTcpLinkDeleteTest)
{
    QString uuid = "test-uuid-delete";
    
    g_udpItem udpItem;
    udpItem.uuid = uuid;
    udpItem.peerListenIp = "127.0.0.1";
    udpItem.peerListenPort = "12345";
    GlobalData::getInstance()->m_tcpLink.udpMaintainAdd(udpItem);

    g_tcpItem item;
    item.uuid = uuid;
    item.module = new TcpModule("127.0.0.1", "12345", uuid);
    GlobalData::getInstance()->m_tcpLink.tcpMaintainAdd(item);

    // 验证添加成功
    EXPECT_NE(GlobalData::getInstance()->m_tcpLink.select(uuid).module, nullptr);

    // 调用删除槽
    QMetaObject::invokeMethod(m_tcpClient, "slotMaintainTcpLinkDelete", Q_ARG(QString, uuid));

    // 验证已删除 (udpMaintainDelete 会真正从 list 中移除，而 slotMaintainTcpLinkDelete 只是置空 module)
    // 根据代码，slotMaintainTcpLinkDelete 调用的是 GlobalData::getInstance()->m_tcpLink.tcpMaintainDelete(uuid)
    // 该函数只是将 module 置 NULL 并没有从 list 移除
    EXPECT_EQ(GlobalData::getInstance()->m_tcpLink.select(uuid).module, nullptr);

    GlobalData::getInstance()->m_tcpLink.udpMaintainDelete(uuid);
}
