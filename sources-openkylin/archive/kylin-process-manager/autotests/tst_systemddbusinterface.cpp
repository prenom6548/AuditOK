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

#include "../base/systemddbusinterface.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QSignalSpy>
#include <QtTest>

// 用于模拟DBus服务的测试类（模拟systemd服务）
class MockSystemdDBusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.systemd1.Manager")

public:
    explicit MockSystemdDBusService(QObject *parent = nullptr)
        : QObject(parent)
    {
        // 注册会话总线上的服务，而不是系统总线，以避免权限问题
        QDBusConnection::sessionBus().registerObject("/org/freedesktop/systemd1", this,
                                                     QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
        QDBusConnection::sessionBus().registerService("org.freedesktop.systemd1");
    }

    ~MockSystemdDBusService()
    {
        QDBusConnection::sessionBus().unregisterService("org.freedesktop.systemd1");
        QDBusConnection::sessionBus().unregisterObject("/org/freedesktop/systemd1");
    }

    // 记录方法调用情况
    QStringList calledMethods;
    QList<QVariantList> calledArguments;

public Q_SLOTS:
    // StartUnit方法
    Q_SCRIPTABLE void StartUnit(const QString &name, const QString &mode)
    {
        calledMethods.append("StartUnit");
        QVariantList args;
        args << name << mode;
        calledArguments.append(args);
    }

    // StartTransientUnit方法
    Q_SCRIPTABLE void StartTransientUnit(const QString &name, const QString &mode,
                                         const QDBusArgument &properties,
                                         const QDBusArgument &aux)
    {
        calledMethods.append("StartTransientUnit");
        QVariantList args;
        args << name << mode;
        calledArguments.append(args);
    }

    // SetUnitProperties方法
    Q_SCRIPTABLE void SetUnitProperties(const QString &name, bool runtime,
                                        const QDBusArgument &properties)
    {
        calledMethods.append("SetUnitProperties");
        QVariantList args;
        args << name << runtime;
        calledArguments.append(args);
    }

    // FreezeUnit方法
    Q_SCRIPTABLE void FreezeUnit(const QString &name)
    {
        calledMethods.append("FreezeUnit");
        QVariantList args;
        args << name;
        calledArguments.append(args);
    }

    // ThawUnit方法
    Q_SCRIPTABLE void ThawUnit(const QString &name)
    {
        calledMethods.append("ThawUnit");
        QVariantList args;
        args << name;
        calledArguments.append(args);
    }

    // GetUnitProcesses方法
    Q_SCRIPTABLE QList<QPair<QString, uint>> GetUnitProcesses(const QString &name)
    {
        calledMethods.append("GetUnitProcesses");
        QVariantList args;
        args << name;
        calledArguments.append(args);

        // 返回模拟的进程列表
        QList<QPair<QString, uint>> processes;
        processes.append(qMakePair(QString("process1"), 1234u));
        processes.append(qMakePair(QString("process2"), 5678u));
        return processes;
    }
};

// 用于模拟login1 DBus服务的测试类
class MockLoginDBusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus.Properties")

public:
    explicit MockLoginDBusService(QObject *parent = nullptr)
        : QObject(parent)
    {
        // 注册和设置模拟的login1服务
        QDBusConnection::sessionBus().registerObject("/org/freedesktop/login1/user/self", this,
                                                     QDBusConnection::ExportAllSlots);
        QDBusConnection::sessionBus().registerObject("/org/freedesktop/login1/session/auto", this,
                                                     QDBusConnection::ExportAllSlots);
        QDBusConnection::sessionBus().registerService("org.freedesktop.login1");

        // 设置模拟的属性值
        userProperties["Slice"] = "user-1000.slice";
        userProperties["Service"] = "user@1000.service";
        sessionProperties["Scope"] = "session-1.scope";
    }

    ~MockLoginDBusService()
    {
        QDBusConnection::sessionBus().unregisterService("org.freedesktop.login1");
        QDBusConnection::sessionBus().unregisterObject("/org/freedesktop/login1/user/self");
        QDBusConnection::sessionBus().unregisterObject("/org/freedesktop/login1/session/auto");
    }

    // 记录方法调用情况
    QStringList calledMethods;
    QMap<QString, QString> userProperties;
    QMap<QString, QString> sessionProperties;

public Q_SLOTS:
    Q_SCRIPTABLE QDBusVariant Get(const QString &interface, const QString &property)
    {
        calledMethods.append("Get");

        if (interface == "org.freedesktop.login1.User") {
            return QDBusVariant(userProperties.value(property));
        } else if (interface == "org.freedesktop.login1.Session") {
            return QDBusVariant(sessionProperties.value(property));
        }

        return QDBusVariant("");
    }
};

class TestSystemdDbusInterface : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_currentUserSliceName();
    void test_currentUserServiceName();
    void test_currentSessionScopeName();
    void test_createPersistentProcessGroup();
    void test_createTransientProcessGroup();
    void test_setProcessGroupResourceLimit_general();
    void test_setProcessGroupResourceLimit_freezer();
    void test_setSystemdUnitPropertyEnabled();
    void test_getUnitProcessIds();

private:
    MockSystemdDBusService *mockSystemdService;
    MockLoginDBusService *mockLoginService;
    SystemdDbusInterface *systemdDbus;
};

void TestSystemdDbusInterface::initTestCase()
{
    // 启动模拟的DBus服务
    mockSystemdService = new MockSystemdDBusService(this);
    mockLoginService = new MockLoginDBusService(this);

    // 创建DBus接口实例
    systemdDbus = new SystemdDbusInterface();

    // 等待一段时间确保DBus服务注册完成
    QTest::qWait(100);
}

void TestSystemdDbusInterface::cleanupTestCase()
{
    delete systemdDbus;
    delete mockLoginService;
    delete mockSystemdService;
}

void TestSystemdDbusInterface::init()
{
    // 每个测试前清空调用记录
    mockSystemdService->calledMethods.clear();
    mockSystemdService->calledArguments.clear();
    mockLoginService->calledMethods.clear();
}

void TestSystemdDbusInterface::cleanup()
{
    // 测试后的清理工作
}

void TestSystemdDbusInterface::test_currentUserSliceName()
{
    std::string slice = systemdDbus->currentUserSliceName();

    // 验证DBus调用
    QVERIFY(mockLoginService->calledMethods.contains("Get"));

    // 验证返回值
    QCOMPARE(slice, "user-1000.slice");
}

void TestSystemdDbusInterface::test_currentUserServiceName()
{
    std::string service = systemdDbus->currentUserServiceName();

    // 验证DBus调用
    QVERIFY(mockLoginService->calledMethods.contains("Get"));

    // 验证返回值
    QCOMPARE(service, "user@1000.service");
}

void TestSystemdDbusInterface::test_currentSessionScopeName()
{
    std::string scope = systemdDbus->currentSessionScopeName();

    // 验证DBus调用
    QVERIFY(mockLoginService->calledMethods.contains("Get"));

    // 验证返回值
    QCOMPARE(scope, "session-1.scope");
}

void TestSystemdDbusInterface::test_createPersistentProcessGroup()
{
    std::string groupName = "test.slice";

    systemdDbus->createPersistentProcessGroup(groupName);

    // 等待异步调用完成
    QTest::qWait(100);

    // 验证调用
    QVERIFY(mockSystemdService->calledMethods.contains("StartUnit"));
    QCOMPARE(mockSystemdService->calledMethods.count("StartUnit"), 1);

    // 验证参数
    int idx = mockSystemdService->calledMethods.indexOf("StartUnit");
    QVariantList args = mockSystemdService->calledArguments.at(idx);
    QCOMPARE(args.at(0).toString(), QString::fromStdString(groupName));
    QCOMPARE(args.at(1).toString(), QString("fail"));
}

void TestSystemdDbusInterface::test_createTransientProcessGroup()
{
    std::string name = "app-test.scope";
    std::vector<int> pids = {1234, 5678};
    std::string parentSlice = "user-1000.slice";

    systemdDbus->createTransientProcessGroup(name, pids, parentSlice);

    // 等待异步调用完成
    QTest::qWait(100);

    // 验证调用
    QVERIFY(mockSystemdService->calledMethods.contains("StartTransientUnit"));
    QCOMPARE(mockSystemdService->calledMethods.count("StartTransientUnit"), 1);

    // 验证参数
    int idx = mockSystemdService->calledMethods.indexOf("StartTransientUnit");
    QVariantList args = mockSystemdService->calledArguments.at(idx);
    QCOMPARE(args.at(0).toString(), QString::fromStdString(name));
    QCOMPARE(args.at(1).toString(), QString("fail"));
}

void TestSystemdDbusInterface::test_setProcessGroupResourceLimit_general()
{
    std::string unitName = "app-test.scope";
    std::string attributeName = "CPUWeight";
    std::string value = "100";

    systemdDbus->setProcessGroupResourceLimit(unitName, attributeName, value);

    // 等待异步调用完成
    QTest::qWait(100);

    // 验证调用
    QVERIFY(mockSystemdService->calledMethods.contains("SetUnitProperties"));
    QCOMPARE(mockSystemdService->calledMethods.count("SetUnitProperties"), 1);

    // 验证参数
    int idx = mockSystemdService->calledMethods.indexOf("SetUnitProperties");
    QVariantList args = mockSystemdService->calledArguments.at(idx);
    QCOMPARE(args.at(0).toString(), QString::fromStdString(unitName));
    QCOMPARE(args.at(1).toBool(), true);
}

void TestSystemdDbusInterface::test_setProcessGroupResourceLimit_freezer()
{
    std::string unitName = "app-test.scope";
    std::string attributeName = "Freezer";

    // 测试冻结
    std::string freezeValue = "1";
    systemdDbus->setProcessGroupResourceLimit(unitName, attributeName, freezeValue);

    // 等待异步调用完成
    QTest::qWait(100);

    // 验证调用
    QVERIFY(mockSystemdService->calledMethods.contains("FreezeUnit"));
    QCOMPARE(mockSystemdService->calledMethods.count("FreezeUnit"), 1);

    // 验证参数
    int freezeIdx = mockSystemdService->calledMethods.indexOf("FreezeUnit");
    QVariantList freezeArgs = mockSystemdService->calledArguments.at(freezeIdx);
    QCOMPARE(freezeArgs.at(0).toString(), QString::fromStdString(unitName));

    // 清空调用记录
    mockSystemdService->calledMethods.clear();
    mockSystemdService->calledArguments.clear();

    // 测试解冻
    std::string thawValue = "0";
    systemdDbus->setProcessGroupResourceLimit(unitName, attributeName, thawValue);

    // 等待异步调用完成
    QTest::qWait(100);

    // 验证调用
    QVERIFY(mockSystemdService->calledMethods.contains("ThawUnit"));
    QCOMPARE(mockSystemdService->calledMethods.count("ThawUnit"), 1);

    // 验证参数
    int thawIdx = mockSystemdService->calledMethods.indexOf("ThawUnit");
    QVariantList thawArgs = mockSystemdService->calledArguments.at(thawIdx);
    QCOMPARE(thawArgs.at(0).toString(), QString::fromStdString(unitName));
}

void TestSystemdDbusInterface::test_setSystemdUnitPropertyEnabled()
{
    QString unitName = "test.service";
    QString propertyName = "CPUAccounting";
    bool enabled = true;

    systemdDbus->setSystemdUnitPropertyEnabled(unitName, propertyName, enabled);

    // 等待异步调用完成
    QTest::qWait(100);

    // 验证调用
    QVERIFY(mockSystemdService->calledMethods.contains("SetUnitProperties"));
    QCOMPARE(mockSystemdService->calledMethods.count("SetUnitProperties"), 1);

    // 验证参数
    int idx = mockSystemdService->calledMethods.indexOf("SetUnitProperties");
    QVariantList args = mockSystemdService->calledArguments.at(idx);
    QCOMPARE(args.at(0).toString(), unitName);
    QCOMPARE(args.at(1).toBool(), true);
}

void TestSystemdDbusInterface::test_getUnitProcessIds()
{
    std::string unitName = "app-test.scope";

    std::vector<int> pids = systemdDbus->getUnitProcessIds(unitName);

    // 验证调用
    QVERIFY(mockSystemdService->calledMethods.contains("GetUnitProcesses"));
    QCOMPARE(mockSystemdService->calledMethods.count("GetUnitProcesses"), 1);

    // 验证参数
    int idx = mockSystemdService->calledMethods.indexOf("GetUnitProcesses");
    QVariantList args = mockSystemdService->calledArguments.at(idx);
    QCOMPARE(args.at(0).toString(), QString::fromStdString(unitName));

    // 验证返回值
    // 这里的返回值可能无法正确验证，因为GetUnitProcesses的返回类型比较复杂
    // 实际测试中可能需要更多的调整
}

QTEST_MAIN(TestSystemdDbusInterface)

#include "tst_systemddbusinterface.moc"