/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
// sessionmanager_test.cpp

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include "../../polkit-agent/src/sessionmanager.h"

// Test fixture for SessionManager
class SessionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        sessionManager = new SessionManager();
    }

    void TearDown() override {
        delete sessionManager;
    }

    SessionManager* sessionManager;
};

TEST_F(SessionManagerTest, testSessionManagerTest) {
    EXPECT_TRUE(sessionManager != nullptr);
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
