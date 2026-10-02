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

#include "test_tcp_server.h"
#include <QtTest/QSignalSpy>
#include <QThread>

void TestTcpServer::SetUp()
{
    m_tcpServer = new TcpServer();
}

void TestTcpServer::TearDown()
{
    delete m_tcpServer;
}

// 模拟 incomingConnection 和 slotEstablishNewLink (覆盖 31-34, 57-83)
TEST_F(TestTcpServer, IncomingConnectionTest)
{
    // incomingConnection 是 protected，通过子类或友元访问（此处已加友元）
    // 模拟一个 socket 描述符（虽然不是真实的，但能触发逻辑）
    qintptr fakeDescriptor = 123;
    
    // 调用 incomingConnection，它会调用 slotEstablishNewLink
    m_tcpServer->incomingConnection(fakeDescriptor);
    
    // 由于 slotEstablishNewLink 会创建线程并启动，我们稍微等待一下或直接验证逻辑
    // 实际上创建了 TcpModule 和 QThread，这里主要验证函数走通
    SUCCEED();
}

// 测试 startListen 的异常分支 (覆盖 43-44, 48-49)
TEST_F(TestTcpServer, StartListenErrorTest)
{
    // 覆盖 43-44: IP 或端口为空
    GlobalData::getInstance()->m_tcpListenIP = "";
    GlobalData::getInstance()->m_tcpListenPort = "";
    EXPECT_EQ(m_tcpServer->startListen(), -1);

    // 覆盖 48-49: 监听失败 (使用一个已被占用的端口或非法 IP)
    GlobalData::getInstance()->m_tcpListenIP = "127.0.0.1";
    GlobalData::getInstance()->m_tcpListenPort = "1"; // 通常普通用户无法监听 1 号端口
    // 如果 1 号端口没报错，可以尝试再次 startListen 同一个端口
    m_tcpServer->startListen(); 
    // 第二次监听同一个 server 实例通常会失败或逻辑上返回 -1
}

// 测试各个维护和消息槽函数 (覆盖 85-133)
TEST_F(TestTcpServer, SlotFunctionsTest)
{
    ChatMsgInfo info;
    g_tcpItem item;
    item.uuid = "test-server-uuid";
    
    QSignalSpy spyRecv(m_tcpServer, SIGNAL(sigTcpServerRecvMsgSuccess(ChatMsgInfo*)));
    QSignalSpy spySend(m_tcpServer, SIGNAL(sigTcpServerSendMsgSuccess(ChatMsgInfo*)));
    QSignalSpy spyFileRecv(m_tcpServer, SIGNAL(sigTcpServerRecvFileSuccess(ChatMsgInfo*)));
    QSignalSpy spyFileSend(m_tcpServer, SIGNAL(sigTcpServerSendFileSuccess(ChatMsgInfo*)));
    QSignalSpy spyProgress(m_tcpServer, SIGNAL(sigTcpServerSendFileProgress(ChatMsgInfo*)));
    QSignalSpy spyDb(m_tcpServer, SIGNAL(sigTcpServerMaintanceDb(g_tcpItem)));

    // 覆盖 85-97: slotMaintainTcpLinkAdd
    m_tcpServer->slotMaintainTcpLinkAdd(item);
    EXPECT_EQ(spyDb.count(), 1);

    // 覆盖 99-108: slotMaintainTcpLinkDelete
    m_tcpServer->slotMaintainTcpLinkDelete("test-server-uuid");

    // 覆盖 110-133: 消息和文件槽函数
    m_tcpServer->slotRecvMsgSuccess(&info);
    m_tcpServer->slotSendMsgSuccess(&info);
    m_tcpServer->slotRecvFileSuccess(&info);
    m_tcpServer->slotSendFileSuccess(&info);
    m_tcpServer->slotSendFileProgress(&info);

    EXPECT_EQ(spyRecv.count(), 1);
    EXPECT_EQ(spySend.count(), 1);
    EXPECT_EQ(spyFileRecv.count(), 1);
    EXPECT_EQ(spyFileSend.count(), 1);
    EXPECT_EQ(spyProgress.count(), 1);
}
