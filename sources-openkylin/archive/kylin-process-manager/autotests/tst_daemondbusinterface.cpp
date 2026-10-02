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

#include "../base/daemondbusinterface.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QSignalSpy>
#include <QtTest>

// 用于模拟DBus服务的测试类
class MockDBusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.ProcessManagerDaemon")

public:
    explicit MockDBusService(QObject *parent = nullptr)
        : QObject(parent)
    {
        QDBusConnection::sessionBus().registerObject("/com/kylin/ProcessManagerDaemon", this,
                                                     QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
        QDBusConnection::sessionBus().registerService("com.kylin.ProcessManagerDaemon");
    }

    ~MockDBusService()
    {
        QDBusConnection::sessionBus().unregisterService("com.kylin.ProcessManagerDaemon");
        QDBusConnection::sessionBus().unregisterObject("/com/kylin/ProcessManagerDaemon");
    }

    // 记录方法调用情况
    QStringList calledMethods;
    QList<QVariantList> calledArguments;

public Q_SLOTS:
    Q_SCRIPTABLE void CreateProcessGroup(const QString &path, const QStringList &controllers, const QList<int> &pids)
    {
        calledMethods.append("CreateProcessGroup");
        QVariantList args;
        args << path << QVariant::fromValue(controllers) << QVariant::fromValue(pids);
        calledArguments.append(args);
    }

    Q_SCRIPTABLE void MoveProcessToGroup(const QString &path, const QStringList &controllers, const QList<int> &pids)
    {
        calledMethods.append("MoveProcessToGroup");
        QVariantList args;
        args << path << QVariant::fromValue(controllers) << QVariant::fromValue(pids);
        calledArguments.append(args);
    }

    Q_SCRIPTABLE void SetProcessGroupResourceLimit(const QString &path, const QString &controller,
                                                   const QString &file, const QString &value)
    {
        calledMethods.append("SetProcessGroupResourceLimit");
        QVariantList args;
        args << path << controller << file << value;
        calledArguments.append(args);
    }

    Q_SCRIPTABLE void SetSystemdUnitPropertyEnabled(const QString &unitName, const QString &propertyName, bool enabled)
    {
        calledMethods.append("SetSystemdUnitPropertyEnabled");
        QVariantList args;
        args << unitName << propertyName << enabled;
        calledArguments.append(args);
    }

    Q_SCRIPTABLE double GetCpuEnergyConsumption()
    {
        calledMethods.append("GetCpuEnergyConsumption");
        calledArguments.append(QVariantList());
        return 123.45; // 返回模拟的能源消耗值
    }

    Q_SCRIPTABLE void ReclaimProcesses(const QList<int> &pids)
    {
        calledMethods.append("ReclaimProcesses");
        QVariantList args;
        args << QVariant::fromValue(pids);
        calledArguments.append(args);
    }

    Q_SCRIPTABLE void ReclaimProcessGroups(const QStringList &groupNames)
    {
        calledMethods.append("ReclaimProcessGroups");
        QVariantList args;
        args << QVariant::fromValue(groupNames);
        calledArguments.append(args);
    }

Q_SIGNALS:
    Q_SCRIPTABLE void ResourceThresholdWarning(const QString &resource, int level);
};

class TestDaemonDbusInterface : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_createProcessGroup();
    void test_moveProcessToGroup();
    void test_setProcessGroupResourceLimit();
    void test_setSystemdUnitPropertyEnabled();
    void test_getCpuEnergyConsumption();
    void test_reclaimProcesses();
    void test_reclaimProcessGroups();
    void test_resourceThresholdWarningSignal();

private:
    MockDBusService *mockService;
    DaemonDbusInterface *dbus;
};

void TestDaemonDbusInterface::initTestCase()
{
    // 启动模拟的DBus服务
    mockService = new MockDBusService(this);

    // 创建DBus接口实例
    dbus = new DaemonDbusInterface(this);

    // 等待一段时间确保DBus服务注册完成
    QTest::qWait(100);
}

void TestDaemonDbusInterface::cleanupTestCase()
{
    delete dbus;
    delete mockService;
}

void TestDaemonDbusInterface::init()
{
    // 每个测试前清空调用记录
    mockService->calledMethods.clear();
    mockService->calledArguments.clear();
}

void TestDaemonDbusInterface::cleanup()
{
    // 测试后的清理工作
}

void TestDaemonDbusInterface::test_createProcessGroup()
{
    std::string path = "/sys/fs/cgroup/test";
    std::vector<std::string> controllers = {"cpu", "memory"};
    std::vector<int> pids = {1234, 5678};

    dbus->createProcessGroup(path, controllers, pids);

    // 等待异步调用完成
    QTest::qWait(100);

    QVERIFY(mockService->calledMethods.contains("CreateProcessGroup"));
    QCOMPARE(mockService->calledMethods.count("CreateProcessGroup"), 1);

    // 验证参数
    int idx = mockService->calledMethods.indexOf("CreateProcessGroup");
    QVariantList args = mockService->calledArguments.at(idx);
    QCOMPARE(args.at(0).toString(), QString::fromStdString(path));
    QCOMPARE(args.at(1).value<QStringList>().size(), controllers.size());
    QCOMPARE(args.at(2).value<QList<int>>().size(), pids.size());
}

void TestDaemonDbusInterface::test_moveProcessToGroup()
{
    std::string path = "/sys/fs/cgroup/test";
    std::vector<std::string> controllers = {"cpu", "memory"};
    std::vector<int> pids = {1234, 5678};

    dbus->moveProcessToGroup(path, controllers, pids);

    // 等待异步调用完成
    QTest::qWait(100);

    QVERIFY(mockService->calledMethods.contains("MoveProcessToGroup"));
    QCOMPARE(mockService->calledMethods.count("MoveProcessToGroup"), 1);

    // 验证参数
    int idx = mockService->calledMethods.indexOf("MoveProcessToGroup");
    QVariantList args = mockService->calledArguments.at(idx);
    QCOMPARE(args.at(0).toString(), QString::fromStdString(path));
    QCOMPARE(args.at(1).value<QStringList>().size(), controllers.size());
    QCOMPARE(args.at(2).value<QList<int>>().size(), pids.size());
}

void TestDaemonDbusInterface::test_setProcessGroupResourceLimit()
{
    std::string path = "/sys/fs/cgroup/test";
    std::string controller = "cpu";
    std::string file = "cpu.shares";
    std::string value = "1000";

    dbus->setProcessGroupResourceLimit(path, controller, file, value);

    // 等待异步调用完成
    QTest::qWait(100);

    QVERIFY(mockService->calledMethods.contains("SetProcessGroupResourceLimit"));
    QCOMPARE(mockService->calledMethods.count("SetProcessGroupResourceLimit"), 1);

    // 验证参数
    int idx = mockService->calledMethods.indexOf("SetProcessGroupResourceLimit");
    QVariantList args = mockService->calledArguments.at(idx);
    QCOMPARE(args.at(0).toString(), QString::fromStdString(path));
    QCOMPARE(args.at(1).toString(), QString::fromStdString(controller));
    QCOMPARE(args.at(2).toString(), QString::fromStdString(file));
    QCOMPARE(args.at(3).toString(), QString::fromStdString(value));
}

void TestDaemonDbusInterface::test_setSystemdUnitPropertyEnabled()
{
    std::string unitName = "test.service";
    std::string propertyName = "CPUQuota";
    bool enabled = true;

    dbus->setSystemdUnitPropertyEnabled(unitName, propertyName, enabled);

    // 等待异步调用完成
    QTest::qWait(100);

    QVERIFY(mockService->calledMethods.contains("SetSystemdUnitPropertyEnabled"));
    QCOMPARE(mockService->calledMethods.count("SetSystemdUnitPropertyEnabled"), 1);

    // 验证参数
    int idx = mockService->calledMethods.indexOf("SetSystemdUnitPropertyEnabled");
    QVariantList args = mockService->calledArguments.at(idx);
    QCOMPARE(args.at(0).toString(), QString::fromStdString(unitName));
    QCOMPARE(args.at(1).toString(), QString::fromStdString(propertyName));
    QCOMPARE(args.at(2).toBool(), enabled);
}

void TestDaemonDbusInterface::test_getCpuEnergyConsumption()
{
    double consumption = dbus->getCpuEnergyConsumption();

    QVERIFY(mockService->calledMethods.contains("GetCpuEnergyConsumption"));
    QCOMPARE(mockService->calledMethods.count("GetCpuEnergyConsumption"), 1);

    // 验证返回的能源消耗值与模拟值一致
    QCOMPARE(consumption, 123.45);
}

void TestDaemonDbusInterface::test_reclaimProcesses()
{
    std::vector<int> pids = {1234, 5678};

    dbus->reclaimProcesses(pids);

    // 等待异步调用完成
    QTest::qWait(100);

    QVERIFY(mockService->calledMethods.contains("ReclaimProcesses"));
    QCOMPARE(mockService->calledMethods.count("ReclaimProcesses"), 1);

    // 验证参数
    int idx = mockService->calledMethods.indexOf("ReclaimProcesses");
    QVariantList args = mockService->calledArguments.at(idx);
    QCOMPARE(args.at(0).value<QList<int>>().size(), pids.size());
}

void TestDaemonDbusInterface::test_reclaimProcessGroups()
{
    std::vector<std::string> groupNames = {"group1", "group2"};

    dbus->reclaimProcessGroups(groupNames);

    // 等待异步调用完成
    QTest::qWait(100);

    QVERIFY(mockService->calledMethods.contains("ReclaimProcessGroups"));
    QCOMPARE(mockService->calledMethods.count("ReclaimProcessGroups"), 1);

    // 验证参数
    int idx = mockService->calledMethods.indexOf("ReclaimProcessGroups");
    QVariantList args = mockService->calledArguments.at(idx);
    QCOMPARE(args.at(0).value<QStringList>().size(), groupNames.size());
}

void TestDaemonDbusInterface::test_resourceThresholdWarningSignal()
{
    // 监听DaemonDbusInterface的信号
    QSignalSpy spy(dbus, SIGNAL(ResourceThresholdWarning(std::string, int)));

    // 模拟DBus服务发出信号
    QString resourceName = "memory";
    int warningLevel = 2;
    Q_EMIT mockService->ResourceThresholdWarning(resourceName, warningLevel);

    // 等待信号传播
    QTest::qWait(100);

    // 验证信号被正确接收
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(1).toInt(), warningLevel);
}

QTEST_MAIN(TestDaemonDbusInterface)

#include "tst_daemondbusinterface.moc"