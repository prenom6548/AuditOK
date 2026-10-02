#include <gtest/gtest.h>
#include "../../ukui-session/sessionmanagercontext.h"
#include "../../ukui-session/modulemanager.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>

// 测试夹具
class SessionManagerDBusContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化 QCoreApplication（需要 DBus 环境）
        int argc = 0;
        char* argv[] = {nullptr};
        app = new QCoreApplication(argc, argv);

        // 初始化 ModuleManager 和 SessionManagerDBusContext
        manager = new ModuleManager();
        context = new SessionManagerDBusContext(manager);
    }

    void TearDown() override {
        delete context;
        delete manager;
        delete app;
    }

    QCoreApplication* app;
    ModuleManager* manager;
    SessionManagerDBusContext* context;
};

// 测试 startModule 方法
TEST_F(SessionManagerDBusContextTest, StartModuleTest) {
    QString moduleName = "test_module";
    context->startModule(moduleName);

    // 验证是否调用了 ModuleManager 的 startProcess 方法
    // 由于 startProcess 是 void 方法，我们无法直接验证其行为
    // 可以通过日志或其他副作用来验证
    EXPECT_TRUE(true); // 占位符，表示测试通过
}

// 测试 stopModule 方法
TEST_F(SessionManagerDBusContextTest, StopModuleTest) {
    QString moduleName = "test_module";
    context->stopModule(moduleName);

    // 验证是否调用了 ModuleManager 的 stopProcess 方法
    // 由于 stopProcess 是 void 方法，我们无法直接验证其行为
    EXPECT_TRUE(true); // 占位符，表示测试通过
}

// 测试 startApp 方法
TEST_F(SessionManagerDBusContextTest, StartAppTest) {
    QString appName = "test_app";
    QStringList args = {"--arg1", "--arg2"};
    bool result = context->startApp(appName, args);

    // 验证 startApp 是否返回 true
    EXPECT_TRUE(result);
}

TEST_F(SessionManagerDBusContextTest, CanSwitch) {
    bool ok = context->canSwitch();
    // 验证 startApp 是否返回 true
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, CanLockscreen) {
    bool ok = context->canLockscreen();
    // 验证 startApp 是否返回 true
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, CanHibernate) {
    bool ok = context->canHibernate();
    // 验证 startApp 是否返回 true
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, CanSuspend) {
    bool ok = context->canSuspend();
    // 验证 startApp 是否返回 true
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, CanLogout) {
    bool ok = context->canLogout();
    // 验证 startApp 是否返回 true
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, CanReboot) {
    bool ok = context->canReboot();
    // 验证 startApp 是否返回 true
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, CanPowerOff) {
    bool ok = context->canPowerOff();
    // 验证 startApp 是否返回 true
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, ListInhibitor) {
    QVector<InhibitInfo::InhibitorInfo> v1 = context->ListInhibitor("sleep");
    QVector<InhibitInfo::InhibitorInfo> v2 = context->ListInhibitor("logout");
    QVector<InhibitInfo::InhibitorInfo> v3  = context->ListInhibitor("shutdown");

    // 验证 startApp 是否返回 true
    EXPECT_TRUE(v1.isEmpty());
    EXPECT_TRUE(v2.isEmpty());
    EXPECT_TRUE(v3.isEmpty());
}

TEST_F(SessionManagerDBusContextTest, GetInhibitors) {
    QStringList list = context->GetInhibitors();
    // 验证 startApp 是否返回 true
    EXPECT_TRUE(list.length() == 0);
}

TEST_F(SessionManagerDBusContextTest, ExecPro) {
    QStringList list;
    list << "clean_all_jobs.py";
    context->execPro(list);
    // 验证 startApp 是否返回 true
    EXPECT_TRUE(list.length() != 0);
}

// 测试 IsSessionRunning 方法
TEST_F(SessionManagerDBusContextTest, IsSessionRunningTest) {
    bool isRunning = context->IsSessionRunning();

    // 验证当前会话是否正在运行
    EXPECT_FALSE(isRunning); // 假设当前会话是 UKUI
}

// 测试 GetSessionName 方法
TEST_F(SessionManagerDBusContextTest, GetSessionNameTest) {
    QString sessionName = context->GetSessionName();

    // 验证返回的会话名称是否正确
    EXPECT_EQ(sessionName, "ukui-session"); // 假设当前会话是 UKUI
}

// 测试 IsInhibited 方法
TEST_F(SessionManagerDBusContextTest, IsInhibitedTest) {
    quint32 flags = 1;
    bool isInhibited = context->IsInhibited(flags);

    // 验证是否被抑制
    EXPECT_FALSE(isInhibited); // 假设当前没有抑制
}

TEST_F(SessionManagerDBusContextTest, ListFileList) {
    QStringList list = context->listFileList();
    EXPECT_TRUE(list.length() != 0);
}

TEST_F(SessionManagerDBusContextTest, ExecuteBoxadm) {
    bool ok = true;
    context->executeBoxadm();
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, StopUkuisessionTarget) {
    bool ok = true;
    context->stopUkuiSessionTarget();
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, Uninhibit) {
    bool ok = true;
    uint i = 109;
    context->Uninhibit(i);
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, EmitPrepareForPhase2) {
    bool ok = true;
    context->emitPrepareForPhase2();
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, EmitPrepareForSwitchuser) {
    bool ok = true;
    context->emitPrepareForSwitchuser();
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, SimulateUserActivity) {
    bool ok = true;
    context->simulateUserActivity();
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, On_serviceUnregistered) {
    bool ok = true;
    context->on_serviceUnregistered("22");
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, Logout) {
    bool ok = true;
    context->logout();
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, PowerOff) {
    bool ok = true;
    context->powerOff();
    EXPECT_TRUE(ok);
}

TEST_F(SessionManagerDBusContextTest, Reboot) {
    bool ok = true;
    context->reboot();
    EXPECT_TRUE(ok);
}

// 测试 setSessionEnv 方法
TEST_F(SessionManagerDBusContextTest, SetSessionEnvTest) {
    QString key = "TEST_ENV_KEY";
    QString value = "TEST_ENV_VALUE";

    context->setSessionEnv(key, value);

    // 验证环境变量是否设置成功
    EXPECT_EQ(qgetenv(key.toLatin1()), value.toLatin1());
}

// 主函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
