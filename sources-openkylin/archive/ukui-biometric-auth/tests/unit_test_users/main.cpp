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
#include <gtest/gtest.h>
#include "../../polkit-agent/src/users.h"
#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDebug>
#include <QFile>
#include <QTemporaryDir>

class UsersTest : public ::testing::Test {
protected:
    void SetUp() override {
        users = new Users(nullptr);
    }

    void TearDown() override {
        delete users;
    }

    Users *users;
};

TEST_F(UsersTest, testGetUsers) {
    QList<UserItem> userList = users->getUsers();
    EXPECT_EQ(userList.size(), 0);  // Initially, the list should be empty

    // Load users should populate the list
    users->loadUsers();
    userList = users->getUsers();
    EXPECT_EQ(userList.size(), 2);  // After loading, we expect 2 users
    EXPECT_EQ(userList[0].name, QString("testuser"));
    EXPECT_EQ(userList[0].uid, quint64(1000));
    EXPECT_EQ(userList[1].name, QString(""));  // The second user might not be fully populated in this mock
}

TEST_F(UsersTest, testGetUserByName) {
    users->loadUsers();
    UserItem user = users->getUserByName("testuser");
    EXPECT_EQ(user.name, QString("testuser"));
    EXPECT_EQ(user.realName, QString("Test User"));
    EXPECT_EQ(user.uid, quint64(1000));

    user = users->getUserByName("nonexistent");
    EXPECT_EQ(user.name, QString("nonexistent"));
    EXPECT_EQ(user.uid, ::getuid());  // Fallback uid should be the current uid

    user = users->getUserByName("root");
    user = users->getUserByName("kylin");
    qDebug()<<user;
}

TEST_F(UsersTest, testGetDefaultIcon) {
    QString defaultIcon = users->getDefaultIcon();
    EXPECT_EQ(defaultIcon, QString(":/image/assets/iconFace.png"));
}

TEST_F(UsersTest, testOnUserAddedAndDeleted) {
    // Set up a temporary directory and file to test icon loading
    QTemporaryDir tempDir;
    QFile tempFile(tempDir.filePath("icon.png"));
    tempFile.open(QIODevice::WriteOnly);
    tempFile.close();

    // Load users
    users->loadUsers();
    QList<UserItem> userList = users->getUsers();
    EXPECT_EQ(userList.size(), 2);

    // Emit userAdded signal (simulate DBus signal)
    QDBusObjectPath path("/user/newuser");
    QMetaObject::invokeMethod(users, "onUserAdded", Q_ARG(QDBusObjectPath, path));
    userList = users->getUsers();
    EXPECT_EQ(userList.size(), 3);  // Now we should have 3 users

    // Emit userDeleted signal (simulate DBus signal)
    QMetaObject::invokeMethod(users, "onUserDeleted", Q_ARG(QDBusObjectPath, path));
    userList = users->getUsers();
    EXPECT_EQ(userList.size(), 2);  // Back to 2 users
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
