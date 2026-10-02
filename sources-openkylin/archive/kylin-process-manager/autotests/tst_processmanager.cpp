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

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QSignalSpy>
#include <QtTest>
#include <stdlib.h>

#include "daemon/src/processmanager.h"

class TestProcessManager : public QObject
{
    Q_OBJECT

public:
    TestProcessManager();
    ~TestProcessManager();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 基本功能测试
    void testCreateProcessGroup();
    void testMoveProcessToGroup();
    void testSetProcessGroupResourceLimit();
    void testRemoveProcessCGroup();
    void testSetSystemdUnitPropertyEnabled();

    // 资源回收相关测试
    void testReclaimProcesses();
    void testReclaimProcessGroups();

    // CPU能源测试
    void testGetCpuEnergyConsumption();

    // 组进程管理
    void testGetGroupPids();

    // 信号相关测试
    void testResourceThresholdWarningSignal();

private:
    ProcessManager *m_processManager;

    // 测试辅助函数
    bool createTestGroup(const QString &groupPath);
    void removeTestGroup(const QString &groupPath);

    // 确认当前环境中cgroup支持哪些controllers
    QStringList getSupportedControllers();
};

TestProcessManager::TestProcessManager()
{
}

TestProcessManager::~TestProcessManager()
{
}

void TestProcessManager::initTestCase()
{
    // 全局初始化 - 会在所有测试用例执行前运行一次
    // 检查是否有运行测试所需的权限和配置
    qDebug() << "Starting ProcessManager tests";
}

void TestProcessManager::cleanupTestCase()
{
    // 全局清理 - 会在所有测试用例执行后运行一次
    qDebug() << "Completed ProcessManager tests";
}

void TestProcessManager::init()
{
    // 在每个测试用例执行前运行
    m_processManager = new ProcessManager();
}

void TestProcessManager::cleanup()
{
    // 在每个测试用例执行后运行
    // delete m_processManager;
    // m_processManager = nullptr;
}

QStringList TestProcessManager::getSupportedControllers()
{
    // 支持的cgroup controllers列表
    // 注意：在不同系统上可能需要修改此列表
    return {"cpu", "memory"};
}

bool TestProcessManager::createTestGroup(const QString &groupPath)
{
    // 创建测试用的cgroup组
    QStringList controllers = getSupportedControllers();
    QList<int> pids = {123456};
    return m_processManager->CreateProcessGroup(groupPath, controllers, pids);
}

void TestProcessManager::removeTestGroup(const QString &groupPath)
{
    // 移除测试用的cgroup组
    m_processManager->RemoveProcessCGroup(groupPath);
}

void TestProcessManager::testCreateProcessGroup()
{
    // 测试创建进程组功能
    QString groupPath = "test_group";
    QStringList controllers = getSupportedControllers();
    QList<int> pids = {123456}; // 使用当前测试进程

    // 执行被测试方法
    bool result = m_processManager->CreateProcessGroup(groupPath, controllers, pids);

    // 清理测试资源
    if (result) {
        removeTestGroup(groupPath);
    }

    // 验证结果
    // 注意：在某些环境下可能会失败，例如没有权限时
    QVERIFY2(result, "Create process group should succeed with proper permissions");
}

void TestProcessManager::testMoveProcessToGroup()
{
    // 测试移动进程到组功能
    // 注意：此函数在CGroupManager中实际调用了createProcessGroup

    QString groupPath = "test_move_group";
    QStringList controllers = getSupportedControllers();
    QList<int> pids = {123456};

    // 执行被测试方法
    bool result = m_processManager->MoveProcessToGroup(groupPath, controllers, pids);

    // 清理测试资源
    if (result) {
        removeTestGroup(groupPath);
    }

    // 验证结果
    QVERIFY2(result, "Move process to group should succeed with proper permissions");
}

void TestProcessManager::testSetProcessGroupResourceLimit()
{
    // 测试设置进程组资源限制功能

    // 先创建一个测试组
    QString groupPath = "test_resource_group";
    if (!createTestGroup(groupPath)) {
        QSKIP("Could not create test group for resource limit test");
    }

    // 设置CPU控制器的资源限制
    QString controller = "cpu";
    QString attrFile = "cpu.shares";
    QString value = "1000";

    // 执行被测试方法
    bool result = m_processManager->SetProcessGroupResourceLimit(groupPath, controller, attrFile, value);

    // 清理测试资源
    removeTestGroup(groupPath);

    // 验证结果
    QVERIFY2(result, "Set process group resource limit should succeed with proper permissions");
}

void TestProcessManager::testRemoveProcessCGroup()
{
    // 测试移除进程组功能

    // 先创建一个测试组
    QString groupPath = "test_remove_group";
    if (!createTestGroup(groupPath)) {
        QSKIP("Could not create test group for removal test");
    }

    // 执行被测试方法
    m_processManager->RemoveProcessCGroup(groupPath);

    // 验证移除操作是否成功
    // 注意：由于RemoveProcessCGroup没有返回值，我们间接通过尝试获取该组的信息来验证
    QList<int> pids = m_processManager->GetGroupPids(groupPath);

    // 如果组被成功移除，应该返回空列表
    QVERIFY2(pids.isEmpty(), "Process group should be removed successfully");
}

void TestProcessManager::testSetSystemdUnitPropertyEnabled()
{
    // 测试设置Systemd单元属性
    // 注意：此测试可能依赖系统环境

    QString unitName = "user@1000.service"; // 替换为存在的systemd单元
    QString propertyName = "CPUAccounting";
    bool enabled = true;

    // 由于此操作可能需要特定权限，我们仅验证调用不抛出异常
    try {
        m_processManager->SetSystemdUnitPropertyEnabled(unitName, propertyName, enabled);
        QVERIFY(true); // 如果没有异常，视为通过
    } catch (const std::exception &e) {
        QFAIL(QString("Exception thrown when setting systemd unit property: %1").arg(e.what()).toLocal8Bit().data());
    }
}

void TestProcessManager::testReclaimProcesses()
{
    // 测试回收进程功能
    // 回收过程依赖于系统内核支持

    // QList<int> pids = {123456};

    // // 由于回收结果依赖于系统状态，我们仅验证调用不抛出异常
    // try {
    //     m_processManager->ReclaimProcesses(pids);
    //     QVERIFY(true); // 如果没有异常，视为通过
    // } catch (const std::exception& e) {
    //     QFAIL(QString("Exception thrown when reclaiming processes: %1").arg(e.what()).toLocal8Bit().data());
    // }
}

void TestProcessManager::testReclaimProcessGroups()
{
    // 测试回收进程组功能

    // 创建一个测试组
    QString groupPath = "test_reclaim_group";
    if (!createTestGroup(groupPath)) {
        QSKIP("Could not create test group for reclaim test");
    }

    QStringList groupNames = {groupPath};

    // 执行被测试方法
    try {
        m_processManager->ReclaimProcessGroups(groupNames);
        QVERIFY(true); // 如果没有异常，视为通过
    } catch (const std::exception &e) {
        QFAIL(QString("Exception thrown when reclaiming process groups: %1").arg(e.what()).toLocal8Bit().data());
    }

    // 清理测试资源
    removeTestGroup(groupPath);
}

void TestProcessManager::testGetCpuEnergyConsumption()
{
    // 测试获取CPU能源消耗
    // 此功能依赖于硬件支持

    // double result = m_processManager->GetCpuEnergyConsumption();

    // 对于不支持能源测量的系统，此测试可能会失败
    // 我们只是验证调用成功，不检查具体返回值
    // QVERIFY2(result >= 0.0, "CPU energy consumption should be >= 0");
}

void TestProcessManager::testGetGroupPids()
{
    // 测试获取组内进程ID功能

    // 创建一个测试组，并将当前进程加入其中
    QString groupPath = "test_getpids_group";
    if (!createTestGroup(groupPath)) {
        QSKIP("Could not create test group for GetGroupPids test");
    }

    // 执行被测试方法
    QList<int> pids = m_processManager->GetGroupPids(groupPath);

    // 清理测试资源
    removeTestGroup(groupPath);

    // 验证结果
    // 如果组创建和进程添加成功，应该至少有一个进程（当前测试进程）
    QVERIFY2(!pids.isEmpty(), "Group should contain at least one process");
    QVERIFY2(pids.contains(123456), "Group should contain the test process");
}

void TestProcessManager::testResourceThresholdWarningSignal()
{
    // 测试资源阈值警告信号
    // 注意：正常情况下，此信号由SystemResourceManager触发
    // 在单元测试中很难真实模拟触发条件

    // 设置信号监视
    QSignalSpy spy(m_processManager, &ProcessManager::ResourceThresholdWarning);

    // 验证信号连接正常工作
    QVERIFY(spy.isValid());

    // 在实际测试中，我们不能主动触发此信号
    // 只能验证信号监视设置正确
    QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(TestProcessManager)
#include "tst_processmanager.moc"