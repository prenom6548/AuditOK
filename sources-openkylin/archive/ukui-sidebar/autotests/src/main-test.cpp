/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <QtTest>

#include "qtsingleapplication.h"
#include "sidebar-main.h"
#include "log-utils.h"

QString parseArgs(const QStringList &args);
int ukui_sidebar_main(int argc, char *argv[]);

class main_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_parseArgs_noArguments();
    void test_parseArgs_knownOptions();
    void test_main_x11EmptyDisplayUsesDisplayEnv();
    void test_main_runningInstanceSendsMessage();
    void test_main_newInstanceCreatesSidebarAndExecs();
    void test_main_waylandEnvironment();
};

void main_test::init()
{
    QtSingleApplication::reset();
    SidebarMain::resetRecords();
    LogUtils::reset();
    qunsetenv("XDG_SESSION_TYPE");
    qunsetenv("DISPLAY");
    qunsetenv("WAYLAND_DISPLAY");
    qunsetenv("QT_WAYLAND_DISABLE_FIXED_POSITIONS");
    qunsetenv("QT_WAYLAND_SHELL_INTEGRATION");
}

void main_test::test_parseArgs_noArguments()
{
    QVERIFY(parseArgs({"ukui-sidebar"}).isEmpty());
}

void main_test::test_parseArgs_knownOptions()
{
    QCOMPARE(parseArgs({"ukui-sidebar", "--state"}), QString("state"));
    QCOMPARE(parseArgs({"ukui-sidebar", "--show", "notify"}), QString("show notify"));
    QCOMPARE(parseArgs({"ukui-sidebar", "-s", "control"}), QString("show control"));
    QCOMPARE(parseArgs({"ukui-sidebar", "--quit"}), QString("quit"));
}

void main_test::test_main_x11EmptyDisplayUsesDisplayEnv()
{
    QtSingleApplication::setRunning(true);
    qputenv("XDG_SESSION_TYPE", "x11");
    qunsetenv("DISPLAY");
    char arg0[] = "ukui-sidebar";
    char *argv[] = {arg0};
    int argc = 1;

    int result = ukui_sidebar_main(argc, argv);

    QCOMPARE(result, 0);
    QCOMPARE(QtSingleApplication::lastId(), QString("ukui-sidebar-qml-"));
}

void main_test::test_main_runningInstanceSendsMessage()
{
    QtSingleApplication::setRunning(true);
    qputenv("DISPLAY", ":99");
    char arg0[] = "ukui-sidebar";
    char arg1[] = "--show";
    char arg2[] = "notify";
    char *argv[] = {arg0, arg1, arg2};
    int argc = 3;

    int result = ukui_sidebar_main(argc, argv);

    QCOMPARE(result, 0);
    QCOMPARE(QtSingleApplication::lastMessage(), QString("show notify"));
    QCOMPARE(QtSingleApplication::lastId(), QString("ukui-sidebar-qml-:99"));
    QCOMPARE(SidebarMain::calls().isEmpty(), true);
    QCOMPARE(LogUtils::lastInitFileName(), QString("ukui-sidebar"));
}

void main_test::test_main_newInstanceCreatesSidebarAndExecs()
{
    QtSingleApplication::setExecResult(7);
    qputenv("DISPLAY", ":88");
    char arg0[] = "ukui-sidebar";
    char arg1[] = "--quit";
    char *argv[] = {arg0, arg1};
    int argc = 2;

    int result = ukui_sidebar_main(argc, argv);

    QCOMPARE(result, 7);
    QVERIFY(SidebarMain::calls().contains("construct"));
    QCOMPARE(SidebarMain::lastParsedMessage(), QString("quit"));
    QCOMPARE(QtSingleApplication::execCallCount(), 1);
}

void main_test::test_main_waylandEnvironment()
{
    qputenv("XDG_SESSION_TYPE", "wayland");
    qputenv("WAYLAND_DISPLAY", "wayland-1");
    char arg0[] = "ukui-sidebar";
    char *argv[] = {arg0};
    int argc = 1;

    ukui_sidebar_main(argc, argv);

    QCOMPARE(QtSingleApplication::lastId(), QString("ukui-sidebar-qml-wayland-1"));
    QCOMPARE(qgetenv("QT_WAYLAND_DISABLE_FIXED_POSITIONS"), QByteArray("true"));
    QCOMPARE(qgetenv("QT_WAYLAND_SHELL_INTEGRATION"), QByteArray("ukui-shell"));
}

QTEST_MAIN(main_test)

#include "main-test.moc"
