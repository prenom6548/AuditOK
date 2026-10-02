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

#ifndef TEST_CONTROL_H
#define TEST_CONTROL_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "controller/control.h"

class TestControl : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    Control* m_control;

    // 增加转发方法，供 TEST_F 使用
    TcpServer* getServer() { return m_control->m_server; }
    TcpClient* getClient() { return m_control->m_client; }
    UdpSocket* getUdpSocket() { return m_control->m_udpSocket; }
    QThread* getThread() { return m_control->m_thread; }

    void triggerSlotSend(ChatMsgInfo *send) { m_control->slotSend(send); }
    void triggerSlotFriendOffline(int id) { m_control->slotFriendOffline(id); }
    void triggerSlotMaintanceDb(g_tcpItem item) { m_control->slotMaintanceDb(item); }
    void triggerSlotConnentNewChatMsg(ChatMsg* msg) { m_control->slotConnentNewChatMsg(msg); }
    void triggerSlotCancel(QString friendUuid, int msgId) { m_control->slotCancel(friendUuid, msgId); }
    void triggerSlotRecvMsgSuccess(ChatMsgInfo *recv) { m_control->slotRecvMsgSuccess(recv); }
    void triggerSlotSendMsgSuccess(ChatMsgInfo *send) { m_control->slotSendMsgSuccess(send); }
    void triggerSlotSendMsgFailed(ChatMsgInfo *send) { m_control->slotSendMsgFailed(send); }
    void triggerSlotSendFileProgress(ChatMsgInfo *send) { m_control->slotSendFileProgress(send); }
    void triggerSlotSendFileSuccess(ChatMsgInfo *send) { m_control->slotSendFileSuccess(send); }
    void triggerSlotRecvFileSuccess(ChatMsgInfo *recv) { m_control->slotRecvFileSuccess(recv); }
    void testCoverageAdditions();
};

#endif // TEST_CONTROL_H
