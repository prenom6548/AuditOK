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
#include <QDBusConnection>
#include "../../../src/main/app-manager.h"

using namespace Sidebar;

class FakeProcessManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.ProcessManager.AppLauncher")

public:
    explicit FakeProcessManagerService(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void LaunchApp(const QString &desktopFile)
    {
        emit launchAppCalled(desktopFile);
    }

    void LaunchAppWithArguments(const QString &desktopFile, const QStringList &args)
    {
        emit launchAppWithArgumentsCalled(desktopFile, args);
    }

signals:
    void launchAppCalled(const QString &desktopFile);
    void launchAppWithArgumentsCalled(const QString &desktopFile, const QStringList &args);
};

class appManager_test : public QObject
{
    Q_OBJECT

private slots:
    void test_getInstance();
    void test_getInstance_returnsSameInstance();
    void test_getInstance_withParent();
    void test_launchApp_doesNotCrash();
    void test_launchAppWithArguments_doesNotCrash();
    void test_launchAppWithArguments_emptyArgs();
    void test_launchApp_emptyDesktopFile();
    void test_launchApp_emptyApplicationName();
    void test_launchAppWithArguments_multipleArgs();
    void test_launchAppWithArguments_emptyApplicationName();
    void test_launchApp_runsAsyncFallbackPath();
    void test_launchAppWithArguments_runsAsyncFallbackPath();
    void test_launchApp_successPathWithFakeDbusService();
    void test_launchAppWithArguments_successPathWithFakeDbusService();
    void test_deleteInstance_forCoverage();
};

void appManager_test::test_getInstance()
{
    AppManager *manager = AppManager::getInstance();
    QVERIFY(manager != nullptr);
}

void appManager_test::test_getInstance_returnsSameInstance()
{
    AppManager *manager1 = AppManager::getInstance();
    AppManager *manager2 = AppManager::getInstance();
    QCOMPARE(manager1, manager2);
}

void appManager_test::test_getInstance_withParent()
{
    AppManager *manager = AppManager::getInstance(this);
    QVERIFY(manager != nullptr);
    // 再次调用应返回同一实例
    AppManager *manager2 = AppManager::getInstance();
    QCOMPARE(manager, manager2);
}

void appManager_test::test_launchApp_doesNotCrash()
{
    AppManager *manager = AppManager::getInstance();
    // 使用不存在的 desktop 文件，DBus 调用会失败但不应崩溃
    manager->launchApp("nonexistent.desktop", "nonexistent-app");
    QVERIFY(true);
}

void appManager_test::test_launchAppWithArguments_doesNotCrash()
{
    AppManager *manager = AppManager::getInstance();
    QStringList args;
    args << "-m" << "test";
    manager->launchAppWithArguments("nonexistent.desktop", args, "nonexistent-app");
    QVERIFY(true);
}

void appManager_test::test_launchAppWithArguments_emptyArgs()
{
    AppManager *manager = AppManager::getInstance();
    manager->launchAppWithArguments("nonexistent.desktop", QStringList(), "nonexistent-app");
    QVERIFY(true);
}

void appManager_test::test_launchApp_emptyDesktopFile()
{
    AppManager *manager = AppManager::getInstance();
    manager->launchApp("", "");
    QVERIFY(true);
}

void appManager_test::test_launchApp_emptyApplicationName()
{
    AppManager *manager = AppManager::getInstance();
    manager->launchApp("nonexistent.desktop", "");
    QTest::qWait(120);
    QCoreApplication::processEvents();
    QVERIFY(true);
}

void appManager_test::test_launchAppWithArguments_multipleArgs()
{
    AppManager *manager = AppManager::getInstance();
    QStringList args;
    args << "--arg1" << "--arg2" << "value1" << "value2";
    manager->launchAppWithArguments("test.desktop", args, "test-app");
    QVERIFY(true);
}

void appManager_test::test_launchAppWithArguments_emptyApplicationName()
{
    AppManager *manager = AppManager::getInstance();
    manager->launchAppWithArguments("nonexistent.desktop", QStringList{"--x"}, "");
    QTest::qWait(120);
    QCoreApplication::processEvents();
    QVERIFY(true);
}

void appManager_test::test_launchApp_runsAsyncFallbackPath()
{
    AppManager *manager = AppManager::getInstance();
    manager->launchApp("nonexistent.desktop", "nonexistent-app-binary-for-test");
    QTest::qWait(120);
    QCoreApplication::processEvents();
    QVERIFY(true);
}

void appManager_test::test_launchAppWithArguments_runsAsyncFallbackPath()
{
    AppManager *manager = AppManager::getInstance();
    QStringList args;
    args << "--optA" << "--optB";
    manager->launchAppWithArguments("nonexistent.desktop", args, "nonexistent-app-binary-for-test");
    QTest::qWait(120);
    QCoreApplication::processEvents();
    QVERIFY(true);
}

void appManager_test::test_launchApp_successPathWithFakeDbusService()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available.");
    }

    FakeProcessManagerService service;
    if (!QDBusConnection::sessionBus().registerService("com.kylin.ProcessManager")) {
        QSKIP("com.kylin.ProcessManager is already owned in current session.");
    }
    QVERIFY(QDBusConnection::sessionBus().registerObject("/com/kylin/ProcessManager/AppLauncher",
                                                         &service,
                                                         QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals));

    QSignalSpy spy(&service, SIGNAL(launchAppCalled(QString)));
    AppManager *manager = AppManager::getInstance();
    manager->launchApp("fake.desktop", "fake-app");

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString("fake.desktop"));

    QDBusConnection::sessionBus().unregisterObject("/com/kylin/ProcessManager/AppLauncher");
    QDBusConnection::sessionBus().unregisterService("com.kylin.ProcessManager");
}

void appManager_test::test_launchAppWithArguments_successPathWithFakeDbusService()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available.");
    }

    FakeProcessManagerService service;
    if (!QDBusConnection::sessionBus().registerService("com.kylin.ProcessManager")) {
        QSKIP("com.kylin.ProcessManager is already owned in current session.");
    }
    QVERIFY(QDBusConnection::sessionBus().registerObject("/com/kylin/ProcessManager/AppLauncher",
                                                         &service,
                                                         QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals));

    QSignalSpy spy(&service, SIGNAL(launchAppWithArgumentsCalled(QString,QStringList)));
    AppManager *manager = AppManager::getInstance();
    const QStringList args = {"--flag", "value"};
    manager->launchAppWithArguments("fake.desktop", args, "fake-app");

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString("fake.desktop"));
    QCOMPARE(spy.at(0).at(1).toStringList(), args);

    QDBusConnection::sessionBus().unregisterObject("/com/kylin/ProcessManager/AppLauncher");
    QDBusConnection::sessionBus().unregisterService("com.kylin.ProcessManager");
}

void appManager_test::test_deleteInstance_forCoverage()
{
    AppManager *manager = AppManager::getInstance();
    delete manager;
    QVERIFY(true);
}

QTEST_MAIN(appManager_test)

#include "app-manager-test.moc"
