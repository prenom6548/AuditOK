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
#include "../../ukui-greeter/greeter/usersmodel.h"
#include <gtest/gtest.h>
#include <QLightDM/UsersModel>
#include <QStandardItemModel>


class UsersModelTest : public ::testing::Test {
protected:
    UsersModel* usersModel;


    void SetUp() override {
        usersModel = new UsersModel(false);
    }


    void TearDown() override {
        delete usersModel;
    }
};


// 测试构造函数
TEST_F(UsersModelTest, ConstructorTest) {
    // 检查 hideUsers 为 false 的情况
    EXPECT_EQ(usersModel->showManualLogin(), true);

    // 检查 hideUsers 为 true 的情况
    UsersModel hiddenUsersModel(true);
    EXPECT_EQ(hiddenUsersModel.showManualLogin(), true);
}


// 测试 setShowGuest 函数
TEST_F(UsersModelTest, SetShowGuestTest) {
    // 测试显示 Guest
    usersModel->setShowGuest(true);

    EXPECT_EQ(usersModel->showGuest(), true);

    // 测试隐藏 Guest
    usersModel->setShowGuest(false);

    EXPECT_EQ(usersModel->showGuest(), false);

    // 测试多次调用
    usersModel->setShowGuest(true);
    usersModel->setShowGuest(true);

    EXPECT_EQ(usersModel->showGuest(), true);

    usersModel->setShowGuest(false);
    usersModel->setShowGuest(false);

    EXPECT_EQ(usersModel->showGuest(), false);
}


// 测试 showGuest 函数
TEST_F(UsersModelTest, ShowGuestTest) {
    usersModel->setShowGuest(true);

    EXPECT_EQ(usersModel->showGuest(), true);

    usersModel->setShowGuest(false);

    EXPECT_EQ(usersModel->showGuest(), false);
}


// 测试 setShowManualLogin 函数
TEST_F(UsersModelTest, SetShowManualLoginTest) {

    // 测试显示 Manual Login
    usersModel->setShowManualLogin(true);

    EXPECT_EQ(usersModel->showManualLogin(), true);

    // 测试隐藏 Manual Login
    usersModel->setShowManualLogin(false);

    EXPECT_EQ(usersModel->showManualLogin(), false);

    // 测试多次调用
    usersModel->setShowManualLogin(true);
    usersModel->setShowManualLogin(true);

    EXPECT_EQ(usersModel->showManualLogin(), true);

    usersModel->setShowManualLogin(false);
    usersModel->setShowManualLogin(false);

    EXPECT_EQ(usersModel->showManualLogin(), false);
}


// 测试 translateManualLogin 函数
TEST_F(UsersModelTest, TranslateManualLoginTest) {
    usersModel->setShowManualLogin(true);

    QStandardItemModel* extraModel = usersModel->extraRowModel();

    QStandardItem* manualLogin = extraModel->item(0, 0);

    // 确保初始 RealNameRole 未翻译
    EXPECT_EQ(manualLogin->data(QLightDM::UsersModel::RealNameRole).toString(), "Login");

    usersModel->translateManualLogin();

    // 确保 RealNameRole 已翻译
    EXPECT_EQ(manualLogin->data(QLightDM::UsersModel::RealNameRole).toString(), "Login");
}


// 测试 getSecUserCount 函数
TEST_F(UsersModelTest, GetSecUserCountTest) {
    // 模拟添加安全用户
    QStandardItem* secUserItem = new QStandardItem();
    secUserItem->setData("secUser", QLightDM::UsersModel::NameRole);

    QStandardItemModel* extraModel = usersModel->extraRowModel();

    extraModel->appendRow(secUserItem);

    // 假设 SecurityUser::isSecrityUser 对 "secUser" 认为是安全用户
    EXPECT_EQ(usersModel->getSecUserCount(), 1);

}


// 测试 showManualLogin 函数
TEST_F(UsersModelTest, ShowManualLoginTest) {
    usersModel->setShowManualLogin(true);

    EXPECT_EQ(usersModel->showManualLogin(), true);

    usersModel->setShowManualLogin(false);

    EXPECT_EQ(usersModel->showManualLogin(), false);
}
