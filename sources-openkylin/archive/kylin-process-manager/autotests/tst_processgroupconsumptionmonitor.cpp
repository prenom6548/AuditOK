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

#include "core/consumptionmonitor/processgroupconsumptionmonitor.h"
#include "core/energymeter/energymeter.h"
#include "core/systemenery/systemenergy.h"
#include <QtTest/QtTest>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

// 创建系统能源接口的Mock实现
class MockSystemEnergy : public SystemEnergy
{
public:
    MockSystemEnergy()
        : m_energyValue(100.0)
        , m_dischargeRate(20.0)
        , m_timeToEmpty(300)
    {
    }

    double energy() override
    {
        return m_energyValue;
    }
    double dischargeRate() override
    {
        return m_dischargeRate;
    }
    int timeToEmpty() override
    {
        return m_timeToEmpty;
    }

    void setEnergyValue(double value)
    {
        m_energyValue = value;
    }
    void setDischargeRate(double rate)
    {
        m_dischargeRate = rate;
    }
    void setTimeToEmpty(int time)
    {
        m_timeToEmpty = time;
    }

private:
    double m_energyValue;
    double m_dischargeRate;
    int m_timeToEmpty;
};

// 创建能源计量器的Mock实现
class MockEnergyMeter : public EnergyMeter
{
public:
    MockEnergyMeter()
        : m_startEnergy(0.0)
        , m_endEnergy(0.0)
        , m_measurementStarted(false)
        , m_measurementEnded(false)
        , m_energyConsumptionValue(0.0)
        , m_energyConsumptionRateValue(0.0)
    {
    }

    void startMeasurement() override
    {
        m_measurementStarted = true;
        m_startTimePoint = std::chrono::high_resolution_clock::now();
    }

    void endMeasurement() override
    {
        m_measurementEnded = true;
        m_endTimePoint = std::chrono::high_resolution_clock::now();
    }

    double energyConsumption() const override
    {
        return m_energyConsumptionValue;
    }

    double energyConsumptionRate() const override
    {
        return m_energyConsumptionRateValue;
    }

    void setEnergyConsumption(double value)
    {
        m_energyConsumptionValue = value;
    }

    void setEnergyConsumptionRate(double value)
    {
        m_energyConsumptionRateValue = value;
    }

    bool isMeasurementStarted() const
    {
        return m_measurementStarted;
    }
    bool isMeasurementEnded() const
    {
        return m_measurementEnded;
    }

    void reset()
    {
        m_measurementStarted = false;
        m_measurementEnded = false;
    }

private:
    double m_startEnergy;
    double m_endEnergy;
    bool m_measurementStarted;
    bool m_measurementEnded;
    double m_energyConsumptionValue;
    double m_energyConsumptionRateValue;
    std::chrono::high_resolution_clock::time_point m_startTimePoint;
    std::chrono::high_resolution_clock::time_point m_endTimePoint;
};

// 进程助手，帮助创建实际运行的进程用于测试
class ProcessHelper
{
public:
    static std::vector<int> startDummyProcesses(int count)
    {
        std::vector<int> pids;
        for (int i = 0; i < count; ++i) {
            pid_t pid = fork();
            if (pid == 0) {
                // 子进程，执行耗CPU的操作
                while (true) {
                    // 简单的循环计算，消耗CPU
                    volatile int sum = 0;
                    for (int j = 0; j < 1000000; ++j) {
                        sum += j;
                    }
                }
                exit(0); // 永远不会执行到这里
            } else if (pid > 0) {
                // 父进程，保存PID
                pids.push_back(pid);
            }
        }
        return pids;
    }

    static void killProcesses(const std::vector<int> &pids)
    {
        for (int pid : pids) {
            kill(pid, SIGKILL);
        }
    }
};

// 测试类
class TestProcessGroupConsumptionMonitor : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testAddProcessGroup();
    void testCpuEnergyConsumptionRate();
    void testMultipleProcessGroups();
    void testEmptyProcessGroup();
    void testUpdatePidsCallback();
    void testMonitorStopsWhenEmpty();

private:
    std::shared_ptr<MockSystemEnergy> m_mockSystemEnergy;
    std::shared_ptr<MockEnergyMeter> m_mockEnergyMeter;
    std::unique_ptr<ProcessGroupConsumptionMonitor> m_monitor;
    std::vector<int> m_testProcesses;
};

void TestProcessGroupConsumptionMonitor::initTestCase()
{
    // 全局初始化，仅执行一次
}

void TestProcessGroupConsumptionMonitor::cleanupTestCase()
{
    // 全局清理，仅执行一次
}

void TestProcessGroupConsumptionMonitor::init()
{
    // 每个测试前初始化
    m_mockSystemEnergy = std::make_shared<MockSystemEnergy>();
    m_mockEnergyMeter = std::make_shared<MockEnergyMeter>();

    // 创建监视器，设置1秒的监控间隔和5个数据点的保留数量
    m_monitor = std::unique_ptr<ProcessGroupConsumptionMonitor>(
        new ProcessGroupConsumptionMonitor(
            m_mockSystemEnergy,
            m_mockEnergyMeter,
            1, // 监控间隔（秒）
            5 // 保留的数据点数量
            ));

    // 启动测试进程
    m_testProcesses = ProcessHelper::startDummyProcesses(2);
}

void TestProcessGroupConsumptionMonitor::cleanup()
{
    // 每个测试后清理
    m_monitor.reset();

    // 停止测试进程
    ProcessHelper::killProcesses(m_testProcesses);
    m_testProcesses.clear();
}

void TestProcessGroupConsumptionMonitor::testAddProcessGroup()
{
    // 测试添加进程组
    std::string groupId = "test_group";

    // 验证初始状态下能量消耗率为0
    QCOMPARE(m_monitor->cpuEnergyConsumptionRate(groupId), 0.0);

    // 添加进程组
    m_monitor->addProcessGroup(
        groupId,
        m_testProcesses,
        [](const std::string &id) -> std::vector<int> {
        return {}; // 空的回调，模拟进程组移除
    });

    // 验证添加进程组后能量计量器已开始测量
    QVERIFY(m_mockEnergyMeter->isMeasurementStarted());

    // 等待监控间隔结束
    QTest::qWait(1100); // 略大于1秒的监控间隔

    // 验证能量计量器的测量已结束并重新开始
    QVERIFY(m_mockEnergyMeter->isMeasurementEnded());
}

void TestProcessGroupConsumptionMonitor::testCpuEnergyConsumptionRate()
{
    // 测试CPU能量消耗率计算
    std::string groupId = "test_group";

    // 设置模拟的能量消耗率
    m_mockEnergyMeter->setEnergyConsumptionRate(10.0);

    // 添加进程组，并保持进程组存活（回调返回相同的PIDs）
    m_monitor->addProcessGroup(
        groupId,
        m_testProcesses,
        [this](const std::string &id) -> std::vector<int> {
        return m_testProcesses;
    });

    // 等待多个监控周期
    QTest::qWait(3100); // 3个监控周期

    // 验证能量消耗率是否被正确计算
    // 由于ProcessGroupConsumptionMonitor内部会根据进程CPU时间比例计算实际消耗率，
    // 这里我们只能验证返回的值是有限的，不能精确预测具体值
    double rate = m_monitor->cpuEnergyConsumptionRate(groupId);
    QVERIFY(std::isfinite(rate));
    qDebug() << "CPU energy consumption rate for process group:" << rate;
}

void TestProcessGroupConsumptionMonitor::testMultipleProcessGroups()
{
    // 测试多个进程组
    std::string groupId1 = "test_group_1";
    std::string groupId2 = "test_group_2";

    // 创建额外的测试进程
    std::vector<int> testProcesses2 = ProcessHelper::startDummyProcesses(1);

    // 设置模拟的能量消耗率
    m_mockEnergyMeter->setEnergyConsumptionRate(10.0);

    // 添加两个进程组
    m_monitor->addProcessGroup(
        groupId1,
        m_testProcesses,
        [this](const std::string &id) -> std::vector<int> {
        return m_testProcesses;
    });

    m_monitor->addProcessGroup(
        groupId2,
        testProcesses2,
        [&testProcesses2](const std::string &id) -> std::vector<int> {
        return testProcesses2;
    });

    // 等待多个监控周期
    QTest::qWait(3100); // 3个监控周期

    // 验证两个进程组都有能量消耗率
    double rate1 = m_monitor->cpuEnergyConsumptionRate(groupId1);
    double rate2 = m_monitor->cpuEnergyConsumptionRate(groupId2);

    QVERIFY(std::isfinite(rate1));
    QVERIFY(std::isfinite(rate2));

    qDebug() << "Group 1 rate:" << rate1 << ", Group 2 rate:" << rate2;

    // 清理额外的测试进程
    ProcessHelper::killProcesses(testProcesses2);
}

void TestProcessGroupConsumptionMonitor::testEmptyProcessGroup()
{
    // 测试空进程组的处理
    std::string groupId = "empty_group";

    // 尝试添加一个空的进程组
    m_monitor->addProcessGroup(
        groupId,
        {},
        [](const std::string &id) -> std::vector<int> {
        return {};
    });

    // 验证能量消耗率为0
    QCOMPARE(m_monitor->cpuEnergyConsumptionRate(groupId), 0.0);

    // 尝试添加一个有效的进程组，但ID为空
    m_monitor->addProcessGroup(
        "",
        m_testProcesses,
        [this](const std::string &id) -> std::vector<int> {
        return m_testProcesses;
    });

    // 验证能量消耗率为0
    QCOMPARE(m_monitor->cpuEnergyConsumptionRate(""), 0.0);
}

void TestProcessGroupConsumptionMonitor::testUpdatePidsCallback()
{
    // 测试PID更新回调功能
    std::string groupId = "update_test_group";
    bool callbackCalled = false;

    // 添加进程组，回调会更新标志并返回不同的PIDs
    m_monitor->addProcessGroup(
        groupId,
        m_testProcesses,
        [this, &callbackCalled](const std::string &id) -> std::vector<int> {
        callbackCalled = true;
        // 第一次调用时返回原始PIDs，之后返回空列表（模拟进程终止）
        static bool firstCall = true;
        if (firstCall) {
            firstCall = false;
            return m_testProcesses;
        }
        return {};
    });

    // 等待监控周期
    QTest::qWait(1100);

    // 验证回调被调用
    QVERIFY(callbackCalled);

    // 等待另一个周期，此时回调应返回空列表
    callbackCalled = false;
    QTest::qWait(1100);

    // 验证回调再次被调用
    QVERIFY(callbackCalled);

    // 由于返回空列表，该进程组应被移除，能量消耗率应为0
    QCOMPARE(m_monitor->cpuEnergyConsumptionRate(groupId), 0.0);
}

void TestProcessGroupConsumptionMonitor::testMonitorStopsWhenEmpty()
{
    // 测试当所有进程组都被移除时，监控是否停止
    std::string groupId = "stop_test_group";

    // 添加进程组，但回调会立即返回空列表
    m_monitor->addProcessGroup(
        groupId,
        m_testProcesses,
        [](const std::string &id) -> std::vector<int> {
        return {};
    });

    // 验证能量计量器已开始测量
    QVERIFY(m_mockEnergyMeter->isMeasurementStarted());

    // 等待监控周期
    QTest::qWait(1100);

    // 由于回调返回空列表，监控应该停止
    // 重置能量计量器的状态标志
    m_mockEnergyMeter->reset();

    // 等待足够长的时间
    QTest::qWait(2000);

    // 验证能量计量器没有再次开始测量
    QVERIFY(!m_mockEnergyMeter->isMeasurementStarted());
}

QTEST_MAIN(TestProcessGroupConsumptionMonitor)
#include "tst_processgroupconsumptionmonitor.moc"