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
// usdblockshortcut_test.cpp

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include "../../polkit-agent/src/usdblockshortcut.h"

// Test fixture for SessionManager
class USDBlockShortCutTest : public ::testing::Test {
protected:
    void SetUp() override {
        usdblockShortCut = new USDBlockShortCut();
    }

    void TearDown() override {
        delete usdblockShortCut;
    }

    USDBlockShortCut *usdblockShortCut;
};

TEST_F(USDBlockShortCutTest, testUSDBlockShortCutTest) {
    EXPECT_TRUE(usdblockShortCut != nullptr);
    usdblockShortCut->blockShortcuts(); 
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
