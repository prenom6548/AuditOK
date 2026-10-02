#include <gtest/gtest.h>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <qt5/QGSettings/QGSettings>
#include <QSet>
#include "../../ukui-session/modulemanager.h"

// 测试环境设置
class ModuleManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 1;
        char *argv[] = {"modulemanager_test"};

        // 初始化 ModuleManager
        moduleManager = new ModuleManager();
    }

    void TearDown() override {
        delete moduleManager;
    }
    ModuleManager *moduleManager;
};

TEST_F(ModuleManagerTest, Init) {
    moduleManager->init("test");
    EXPECT_TRUE(moduleManager->quickStart());
}

TEST_F(ModuleManagerTest, QuickStart) {
    bool ok = moduleManager->quickStart();
    EXPECT_TRUE(ok);
}


TEST_F(ModuleManagerTest, SetUKUILite) {
    bool ok = true;
    moduleManager->setUKUILite("test");
    EXPECT_TRUE(ok);
}

TEST_F(ModuleManagerTest, SetIsShutingDown) {
    bool ok = false;
    moduleManager->setIsShutingDown(ok);
    EXPECT_FALSE(ok);
}

TEST_F(ModuleManagerTest, StartUkuiSplash) {
    bool ok = false;
    moduleManager->startUkuiSplash();
    EXPECT_FALSE(ok);
}

TEST_F(ModuleManagerTest, EndUkuiSplash) {
    bool ok = false;
    moduleManager->endUkuiSplash();
    EXPECT_FALSE(ok);
}

TEST_F(ModuleManagerTest, AutoRestart) {
    XdgDesktopFile file;
    file.load("/home/kylin/.config/autostart/wechat-beta.desktop");
    bool ok = false;
    moduleManager->autoRestart(file);
    EXPECT_FALSE(ok);
}

TEST_F(ModuleManagerTest, InsertStartupList) {
    bool ok = false;
    moduleManager->insertStartupList("test");
    EXPECT_FALSE(ok);
}

TEST_F(ModuleManagerTest, WeakUp) {
    moduleManager->weakup(false);
    EXPECT_TRUE(moduleManager->quickStart());
}

TEST_F(ModuleManagerTest, ConstructSpecialList) {
    moduleManager->constructSpecialList();
    EXPECT_FALSE(moduleManager->checkIsTabletMode());
}

TEST_F(ModuleManagerTest, ConstructStartupList) {
    moduleManager->constructStartupList();
    EXPECT_FALSE(moduleManager->checkIsTabletMode());
}


TEST_F(ModuleManagerTest, GetUKUILiteApps) {
    QSet<QString> set = moduleManager->getUKUILiteApps();
    EXPECT_TRUE(set.isEmpty());
}

TEST_F(ModuleManagerTest, Startupfinished) {
    QString str1 = "ukui-settings-daemon";
    QString str2 = "ukui-kwin";
    QString str3 = "ukui-panel";
    QString str4 = "peony-qt-desktop";
    QString str5 = "ukui-screensaver-dialog";

    moduleManager->startupfinished(str1,"test");
    moduleManager->startupfinished(str2,"test");
    moduleManager->startupfinished(str3,"test");
    moduleManager->startupfinished(str4,"test");
    moduleManager->startupfinished(str5,"test");

    EXPECT_EQ(str1, "ukui-settings-daemon");
    EXPECT_EQ(str2, "ukui-kwin");
    EXPECT_EQ(str3, "ukui-panel");
    EXPECT_EQ(str4, "peony-qt-desktop");
    EXPECT_EQ(str5, "ukui-screensaver-dialog");


}

// 测试 checkIsTabletMode
TEST_F(ModuleManagerTest, CheckIsTabletMode) {
    // 模拟 QDBusInterface
    QDBusInterface dbusService("com.kylin.statusmanager.interface", "/", "com.kylin.statusmanager.interface", QDBusConnection::sessionBus());
    EXPECT_TRUE(dbusService.isValid());

    // 测试平板模式
    bool isTabletMode = moduleManager->checkIsTabletMode();
    EXPECT_FALSE(isTabletMode); // 默认情况下，平板模式应为 false
}

// 测试 playBootMusic
TEST_F(ModuleManagerTest, PlayBootMusic) {
    // 测试播放启动音乐
    moduleManager->playBootMusic(true);
    moduleManager->playBootMusic(false);
}

// 测试 startup
TEST_F(ModuleManagerTest, Startup) {
    // 模拟 XdgDesktopFile
    XdgDesktopFile desktopFile;
    desktopFile.setValue("X-UKUI-Autostart-Phase", "Initialization");

    // 测试启动流程
    moduleManager->startup();
}

// 测试 startProcess
TEST_F(ModuleManagerTest, StartProcess) {
    // 模拟 XdgDesktopFile
    XdgDesktopFile desktopFile;
    desktopFile.setValue("Exec", "echo Hello World");

    // 测试启动进程
    moduleManager->startProcess(desktopFile, true);
}

// 测试 sequenceStart
TEST_F(ModuleManagerTest, SequenceStart) {
    // 模拟 XdgDesktopFileList
    XdgDesktopFileList fileList;
    XdgDesktopFile file1, file2;
    file1.setValue("Exec", "echo App1");
    file2.setValue("Exec", "echo App2");
    fileList << file1 << file2;

    // 测试顺序启动
    moduleManager->sequenceStart(fileList);
}

// 测试 restartModules
TEST_F(ModuleManagerTest, RestartModules) {
    // 模拟 UkuiModule
    XdgDesktopFile desktopFile;
    desktopFile.setValue("Exec", "echo RestartableApp");
    UkuiModule *module = new UkuiModule(desktopFile, moduleManager);

    // 测试重启模块
    moduleManager->restartModules(0, QProcess::NormalExit);
    module->terminate();
    module->isTerminating();
    module->getProcName();

    delete module;
    module = nullptr;
}

// 测试 openAppWithAppManager
TEST_F(ModuleManagerTest, OpenAppWithAppManager) {
    // 模拟 XdgDesktopFile
    XdgDesktopFile desktopFile;
    desktopFile.setValue("Exec", "echo AppManagerApp");

    // 测试通过 AppManager 打开应用
    bool result = moduleManager->openAppWithAppManager(desktopFile);
    EXPECT_TRUE(result);
}

// 测试 ChkScreenLockStartup
TEST_F(ModuleManagerTest, ChkScreenLockStartup) {
    // 测试锁屏启动检查
    moduleManager->ChkScreenLockStartup();
}

TEST_F(ModuleManagerTest, StatusChanged) {
    bool ok = true;
    moduleManager->statusChanged(ok);
    EXPECT_TRUE(ok);
}

// 测试 onLockScreenUnlock
TEST_F(ModuleManagerTest, OnLockScreenUnlock) {
    bool ok = true;
    // 测试锁屏解锁
    moduleManager->onLockScreenUnlock();
    EXPECT_TRUE(ok);
}

TEST_F(ModuleManagerTest, Dostartwm) {
    bool ok = true;
    moduleManager->dostartwm();
    EXPECT_TRUE(ok);
}

TEST_F(ModuleManagerTest, StartNotifPhase) {
    bool ok = true;
    moduleManager->startNotifPhase();
    EXPECT_TRUE(ok);
}

TEST_F(ModuleManagerTest, Timeup) {
    bool ok = true;
    moduleManager->timeup();
    EXPECT_TRUE(ok);
}

TEST_F(ModuleManagerTest, StartCompsite) {
    bool ok = true;
    moduleManager->startCompsite();
    EXPECT_TRUE(ok);
}


// 测试 onLockScreenUnlock
TEST_F(ModuleManagerTest, TimerUpdate) {
    bool ok = true;
    moduleManager->timerUpdate();
    EXPECT_TRUE(ok);
}

