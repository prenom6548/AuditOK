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

#include "test_control.h"
#include <gsettingmonitor.h>
#include <QDir>
#include <QFile>
#include <QStandardItem>
#include "model/friendlistmodel.h"

// Counters/toggles from stub_all.cpp
extern int g_test_tcpclient_tran_calls;
extern int g_test_tcpclient_send_cancel_calls;
extern int g_test_chatmsg_send_success_calls;
extern int g_test_chatmsg_send_failed_calls;
extern int g_test_chatmsg_recv_success_calls;
extern int g_test_chatmsgmodel_add_calls;
extern int g_test_chatmsgmodel_update_state_calls;
extern int g_test_chatmsgmodel_update_transfer_calls;
extern int g_test_friendlist_add_friend_calls;
extern int g_test_friendlist_get_id_by_uuid_override;

void TestControl::SetUp()
{
    m_control = Control::getInstance();
    // reset counters/toggles
    g_test_tcpclient_tran_calls = 0;
    g_test_tcpclient_send_cancel_calls = 0;
    g_test_chatmsg_send_success_calls = 0;
    g_test_chatmsg_send_failed_calls = 0;
    g_test_chatmsg_recv_success_calls = 0;
    g_test_chatmsgmodel_add_calls = 0;
    g_test_chatmsgmodel_update_state_calls = 0;
    g_test_chatmsgmodel_update_transfer_calls = 0;
    g_test_friendlist_add_friend_calls = 0;
    g_test_friendlist_get_id_by_uuid_override = -1;
}

void TestControl::TearDown()
{
    // Singleton, don't delete
}

TEST_F(TestControl, SingletonInstance)
{
    ASSERT_NE(m_control, nullptr);
    ASSERT_EQ(m_control, Control::getInstance());
}

TEST_F(TestControl, InitializationTest)
{
    // Verify init runs without crashing
    m_control->init();
}

TEST_F(TestControl, GoodbyeTest)
{
    // Verify sayGoodbye runs without crashing
    m_control->sayGoodbye();
}

TEST_F(TestControl, PrivateMembersTest)
{
    // Now we can access private members through protected helper methods in the fixture
    ASSERT_NE(getServer(), nullptr);
    ASSERT_NE(getClient(), nullptr);
    ASSERT_NE(getUdpSocket(), nullptr);
    ASSERT_NE(getThread(), nullptr);
}

TEST_F(TestControl, SlotFriendOfflineTest)
{
    // Test private slot via helper
    triggerSlotFriendOffline(1);
}

TEST_F(TestControl, SlotMaintanceDbTest)
{
    g_tcpItem item;
    item.uuid = "test-uuid";
    item.peerIp = "127.0.0.1";
    item.peerPort = "12345";
    
    triggerSlotMaintanceDb(item);
}

TEST_F(TestControl, SlotConnentNewChatMsgTest)
{
    // ChatMsg is stubbed
    ChatMsg *chatMsg = new ChatMsg(1, "test-uuid");
    triggerSlotConnentNewChatMsg(chatMsg);
    delete chatMsg;
}

TEST_F(TestControl, ChangeFontTest)
{
    // Cover Control::changeFont
    m_control->changeFont();
}

TEST_F(TestControl, SlotSend_OnlineBranch)
{
    // Make friend online
    auto *item = FriendListModel::getInstance()->getFriendByUuid("uuid-online");
    item->setData(FriendListModel::OnlineType::Online, FriendListModel::OnlineState);

    auto *send = new ChatMsgInfo(1, 0, "uuid-online", 0, "", "hello", 0, "", 0, 0, 0, 0);
    triggerSlotSend(send);

    EXPECT_EQ(g_test_tcpclient_tran_calls, 1);
    delete send; // online branch does not delete send
}

TEST_F(TestControl, SlotSend_OfflineBranch)
{
    // Make friend offline -> should go to slotSendMsgFailed and delete send
    auto *item = FriendListModel::getInstance()->getFriendByUuid("uuid-offline");
    item->setData(FriendListModel::OnlineType::Offline, FriendListModel::OnlineState);

    auto *send = new ChatMsgInfo(2, 0, "uuid-offline", 0, "", "hello", 0, "", 0, 0, 0, 0);
    triggerSlotSend(send);

    EXPECT_EQ(g_test_chatmsg_send_failed_calls, 1);
    EXPECT_EQ(g_test_chatmsgmodel_update_state_calls, 1);
}

TEST_F(TestControl, SlotCancelTest)
{
    triggerSlotCancel("uuid-cancel", 42);
    EXPECT_EQ(g_test_tcpclient_send_cancel_calls, 1);
}

TEST_F(TestControl, SlotRecvMsgSuccessTest)
{
    auto *recv = new ChatMsgInfo(3, 0, "uuid-recv", 0, "", "recv-msg", 0, "", 0, 0, 0, 0);
    triggerSlotRecvMsgSuccess(recv);
    EXPECT_EQ(g_test_chatmsgmodel_add_calls, 1);
    EXPECT_EQ(g_test_chatmsg_recv_success_calls, 1);
}

TEST_F(TestControl, SlotSendMsgSuccessTest)
{
    auto *send = new ChatMsgInfo(4, 0, "uuid-send", 0, "", "send-msg", 0, "", 0, 0, 0, 0);
    triggerSlotSendMsgSuccess(send);
    EXPECT_EQ(g_test_chatmsgmodel_update_state_calls, 1);
    EXPECT_EQ(g_test_chatmsg_send_success_calls, 1);
}

TEST_F(TestControl, SlotSendFileProgressTest)
{
    auto *send = new ChatMsgInfo(5, 1, "uuid-file", 0, "", "", 0, "/tmp/a.txt", 0, 0, 100, 10);
    triggerSlotSendFileProgress(send);
    EXPECT_EQ(g_test_chatmsgmodel_update_transfer_calls, 1);
    EXPECT_EQ(g_test_chatmsg_send_success_calls, 1);
    delete send; // not deleted by this slot
}

TEST_F(TestControl, SlotSendFileSuccessTest)
{
    auto *send = new ChatMsgInfo(6, 1, "uuid-file", 0, "", "", 0, "/tmp/a.txt", 0, 0, 100, 10);
    triggerSlotSendFileSuccess(send);
    EXPECT_EQ(g_test_chatmsgmodel_update_transfer_calls, 1);
    EXPECT_EQ(g_test_chatmsgmodel_update_state_calls, 1);
    EXPECT_EQ(g_test_chatmsg_send_success_calls, 1);
    delete send; // not deleted by this slot
}

TEST_F(TestControl, SlotRecvFileSuccess_FileMsg)
{
    // Use FileMsg to avoid Dir size branch
    auto *recv = new ChatMsgInfo(7, 0, "uuid-recv-file", 0, "", "", ChatMsgModel::MessageType::FileMsg, "/tmp/a.txt", 0, 0, 0, 0);
    triggerSlotRecvFileSuccess(recv);
    EXPECT_EQ(g_test_chatmsgmodel_add_calls, 1);
}

TEST_F(TestControl, SlotMaintanceDb_ReturnBranch)
{
    // Force early return branch in slotMaintanceDb
    g_test_friendlist_get_id_by_uuid_override = 123;
    g_tcpItem item;
    item.uuid = "uuid-exists";
    item.peerIp = "127.0.0.1";
    item.peerPort = "12345";

    triggerSlotMaintanceDb(item);
    EXPECT_EQ(g_test_friendlist_add_friend_calls, 0);
}

TEST_F(TestControl, CoverageAdditions)
{
    // 1. 覆盖 141-144 (monitorFont lambda)
    emit kdk::GsettingMonitor::getInstance()->systemFontSizeChange();

    // 2. 覆盖 289-290 (slotRecvFileSuccess DirMsg branch)
    // 需要创建一个临时目录来让 getDirSize 有效执行
    QString tempDirPath = QDir::tempPath() + "/test_control_dir_coverage";
    QDir().mkpath(tempDirPath);
    QFile tempFile(tempDirPath + "/test.txt");
    if (tempFile.open(QIODevice::WriteOnly)) {
        tempFile.write("test data");
        tempFile.close();
    }

    auto *recv = new ChatMsgInfo();
    recv->m_friendUuid = "uuid-recv-dir";
    recv->m_msgType = ChatMsgModel::MessageType::DirMsg;
    recv->m_filePath = tempDirPath;
    
    // 确保 FriendListModel 能找到这个 UUID
    QStandardItem *item = new QStandardItem();
    item->setData(100, FriendListModel::Id);
    item->setData("uuid-recv-dir", FriendListModel::Uuid);
    FriendListModel::getInstance()->appendRow(item);
    FriendListModel::getInstance()->m_uuidItemsMap.insert("uuid-recv-dir", item);

    triggerSlotRecvFileSuccess(recv);
    
    // 清理
    QDir(tempDirPath).removeRecursively();
}
