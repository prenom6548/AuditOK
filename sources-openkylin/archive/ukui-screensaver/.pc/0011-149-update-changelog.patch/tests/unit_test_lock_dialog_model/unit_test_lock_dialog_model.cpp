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

