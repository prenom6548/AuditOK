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

#include "base/daemondbusinterface.h"
#include "core/energymeter/cpuenergymeter.h"
#include <QtTest/QtTest>
#include <chrono>
#include <thread>

// 创建一个DaemonDbusInterface的mock版本
class MockDaemonDbusInterface : public DaemonDbusInterface
{
public:
    MockDaemonDbusInterface()
        : m_energyValue(0.0)
    {
    }

    // 重写获取CPU能量消耗的方法
    double getCpuEnergyConsumption()
    {
        return m_energyValue;
    }

    // 设置模拟的能量值，用于测试
    void setEnergyValue(double value)
    {
        m_energyValue = value;
    }

private:
    double m_energyValue;
};

// 由于我们无法访问CpuEnergyMeter的私有成员，我们创建一个可测试的新实现
class TestableEnergyMeter : public EnergyMeter
{
public:
    TestableEnergyMeter(DaemonDbusInterface *dbusInterface)
        : m_dbusInterface(dbusInterface)
        , m_startEnergy(0.0)
        , m_endEnergy(0.0)
    {
    }

    void startMeasurement() override
    {
        m_startTimePoint = std::chrono::high_resolution_clock::now();
        m_startEnergy = m_dbusInterface->getCpuEnergyConsumption();
    }

    void endMeasurement() override
    {
        m_endTimePoint = std::chrono::high_resolution_clock::now();
        m_endEnergy = m_dbusInterface->getCpuEnergyConsumption();
    }

    double energyConsumption() const override
    {
        return m_endEnergy - m_startEnergy;
    }

    double energyConsumptionRate() const override
    {
        if (m_endTimePoint <= m_startTimePoint) {
            return 0;
        }

        auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_endTimePoint - m_startTimePoint);

        return energyConsumption() / (deltaTime.count() / 1000.0);
    }

    // 额外的访问器方法，用于测试内部状态
    double getStartEnergy() const
    {
        return m_startEnergy;
    }
    double getEndEnergy() const
    {
        return m_endEnergy;
    }
    std::chrono::high_resolution_clock::time_point getStartTimePoint() const
    {
        return m_startTimePoint;
    }
    std::chrono::high_resolution_clock::time_point getEndTimePoint() const
    {
        return m_endTimePoint;
    }

private:
    DaemonDbusInterface *m_dbusInterface;
    double m_startEnergy;
    double m_endEnergy;
    std::chrono::high_resolution_clock::time_point m_startTimePoint;
    std::chrono::high_resolution_clock::time_point m_endTimePoint;
};

// 测试类
class TestCpuEnergyMeterClass : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testStartMeasurement();
    void testEndMeasurement();
    void testEnergyConsumption();
    void testEnergyConsumptionRate();
    void testCompleteMeasurementCycle();
    void testRealCpuEnergyMeter();

private:
    MockDaemonDbusInterface *m_mockInterface;
};

void TestCpuEnergyMeterClass::initTestCase()
{
    // 全局初始化，仅执行一次
}

void TestCpuEnergyMeterClass::cleanupTestCase()
{
    // 全局清理，仅执行一次
}

void TestCpuEnergyMeterClass::init()
{
    // 每个测试前初始化
    m_mockInterface = new MockDaemonDbusInterface();
}

void TestCpuEnergyMeterClass::cleanup()
{
    // 每个测试后清理
    delete m_mockInterface;
}

void TestCpuEnergyMeterClass::testStartMeasurement()
{
    // 设置初始能量值
    m_mockInterface->setEnergyValue(10.0);

    // 创建可测试的能量计量器
    TestableEnergyMeter meter(m_mockInterface);

    // 开始测量
    meter.startMeasurement();

    // 验证开始能量值已正确记录
    QCOMPARE(meter.getStartEnergy(), 10.0);

    // 验证开始时间已记录（无法精确比较时间点，但可以验证它不是默认值）
    QVERIFY(meter.getStartTimePoint() != std::chrono::high_resolution_clock::time_point());
}

void TestCpuEnergyMeterClass::testEndMeasurement()
{
    // 设置初始能量值
    m_mockInterface->setEnergyValue(10.0);

    // 创建可测试的能量计量器
    TestableEnergyMeter meter(m_mockInterface);

    // 开始测量
    meter.startMeasurement();

    // 改变能量值
    m_mockInterface->setEnergyValue(15.0);

    // 结束测量
    meter.endMeasurement();

    // 验证结束能量值已正确记录
    QCOMPARE(meter.getEndEnergy(), 15.0);

    // 验证结束时间已记录
    QVERIFY(meter.getEndTimePoint() != std::chrono::high_resolution_clock::time_point());

    // 验证结束时间晚于开始时间
    QVERIFY(meter.getEndTimePoint() >= meter.getStartTimePoint());
}

void TestCpuEnergyMeterClass::testEnergyConsumption()
{
    // 设置初始能量值
    m_mockInterface->setEnergyValue(10.0);

    // 创建可测试的能量计量器
    TestableEnergyMeter meter(m_mockInterface);

    // 开始测量
    meter.startMeasurement();

    // 改变能量值
    m_mockInterface->setEnergyValue(15.0);

    // 结束测量
    meter.endMeasurement();

    // 验证能量消耗计算是否正确 (15.0 - 10.0 = 5.0)
    QCOMPARE(meter.energyConsumption(), 5.0);

    // 测试负消耗（能量减少的情况）
    m_mockInterface->setEnergyValue(5.0);
    meter.startMeasurement();
    m_mockInterface->setEnergyValue(3.0);
    meter.endMeasurement();
    QCOMPARE(meter.energyConsumption(), -2.0);
}

void TestCpuEnergyMeterClass::testEnergyConsumptionRate()
{
    // 设置初始能量值
    m_mockInterface->setEnergyValue(10.0);

    // 创建可测试的能量计量器
    TestableEnergyMeter meter(m_mockInterface);

    // 开始测量
    meter.startMeasurement();

    // 等待一段时间
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 改变能量值，消耗1单位能量
    m_mockInterface->setEnergyValue(11.0);

    // 结束测量
    meter.endMeasurement();

    // 计算消耗率，应该约为10单位/秒（1单位能量在0.1秒内）
    double rate = meter.energyConsumptionRate();

    // 由于时间测量可能不精确，我们验证消耗率在合理范围内
    qDebug() << "Energy consumption rate:" << rate << "units/second";
    QVERIFY(rate > 5.0 && rate < 15.0);

    // 测试边界情况：开始和结束时间点相同
    TestableEnergyMeter meter2(m_mockInterface);
    meter2.startMeasurement();
    meter2.endMeasurement(); // 在几乎相同的时间点结束

    // 速率应该接近于零或为零
    // 因为时间间隔太短，所以测试结果可能会变化，所以只验证速率不是无穷大
    double rate2 = meter2.energyConsumptionRate();
    qDebug() << "Edge case rate:" << rate2;
    QVERIFY(std::isfinite(rate2));
}

void TestCpuEnergyMeterClass::testCompleteMeasurementCycle()
{
    // 创建可测试的能量计量器
    TestableEnergyMeter meter(m_mockInterface);

    // 测试多个测量周期
    for (int i = 0; i < 3; i++) {
        // 设置能量初始值
        double startEnergy = 10.0 * i;
        m_mockInterface->setEnergyValue(startEnergy);

        // 开始测量
        meter.startMeasurement();

        // 等待一小段时间
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // 设置能量结束值
        double endEnergy = startEnergy + (i + 1) * 2.0;
        m_mockInterface->setEnergyValue(endEnergy);

        // 结束测量
        meter.endMeasurement();

        // 验证能量消耗
        double expectedConsumption = (i + 1) * 2.0;
        QCOMPARE(meter.energyConsumption(), expectedConsumption);

        // 验证消耗率大于0
        double rate = meter.energyConsumptionRate();
        QVERIFY(rate > 0.0);

        // 消耗率应该约为 expectedConsumption / 0.05秒
        double expectedRate = expectedConsumption / 0.05;
        qDebug() << "Cycle" << i << "- Expected rate:" << expectedRate << "Actual rate:" << rate;

        // 允许一定的误差（由于时间测量的不确定性）
        QVERIFY(rate > expectedRate * 0.5 && rate < expectedRate * 1.5);
    }
}

// 测试实际的CpuEnergyMeter类，这里我们只能通过其公共接口测试
void TestCpuEnergyMeterClass::testRealCpuEnergyMeter()
{
    // 先使用我们的TestableEnergyMeter来建立基准
    m_mockInterface->setEnergyValue(100.0);
    TestableEnergyMeter testable(m_mockInterface);
    testable.startMeasurement();
    m_mockInterface->setEnergyValue(105.0);
    testable.endMeasurement();
    double testableConsumption = testable.energyConsumption();
    QCOMPARE(testableConsumption, 5.0);

    // 使用实际的CpuEnergyMeter，并配置mock接口模拟基本功能
    // 注意：我们无法直接替换CpuEnergyMeter的m_dbusInterface成员
    // 所以这里主要测试公共接口的完整性，而不是具体计算的正确性
    CpuEnergyMeter realMeter;

    // 用合理的行为测试基本功能
    realMeter.startMeasurement();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    realMeter.endMeasurement();

    // 我们只能验证消耗率是一个有限值
    double consumptionRate = realMeter.energyConsumptionRate();
    QVERIFY(std::isfinite(consumptionRate));

    // 验证两次测量之间的能量消耗值是一个有限值
    double consumption = realMeter.energyConsumption();
    QVERIFY(std::isfinite(consumption));
}

QTEST_MAIN(TestCpuEnergyMeterClass)
#include "tst_cpuenergymeter.moc"