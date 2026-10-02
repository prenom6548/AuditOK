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

#ifndef TEST_CHATMSGDB_H
#define TEST_CHATMSGDB_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include "global/database/chatmsgdb.h"
#include "global/database/database.h"

class TestChatMsgDB : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    // Helper functions for testing
    ChatMsgInfo* createTestChatMsgInfo();
    void verifyChatMsgInfo(const ChatMsgInfo* actual, const ChatMsgInfo* expected);
    void testCoverageAdditions();
    
    // Test database setup
    QTemporaryDir tempDir;
    QString dbPath;
};

#endif // TEST_CHATMSGDB_H