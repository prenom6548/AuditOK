#include <gtest/gtest.h>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QVariant>
#include <QFile>
#include "../../tools/lockchecker.h"
#include "../../tools/loginedusers.h"
#include "../../ukui-session/ukuilockinfo.h"
#include "../../ukui-session/xdgdesktopfile.h"
#include "../../ukui-session/xdgdirs.h"

class LockCheckerTest : public testing::Test {
protected:
    static void SetUpTestSuite() {
        // 可在此添加全局的设置，例如初始化一些资源
    }


    static void TearDownTestSuite() {
        // 可在此进行全局的清理，例如释放资源
    }


    void SetUp() override {
        // 每个测试用例开始前创建 LockChecker 实例
        lockChecker = new LockChecker();
    }


    void TearDown() override {
        // 每个测试用例结束后删除 LockChecker 实例
        delete lockChecker;
        lockChecker = nullptr;
    }


    LockChecker* lockChecker;
};


// 测试 LockChecker 的构造函数
TEST_F(LockCheckerTest, ConstructorTest) {
    // 创建 LockChecker 实例
    LockChecker* checker = new LockChecker();
    // 这里只是简单创建实例，可根据实际需求添加更多检查
    delete checker;
}


// 测试 getLoginedUsers 方法
TEST_F(LockCheckerTest, GetLoginedUsersTest) {
    // 测试正常调用
    QStringList users = lockChecker->getLoginedUsers();
    EXPECT_TRUE(users.contains("kylin"));
}


// 测试 listInhibitor 方法
TEST_F(LockCheckerTest, ListInhibitorTest) {
    // 测试正常调用
    QVector<InhibitInfo::InhibitorInfo> inhibitors_logout = lockChecker->listInhibitor("logout");
    EXPECT_EQ(inhibitors_logout.size(), 0);

    QVector<InhibitInfo::InhibitorInfo> inhibitors_sleep = lockChecker->listInhibitor("sleep");
    EXPECT_EQ(inhibitors_sleep.size(), 0);

    QVector<InhibitInfo::InhibitorInfo> inhibitors_shutdown = lockChecker->listInhibitor("shutdown");
    EXPECT_EQ(inhibitors_shutdown.size(), 0);

    QVector<InhibitInfo::InhibitorInfo> inhibitors_false = lockChecker->listInhibitor("logout");
    EXPECT_EQ(inhibitors_false.size(), 0);

}

// 测试 getName 方法
TEST_F(LockCheckerTest, GetNameTest) {
    // 测试文件存在的情况
    QFile file("test_file");
    QString name = lockChecker->getName(&file);


    // 测试文件不存在的情况
    QFile nonExistentFile("non_existent_file");
    name = lockChecker->getName(&nonExistentFile);
}


// 测试 getInhibitorLocal 方法
TEST_F(LockCheckerTest, GetInhibitorLocalTest) {
    QVector<InhibitInfo::InhibitorInfo> inhibitVec;


    // 测试不同的 type 参数
    bool result = lockChecker->getInhibitorLocal(inhibitVec, "logout");
    EXPECT_FALSE(result);


    result = lockChecker->getInhibitorLocal(inhibitVec, "sleep");
    EXPECT_FALSE(result);


    result = lockChecker->getInhibitorLocal(inhibitVec, "shutdown");
    EXPECT_FALSE(result);
}


// 测试 getCachedUsers 方法
TEST_F(LockCheckerTest, GetCachedUsersTest) {
    // 测试正常调用
    int userNum = lockChecker->getCachedUsers();
    EXPECT_EQ(userNum,1);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
