/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "../core/processinfomanager.h"
#include <QtTest/QtTest>
#include <memory>
#include <string>
#include <vector>

// 定义private为public以便访问私有成员
#define private public
#define protected public
#include "../base/appinfo.h"
#include "../base/schedpolicy.h"
#include "../core/appinfomanager.h"
#include "../core/configmanager.h"
#undef private
#undef protected

// 模拟AppInfoManager类
class MockAppInfoManager
{
public:
    using AppChangedCallback = std::function<void(const AppInfo &)>;

    static std::string lastWid;
    static std::string lastDesktopFile;
    static std::string lastService;
    static std::string lastAppId;
    static std::string lastGroupName;
    static std::string lastCmdline;
    static std::vector<std::string> lastArgs;
    static int lastPid;
    static int lastInterval;
    static sched_policy::AppState lastState;
    static sched_policy::AppType lastAppType;
    static AppInfo lastAppInfo;
    static bool lastResult;
    static AppChangedCallback appStartedCallback;
    static AppChangedCallback appStateChangedCallback;
    static AppChangedCallback appChildPidsUpdatedCallback;
    static std::vector<AppInfo> allAppInfos;
    static std::vector<AppInfo> appInfosWithState;

    static void reset()
    {
        lastWid.clear();
        lastDesktopFile.clear();
        lastService.clear();
        lastAppId.clear();
        lastGroupName.clear();
        lastCmdline.clear();
        lastArgs.clear();
        lastPid = 0;
        lastInterval = 0;
        lastState = sched_policy::AppState::Focus;
        lastAppType = sched_policy::AppType::Normal;
        lastResult = false;
        appStartedCallback = nullptr;
        appStateChangedCallback = nullptr;
        appChildPidsUpdatedCallback = nullptr;
        allAppInfos.clear();
        appInfosWithState.clear();
    }

    // 模拟AppInfoManager方法
    void handleWindowAdded(const std::string &wid)
    {
        lastWid = wid;
    }

    void handleWindowRemoved(const std::string &wid)
    {
        lastWid = wid;
    }

    void handleActiveWindowChanged(const std::string &wid)
    {
        lastWid = wid;
    }

    void handleWindowMinimized(const std::string &wid)
    {
        lastWid = wid;
    }

    void handleExceptionGroupSessionApp(const std::string &desktopFile, int pid)
    {
        lastDesktopFile = desktopFile;
        lastPid = pid;
    }

    void handleMprisDbusAdded(int pid, const std::string &service)
    {
        lastPid = pid;
        lastService = service;
    }

    void handleStatusItemNotifierItemRegistered(int pid)
    {
        lastPid = pid;
    }

    void createAppInstance(const std::string &desktopFile,
                           const std::vector<std::string> &args, int pid)
    {
        lastDesktopFile = desktopFile;
        lastArgs = args;
        lastPid = pid;
    }

    void setAppGroupName(const std::string &appId, const std::string &groupName)
    {
        lastAppId = appId;
        lastGroupName = groupName;
    }

    void forceChangCachedStateTo(sched_policy::AppState state)
    {
        lastState = state;
    }

    bool forceChangeAppStateByPid(int pid, sched_policy::AppState state)
    {
        lastPid = pid;
        lastState = state;
        return lastResult;
    }

    void setBackgroundToCachedStateInterval(int interval)
    {
        lastInterval = interval;
    }

    sched_policy::AppType getAppTypeByPid(int pid)
    {
        lastPid = pid;
        return lastAppType;
    }

    std::string syncGetDesktopFileByPid(int pid)
    {
        lastPid = pid;
        return lastDesktopFile;
    }

    std::vector<AppInfo> getAllAppInfos()
    {
        return allAppInfos;
    }

    std::vector<AppInfo> getAppInfosWithState(sched_policy::AppState appState)
    {
        lastState = appState;
        return appInfosWithState;
    }

    AppInfo getLatestAppInfoByDesktopFile(const std::string &desktopFile,
                                          const std::vector<std::string> &args)
    {
        lastDesktopFile = desktopFile;
        lastArgs = args;
        return lastAppInfo;
    }

    AppInfo getLatestAppInfoByCmdline(const std::string &cmdline)
    {
        lastCmdline = cmdline;
        return lastAppInfo;
    }

    AppInfo getAppInfo(const std::string &appId)
    {
        lastAppId = appId;
        return lastAppInfo;
    }

    std::string getDesktopFileByPid(int pid)
    {
        lastPid = pid;
        return lastDesktopFile;
    }

    void setAppStartedCallback(AppChangedCallback callback)
    {
        appStartedCallback = std::move(callback);
    }

    void setAppStateChangedCallback(AppChangedCallback callback)
    {
        appStateChangedCallback = std::move(callback);
    }

    void setAppChildPidsUpdatedCallback(AppChangedCallback callback)
    {
        appChildPidsUpdatedCallback = std::move(callback);
    }
};

// 初始化静态变量
std::string MockAppInfoManager::lastWid;
std::string MockAppInfoManager::lastDesktopFile;
std::string MockAppInfoManager::lastService;
std::string MockAppInfoManager::lastAppId;
std::string MockAppInfoManager::lastGroupName;
std::string MockAppInfoManager::lastCmdline;
std::vector<std::string> MockAppInfoManager::lastArgs;
int MockAppInfoManager::lastPid = 0;
int MockAppInfoManager::lastInterval = 0;
sched_policy::AppState MockAppInfoManager::lastState = sched_policy::AppState::Focus;
sched_policy::AppType MockAppInfoManager::lastAppType = sched_policy::AppType::Normal;
AppInfo MockAppInfoManager::lastAppInfo;
bool MockAppInfoManager::lastResult = false;
MockAppInfoManager::AppChangedCallback MockAppInfoManager::appStartedCallback = nullptr;
MockAppInfoManager::AppChangedCallback MockAppInfoManager::appStateChangedCallback = nullptr;
MockAppInfoManager::AppChangedCallback MockAppInfoManager::appChildPidsUpdatedCallback = nullptr;
std::vector<AppInfo> MockAppInfoManager::allAppInfos;
std::vector<AppInfo> MockAppInfoManager::appInfosWithState;

// 替换AppInfoManager为MockAppInfoManager以便测试
#define AppInfoManager MockAppInfoManager

class TestProcessInfoManager : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试用例
    void testHandleWindowAdded();
    void testHandleWindowRemoved();
    void testHandleActiveWindowChanged();
    void testHandleWindowMinimized();
    void testHandleExceptionGroupSessionApp();
    void testHandleMprisDbusAdded();
    void testHandleStatusNotifierItemRegistered();
    void testCreateAppInstance();
    void testSetAppGroupName();
    void testForceChangCachedStateTo();
    void testForceChangeAppStateByPid();
    void testSetBackgroundToCachedStateInterval();
    void testGetAppTypeByPid();
    void testSyncGetDesktopFileByPid();
    void testGetAllAppInfos();
    void testGetAppInfosWithState();
    void testGetLatestAppInfoByDesktopFile();
    void testGetLatestAppInfoByCmdline();
    void testGetAppInfo();
    void testGetDesktopFileByPid();
    void testSetCallbacks();

private:
    std::shared_ptr<ConfigManager> m_configManager;
    ProcessInfoManager *m_processInfoManager;
};

void TestProcessInfoManager::initTestCase()
{
    // 测试套件初始化
    m_configManager = std::make_shared<ConfigManager>();
}

void TestProcessInfoManager::cleanupTestCase()
{
    // 测试套件清理
    m_configManager.reset();
}

void TestProcessInfoManager::init()
{
    // 每个测试前初始化
    MockAppInfoManager::reset();
    m_processInfoManager = new ProcessInfoManager(m_configManager);
}

void TestProcessInfoManager::cleanup()
{
    // 每个测试后清理
    delete m_processInfoManager;
}

void TestProcessInfoManager::testHandleWindowAdded()
{
    std::string testWid = "test-window-id";
    m_processInfoManager->handleWindowAdded(testWid);
    QCOMPARE(QString::fromStdString(MockAppInfoManager::lastWid), QString::fromStdString(testWid));
}

void TestProcessInfoManager::testHandleWindowRemoved()
{
    std::string testWid = "test-window-id";
    m_processInfoManager->handleWindowRemoved(testWid);
    QCOMPARE(QString::fromStdString(MockAppInfoManager::lastWid), QString::fromStdString(testWid));
}

void TestProcessInfoManager::testHandleActiveWindowChanged()
{
    std::string testWid = "test-window-id";
    m_processInfoManager->handleActiveWindowChanged(testWid);
    QCOMPARE(QString::fromStdString(MockAppInfoManager::lastWid), QString::fromStdString(testWid));
}

void TestProcessInfoManager::testHandleWindowMinimized()
{
    std::string testWid = "test-window-id";
    m_processInfoManager->handleWindowMinimized(testWid);
    QCOMPARE(QString::fromStdString(MockAppInfoManager::lastWid), QString::fromStdString(testWid));
}

void TestProcessInfoManager::testHandleExceptionGroupSessionApp()
{
    std::string testDesktopFile = "test.desktop";
    int testPid = 12345;
    m_processInfoManager->handleExceptionGroupSessionApp(testDesktopFile, testPid);
    QCOMPARE(QString::fromStdString(MockAppInfoManager::lastDesktopFile), QString::fromStdString(testDesktopFile));
    QCOMPARE(MockAppInfoManager::lastPid, testPid);
}

void TestProcessInfoManager::testHandleMprisDbusAdded()
{
    int testPid = 12345;
    std::string testService = "org.mpris.MediaPlayer2.test";
    m_processInfoManager->handleMprisDbusAdded(testPid, testService);
    QCOMPARE(MockAppInfoManager::lastPid, testPid);
    QCOMPARE(QString::fromStdString(MockAppInfoManager::lastService), QString::fromStdString(testService));
}

void TestProcessInfoManager::testHandleStatusNotifierItemRegistered()
{
    int testPid = 12345;
    m_processInfoManager->handleStatusNotifierItemRegistered(testPid);
    QCOMPARE(MockAppInfoManager::lastPid, testPid);
}

void TestProcessInfoManager::testCreateAppInstance()
{
    std::string testDesktopFile = "test.desktop";
    std::vector<std::string> testArgs = {"--arg1", "--arg2", "value"};
    int testPid = 12345;
    m_processInfoManager->createAppInstance(testDesktopFile, testArgs, testPid);
    QCOMPARE(QString::fromStdString(MockAppInfoManager::lastDesktopFile), QString::fromStdString(testDesktopFile));
    QCOMPARE(MockAppInfoManager::lastArgs, testArgs);
    QCOMPARE(MockAppInfoManager::lastPid, testPid);
}

void TestProcessInfoManager::testSetAppGroupName()
{
    std::string testAppId = "test-app-id";
    std::string testGroupName = "test-group";
    m_processInfoManager->setAppGroupName(testAppId, testGroupName);
    QCOMPARE(QString::fromStdString(MockAppInfoManager::lastAppId), QString::fromStdString(testAppId));
    QCOMPARE(QString::fromStdString(MockAppInfoManager::lastGroupName), QString::fromStdString(testGroupName));
}

void TestProcessInfoManager::testForceChangCachedStateTo()
{
    sched_policy::AppState testState = sched_policy::AppState::Background;
    m_processInfoManager->forceChangCachedStateTo(testState);
    QCOMPARE(static_cast<int>(MockAppInfoManager::lastState), static_cast<int>(testState));
}

void TestProcessInfoManager::testForceChangeAppStateByPid()
{
    int testPid = 12345;
    sched_policy::AppState testState = sched_policy::AppState::Cached;
    MockAppInfoManager::lastResult = true;
    bool result = m_processInfoManager->forceChangeAppStateByPid(testPid, testState);
    QCOMPARE(MockAppInfoManager::lastPid, testPid);
    QCOMPARE(static_cast<int>(MockAppInfoManager::lastState), static_cast<int>(testState));
    QCOMPARE(result, MockAppInfoManager::lastResult);
}

void TestProcessInfoManager::testSetBackgroundToCachedStateInterval()
{
    int testInterval = 1000;
    m_processInfoManager->setBackgroundToCachedStateInterval(testInterval);
    QCOMPARE(MockAppInfoManager::lastInterval, testInterval);
}

void TestProcessInfoManager::testGetAppTypeByPid()
{
    int testPid = 12345;
    MockAppInfoManager::lastAppType = sched_policy::AppType::Session;
    sched_policy::AppType result = m_processInfoManager->getAppTypeByPid(testPid);
    QCOMPARE(MockAppInfoManager::lastPid, testPid);
    QCOMPARE(static_cast<int>(result), static_cast<int>(MockAppInfoManager::lastAppType));
}

void TestProcessInfoManager::testSyncGetDesktopFileByPid()
{
    int testPid = 12345;
    std::string testDesktopFile = "test.desktop";
    MockAppInfoManager::lastDesktopFile = testDesktopFile;
    std::string result = m_processInfoManager->syncGetDesktopFileByPid(testPid);
    QCOMPARE(MockAppInfoManager::lastPid, testPid);
    QCOMPARE(QString::fromStdString(result), QString::fromStdString(testDesktopFile));
}

void TestProcessInfoManager::testGetAllAppInfos()
{
    // 准备测试数据
    AppInfo testApp1;
    AppInfo testApp2;
    std::vector<AppInfo> testAppInfos = {testApp1, testApp2};
    MockAppInfoManager::allAppInfos = testAppInfos;

    // 执行测试
    std::vector<AppInfo> result = m_processInfoManager->getAllAppInfos();

    // 验证结果
    QCOMPARE(result.size(), testAppInfos.size());
}

void TestProcessInfoManager::testGetAppInfosWithState()
{
    // 准备测试数据
    AppInfo testApp1;
    AppInfo testApp2;
    std::vector<AppInfo> testAppInfos = {testApp1, testApp2};
    MockAppInfoManager::appInfosWithState = testAppInfos;
    sched_policy::AppState testState = sched_policy::AppState::Background;

    // 执行测试
    std::vector<AppInfo> result = m_processInfoManager->getAppInfosWithState(testState);

    // 验证结果
    QCOMPARE(result.size(), testAppInfos.size());
    QCOMPARE(static_cast<int>(MockAppInfoManager::lastState), static_cast<int>(testState));
}

void TestProcessInfoManager::testGetLatestAppInfoByDesktopFile()
{
    // 准备测试数据
    std::string testDesktopFile = "test.desktop";
    std::vector<std::string> testArgs = {"--arg1", "--arg2"};
    AppInfo testAppInfo;
    testAppInfo.setAppId("test-app-id");
    MockAppInfoManager::lastAppInfo = testAppInfo;

    // 执行测试
    AppInfo result = m_processInfoManager->getLatestAppInfoByDesktopFile(testDesktopFile, testArgs);

    // 验证结果
    QCOMPARE(QString::fromStdString(MockAppInfoManager::lastDesktopFile), QString::fromStdString(testDesktopFile));
    QCOMPARE(MockAppInfoManager::lastArgs, testArgs);
    QCOMPARE(QString::fromStdString(result.desktopFilePid()), QString::fromStdString(testAppInfo.desktopFilePid()));
}

void TestProcessInfoManager::testGetLatestAppInfoByCmdline()
{
    // 准备测试数据
    std::string testCmdline = "test --arg1 --arg2";
    AppInfo testAppInfo;
    testAppInfo.setAppId("test-app-id");
    MockAppInfoManager::lastAppInfo = testAppInfo;

    // 执行测试
    AppInfo result = m_processInfoManager->getLatestAppInfoByCmdline(testCmdline);

    // 验证结果
    QCOMPARE(QString::fromStdString(MockAppInfoManager::lastCmdline), QString::fromStdString(testCmdline));
    QCOMPARE(QString::fromStdString(result.desktopFilePid()), QString::fromStdString(testAppInfo.desktopFilePid()));
}

void TestProcessInfoManager::testGetAppInfo()
{
    // 准备测试数据
    std::string testAppId = "test-app-id";
    AppInfo testAppInfo;
    testAppInfo.setAppId(testAppId);
    MockAppInfoManager::lastAppInfo = testAppInfo;

    // 执行测试
    AppInfo result = m_processInfoManager->getAppInfo(testAppId);

    // 验证结果
    QCOMPARE(QString::fromStdString(MockAppInfoManager::lastAppId), QString::fromStdString(testAppId));
    QCOMPARE(QString::fromStdString(result.desktopFilePid()), QString::fromStdString(testAppInfo.desktopFilePid()));
}

void TestProcessInfoManager::testGetDesktopFileByPid()
{
    // 准备测试数据
    int testPid = 12345;
    std::string testDesktopFile = "test.desktop";
    MockAppInfoManager::lastDesktopFile = testDesktopFile;

    // 执行测试
    std::string result = m_processInfoManager->getDesktopFileByPid(testPid);

    // 验证结果
    QCOMPARE(MockAppInfoManager::lastPid, testPid);
    QCOMPARE(QString::fromStdString(result), QString::fromStdString(testDesktopFile));
}

void TestProcessInfoManager::testSetCallbacks()
{
    // 准备测试数据
    auto testCallback = [](const AppInfo &) {};

    // 执行测试 - 设置应用启动回调
    m_processInfoManager->setAppStartedCallback(testCallback);
    QVERIFY(MockAppInfoManager::appStartedCallback != nullptr);

    // 执行测试 - 设置应用状态变化回调
    m_processInfoManager->setAppStateChangedCallback(testCallback);
    QVERIFY(MockAppInfoManager::appStateChangedCallback != nullptr);

    // 执行测试 - 设置应用子进程更新回调
    m_processInfoManager->setAppChildPidsUpdatedCallback(testCallback);
    QVERIFY(MockAppInfoManager::appChildPidsUpdatedCallback != nullptr);
}

// 取消宏定义
#undef AppInfoManager

QTEST_MAIN(TestProcessInfoManager)
#include "tst_processinfomanager.moc"
