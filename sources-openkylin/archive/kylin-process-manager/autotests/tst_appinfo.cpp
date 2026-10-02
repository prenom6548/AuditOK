/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include "appinfo.h"
#include <QtTest>

using namespace sched_policy;

class TestAppInfo : public QObject
{
    Q_OBJECT

public:
    TestAppInfo();
    ~TestAppInfo();

private Q_SLOTS:
    void testDefaultConstructor();
    void testParameterizedConstructor();
    void testCopyConstructor();
    void testAssignmentOperator();
    void testGetters();
    void testSetters();
    void testPidsOperations();
    void testWidsOperations();
    void testStateTransitions();
    void testCallbackHandling();
};

TestAppInfo::TestAppInfo()
{
}
TestAppInfo::~TestAppInfo()
{
}

void TestAppInfo::testDefaultConstructor()
{
    AppInfo info;
    QVERIFY(info.desktopFilePid().empty());
    QVERIFY(info.desktopFile().empty());
    QVERIFY(info.cmdline().empty());
    QVERIFY(info.pids().empty());
    QVERIFY(info.wids().empty());
    QCOMPARE(info.appState(), AppState::Focus);
}

void TestAppInfo::testParameterizedConstructor()
{
    AppInfo info("/usr/share/applications/test.desktop",
                 1234, 1698134400,
                 AppType::Normal,
                 AppState::Focus,
                 [](AppInfo &) {});

    QCOMPARE(info.desktopFile(), "/usr/share/applications/test.desktop");
    QCOMPARE(info.launcherPid(), 1234);
    QCOMPARE(info.launchTimestamp(), 1698134400);
    QCOMPARE(info.appType(), AppType::Normal);
    QCOMPARE(info.appState(), AppState::Focus);
}

void TestAppInfo::testCopyConstructor()
{
    AppInfo original("/usr/share/applications/test.desktop",
                     1234, 1698134400,
                     AppType::Normal,
                     AppState::Focus,
                     [](AppInfo &) {});

    AppInfo copy(original);

    QCOMPARE(copy.desktopFile(), original.desktopFile());
    QCOMPARE(copy.launcherPid(), original.launcherPid());
    QCOMPARE(copy.appType(), original.appType());
    QCOMPARE(copy.appState(), original.appState());
}

void TestAppInfo::testAssignmentOperator()
{
    AppInfo original("/usr/share/applications/test.desktop",
                     1234, 1698134400,
                     AppType::Normal,
                     AppState::Focus,
                     [](AppInfo &) {});

    AppInfo copy;
    copy = original;

    QCOMPARE(copy.desktopFile(), original.desktopFile());
    QCOMPARE(copy.launcherPid(), original.launcherPid());
    QCOMPARE(copy.appType(), original.appType());
    QCOMPARE(copy.appState(), original.appState());
}

void TestAppInfo::testGetters()
{
    AppInfo info("/usr/share/applications/test.desktop",
                 1234, 1698134400,
                 AppType::Normal,
                 AppState::Focus,
                 [](AppInfo &) {});

    QCOMPARE(info.desktopFile(), "/usr/share/applications/test.desktop");
    QCOMPARE(info.launcherPid(), 1234);
    QCOMPARE(info.launchTimestamp(), 1698134400);
    QCOMPARE(info.appType(), AppType::Normal);
    QCOMPARE(info.appState(), AppState::Focus);
}

void TestAppInfo::testSetters()
{
    AppInfo info;

    info.setAppId("testapp");
    QCOMPARE(info.appId(), "testapp");

    info.setGroupName("testgroup");
    QCOMPARE(info.groupName(), "testgroup");

    info.setCmdline("testcmd");
    QCOMPARE(info.cmdline(), "testcmd");

    info.setMrprisDbusService("testmpris");
    QCOMPARE(info.mprisDbusService(), "testmpris");

    info.setIsSystemTrayIconApp(true);
    QVERIFY(info.isSystemTrayIconApp());
}

void TestAppInfo::testPidsOperations()
{
    AppInfo info;
    QVERIFY(info.pids().empty());

    info.setAppId("testapp");
    QVERIFY(info.containsPid(1234) == false);
}

void TestAppInfo::testWidsOperations()
{
    AppInfo info;
    QVERIFY(info.wids().empty());

    info.appendWid("wid1");
    QVERIFY(info.containsWid("wid1"));

    info.removeWid("wid1");
    QVERIFY(!info.containsWid("wid1"));
}

void TestAppInfo::testStateTransitions()
{
    AppInfo info;

    info.setAppState(AppState::Focus);
    QCOMPARE(info.appState(), AppState::Focus);

    info.setAppState(AppState::Background);
    QCOMPARE(info.appState(), AppState::Background);
}

void TestAppInfo::testCallbackHandling()
{
    bool callbackCalled = false;
    AppInfo info("/usr/share/applications/test.desktop",
                 1234, 1698134400,
                 AppType::Normal,
                 AppState::Focus,
                 [&callbackCalled](AppInfo &) {
        callbackCalled = true;
    });

    info.setAppState(AppState::Background);
    QVERIFY(callbackCalled);
}

QTEST_MAIN(TestAppInfo)

#include "tst_appinfo.moc"
