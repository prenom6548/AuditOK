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

#ifndef TEST_FRIENDLISTDB_H
#define TEST_FRIENDLISTDB_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "friendlistdb.h"

class MockFriendInfoData : public FriendInfoData {
public:
    MockFriendInfoData() : FriendInfoData() {}
    
    MockFriendInfoData(int friendId, QString friendUuid, QString friendIp, int friendPort,
                      QString friendMac, QString username, QString nickname, QString system,
                      QString platform, QString avatarUrl, QString recentMsgCont,
                      QString recentMsgTime, int unreadMsgNum, int onlineState, int priority,
                      int display) : 
        FriendInfoData(friendId, friendUuid, friendIp, friendPort, friendMac, username, 
                      nickname, system, platform, avatarUrl, recentMsgCont, recentMsgTime, 
                      unreadMsgNum, onlineState, priority, display) {}
};

class TestFriendInfoDataBase : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    FriendInfoDataBase* dbInstance;
};

#endif // TEST_FRIENDLISTDB_H