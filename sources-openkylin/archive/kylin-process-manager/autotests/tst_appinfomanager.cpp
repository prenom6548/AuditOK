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

#include "appinfomanager.h"
#include "configmanager.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QtTest>

// add necessary includes here

class TestAppInfoManager : public QObject
{
    Q_OBJECT

public:
    TestAppInfoManager();
    ~TestAppInfoManager();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();
    void test_case2();
    void test_appinfo();
    void test_appTypeDetection();
    void test_appInfoProperties();
};

TestAppInfoManager::TestAppInfoManager()
{
}

TestAppInfoManager::~TestAppInfoManager()
{
}

void TestAppInfoManager::initTestCase()
{
}

void TestAppInfoManager::cleanupTestCase()
{
}

void TestAppInfoManager::test_case1()
{
    AppInfoManager aim(std::make_shared<ConfigManager>());
    QVERIFY(aim.getAppTypeByDesktopFile("/etc/xdg/autostart/ukui-menu.desktop") == sched_policy::AppType::Session);
    QVERIFY(aim.getAppTypeByDesktopFile("/etc/xdg/autostart/ukui-kwin.desktop") == sched_policy::AppType::Session);
    QVERIFY(aim.getAppTypeByDesktopFile("/etc/xdg/autostart/ukui-panel.desktop") == sched_policy::AppType::Session);
    QVERIFY(aim.getAppTypeByDesktopFile("/usr/share/applications/kylin-video.desktop") == sched_policy::AppType::Normal);
    QVERIFY(aim.getAppTypeByDesktopFile("/etc/xdg/autostart/ukui-sidebar.desktop") == sched_policy::AppType::Session);
    QVERIFY(aim.getAppTypeByDesktopFile("/etc/xdg/autostart/1234.desktop") == sched_policy::AppType::Unknown);
    QVERIFY(aim.getAppTypeByDesktopFile("/usr/share/applications/kylin-music.desktop") == sched_policy::AppType::Normal);
    QVERIFY(aim.getAppTypeByDesktopFile("/usr/share/applications/kylin1234.desktop") == sched_policy::AppType::Unknown);
}

void TestAppInfoManager::test_case2()
{
    QEventLoop eventLoop;
    // AppInfoManager aim(std::make_shared<ConfigManager>());
    // QSignalSpy spy(&aim, &AppInfoManager::appStarted);

    // bool callbackCalled = false;
    // aim.setAppStartedCallback([&](const AppInfo &appInfo) {
    // QVERIFY(!appInfo.appId().empty());
    // QVERIFY(!appInfo.pids().empty());
    //     callbackCalled = true;
    //     eventLoop.quit();
    // });

    // // Test with valid window ID
    // aim.handleWindowAdded("12345");
    // QVERIFY(spy.wait(1000));
    // QVERIFY(callbackCalled);

    // // Test with invalid window ID
    // callbackCalled = false;
    // aim.handleWindowAdded("invalid");
    // QTest::qWait(500);
    // QVERIFY(!callbackCalled);
}

void TestAppInfoManager::test_appTypeDetection()
{
    AppInfoManager aim(std::make_shared<ConfigManager>());

    // Test system service detection
    QCOMPARE(aim.getAppTypeByDesktopFile("/usr/share/applications/kylin-video.desktop"),
             sched_policy::AppType::Normal);

    // Test background service detection
    QCOMPARE(aim.getAppTypeByDesktopFile("/etc/xdg/autostart/ukui-menu.desktop"),
             sched_policy::AppType::Session);

    // Test invalid desktop file
    QCOMPARE(aim.getAppTypeByDesktopFile("/invalid/path.desktop"),
             sched_policy::AppType::Unknown);
}

void TestAppInfoManager::test_appInfoProperties()
{
    AppInfo ai("/usr/share/applications/kylin-video.desktop",
               45878, 1234, sched_policy::AppType::Normal,
               sched_policy::AppState::Focus, nullptr);

    QCOMPARE(ai.desktopFile(), "/usr/share/applications/kylin-video.desktop");
    QCOMPARE(ai.launcherPid(), 45878);
    QCOMPARE(ai.wids().size(), 0);
    QCOMPARE(ai.appType(), sched_policy::AppType::Normal);
    QCOMPARE(ai.appState(), sched_policy::AppState::Focus);
}

void TestAppInfoManager::test_appinfo()
{
    AppInfo ai1("/usr/share/applications/kylin-video.desktop",
                45878, 0, sched_policy::AppType::Normal, sched_policy::AppState::Focus, nullptr);
    AppInfo ai2("/usr/share/applications/kylin-video.desktop",
                45878, 0, sched_policy::AppType::Normal, sched_policy::AppState::Focus, nullptr);
    AppInfo ai3("/usr/share/applications/kylin-video.desktop",
                45879, 0, sched_policy::AppType::Normal, sched_policy::AppState::Focus, nullptr);
    QVERIFY(ai1 == ai2);
    QVERIFY(!(ai1 == ai3));
    QVERIFY(!(ai2 == ai3));
}

QTEST_MAIN(TestAppInfoManager)

#include "tst_appinfomanager.moc"
