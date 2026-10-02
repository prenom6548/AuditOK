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

#include <gtest/gtest.h>
#include "chatmsgmodel.h"
#include "global/utils/globalutils.h"
#include <QStandardItem>
#include <QDateTime>

TEST(TestChatMsgModel, Constructor) {
    ChatMsgModel model(1, "test-uuid");
    EXPECT_EQ(model.rowCount(), 0);
}

TEST(TestChatMsgModel, RoleNames) {
    ChatMsgModel model(1, "test-uuid");
    QHash<int, QByteArray> roles = model.roleNames();
    EXPECT_TRUE(roles.contains(ChatMsgModel::MsgId));
    EXPECT_TRUE(roles.contains(ChatMsgModel::MsgContent));
}

TEST(TestChatMsgModel, CoverageAdditions) {
    ChatMsgModel model(123, "test-uuid");

    // 1. 覆盖 addTimeItem (lines 497-515)
    QString t1 = "2023-01-01 10:00:00";
    QString t2 = "2023-01-01 10:05:00"; // 相差超过120秒
    QStandardItem *timeItem1 = model.addTimeItem(t1);
    QStandardItem *timeItem2 = model.addTimeItem(t1, t2);
    EXPECT_NE(timeItem1, nullptr);

    // 2. 覆盖 updateMsgState (lines 339-350) - 空指针安全
    ChatMsgInfo info;
    info.m_msgId = 9999; // 不存在的ID
    info.m_sendState = ChatMsgModel::SendSucceed;
    model.updateMsgState(&info);

    // 3. 覆盖 updateTransferSize (lines 362-373) - 空指针安全
    info.m_totalSize = 1024;
    info.m_transferSize = 512;
    model.updateTransferSize(&info);

    // 4. 覆盖 updateDateTimeFormat (lines 413-418)
    model.updateDateTimeFormat("yyyy/MM/dd");

    // 5. 覆盖 setMsgChooseState (lines 528, 541) - 空指针安全
    ChatMsgInfo chooseInfo;
    chooseInfo.m_msgId = 9999; // 不存在的ID
    chooseInfo.m_chooseMsgState = ChatMsgModel::ChooseConfirm;
    model.setMsgChooseState(&chooseInfo);
}

