/*
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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
#include <gtest/gtest-death-test.h>
#include <QCommandLineParser>
#include <QDebug>

#include "../../src/lock-dialog/lockdialogmodel.h"
#include "../../src/common/definetypes.h"
#include "../../src/agreementinfo.h"
#include "stubext.h"

using namespace stub_ext;

// 测试夹具类，用于初始化和清理测试环境
class LockDialogModelTest : public ::testing::Test {
protected:
    LockDialogModel* model;

    virtual void SetUp() {
        model = new LockDialogModel(nullptr);
    }

    virtual void TearDown() {
        delete model;
    }
};

TEST_F(LockDialogModelTest, ParseCmdArguments) {
    QStringList args;
    LockDialogModel::CommandLineArgs cmdArgs;

    // 天翼云情况测试（模拟天翼云平台）
    args << "--lock-startup";
    setenv("HostCloundPlatform", "ctyun", 1);
    EXPECT_TRUE(model->parseCmdArguments(args, cmdArgs));
    unsetenv("HostCloundPlatform");

    // 非天翼云且参数不全情况测试
    args.clear();
//    EXPECT_FALSE(model->parseCmdArguments(args, cmdArgs));

    // 完整参数设置测试
    args << "--lock";
    EXPECT_TRUE(model->parseCmdArguments(args, cmdArgs));
    EXPECT_TRUE(cmdArgs.isLock);
}

TEST_F(LockDialogModelTest, FindUserByName) {

    // 查找已存在的用户
    UserInfoPtr result = model->findUserByName("testuser");
    EXPECT_NE(result, nullptr);

    // 查找不存在的用户
    result = model->findUserByName("nonexistentuser");
    EXPECT_EQ(result, nullptr);
}

TEST_F(LockDialogModelTest, GetScreensaver) {

    ScreenSaver* saver = model->getScreensaver();
    EXPECT_EQ(saver->mode, SAVER_RANDOM);
    delete saver;

    saver = model->getScreensaver();
    EXPECT_EQ(saver->mode, SAVER_IMAGE);
    delete saver;
}

