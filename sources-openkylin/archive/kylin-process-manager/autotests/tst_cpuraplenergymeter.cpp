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
#include <QtTest>
#include <fcntl.h>

// 为了能够访问私有方法
#define private public
#define protected public
#include "../daemon/src/cpuraplenergymeter.h"
#undef private
#undef protected

class TestCpuRaplEnergyMeter : public QObject
{
    Q_OBJECT

public:
    TestCpuRaplEnergyMeter();
    ~TestCpuRaplEnergyMeter();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试构造函数和初始化
    void testConstructor();

    // 测试可用性检查方法
    void testIsAvailable();

    // 测试能源消耗读取方法
    void testEnergyConsumption();

    // 测试底层支持检查方法
    void testPowercapSupported();
    void testMsrSupported();

    // 测试接口获取方法
    void testGetMsrInterface();

    // 测试能源读取方法
    void testReadEnergyFromPowercap();
    void testReadEnergyFromMsr();

    // 测试RAPL单位读取
    void testReadRaplPowerUnit();

    // 测试底层读取方法
    void testReadPowercap();
    void testReadMsr();

private:
    CpuRaplEnergyMeter *m_meter;
    bool m_hasRaplSupport;
};

TestCpuRaplEnergyMeter::TestCpuRaplEnergyMeter()
{
}

TestCpuRaplEnergyMeter::~TestCpuRaplEnergyMeter()
{
}

void TestCpuRaplEnergyMeter::initTestCase()
{
    // 检查系统是否支持RAPL接口
    bool powercapExists = QFile::exists("/sys/devices/virtual/powercap/intel-rapl/intel-rapl:0/energy_uj");
    bool msrExists = QFile::exists("/dev/cpu/0/msr") || QFile::exists("/dev/cpu/msr0");

    m_hasRaplSupport = powercapExists || msrExists;
    if (!m_hasRaplSupport) {
        QWARN("系统不支持RAPL接口，某些测试可能被跳过");
    }
}

void TestCpuRaplEnergyMeter::cleanupTestCase()
{
}

void TestCpuRaplEnergyMeter::init()
{
    m_meter = new CpuRaplEnergyMeter();
}

void TestCpuRaplEnergyMeter::cleanup()
{
    delete m_meter;
    m_meter = nullptr;
}

void TestCpuRaplEnergyMeter::testConstructor()
{
    // 验证构造函数设置了正确的初始状态
    QVERIFY(m_meter != nullptr);

    // 验证支持状态检查是否被正确初始化
    bool powercapSupported = m_meter->m_isPowercapSupported;
    bool msrSupported = m_meter->m_isMsrSupported;

    // 这些值应该反映系统的实际可用性
    if (m_hasRaplSupport) {
        QVERIFY(powercapSupported || msrSupported);
    }
}

void TestCpuRaplEnergyMeter::testIsAvailable()
{
    bool available = m_meter->isAvailable();

    // isAvailable 应该与我们在initTestCase中检测到的一致
    QCOMPARE(available, m_hasRaplSupport);

    // 应该与内部状态一致
    QCOMPARE(available, (m_meter->m_isPowercapSupported || m_meter->m_isMsrSupported));
}

void TestCpuRaplEnergyMeter::testEnergyConsumption()
{
    // 只有在系统支持RAPL时才能进行实际测试
    if (!m_hasRaplSupport) {
        QSKIP("系统不支持RAPL，跳过能源消耗测试");
    }

    // 获取能源消耗
    double energy = m_meter->energyConsumption();

    // 能源消耗不应该是负值
    QVERIFY(energy >= 0.0);

    // 如果系统支持powercap，应该使用powercap方法
    if (m_meter->m_isPowercapSupported) {
        double powercapEnergy = m_meter->readEnergyFromPowercap();
        QCOMPARE(energy, powercapEnergy);
    }
    // 如果系统只支持msr，应该使用msr方法
    else if (m_meter->m_isMsrSupported && !m_meter->m_isPowercapSupported) {
        double msrEnergy = m_meter->readEnergyFromMsr();
        QCOMPARE(energy, msrEnergy);
    }
}

void TestCpuRaplEnergyMeter::testPowercapSupported()
{
    // 直接调用底层检查方法
    bool supported = m_meter->checkIfPowercapSupported();

    // 应该与构造函数设置的状态一致
    QCOMPARE(supported, m_meter->m_isPowercapSupported);

    // 查看接口文件是否存在
    bool fileExists = QFile::exists("/sys/devices/virtual/powercap/intel-rapl/intel-rapl:0/energy_uj");
    QCOMPARE(supported, fileExists);
}

void TestCpuRaplEnergyMeter::testMsrSupported()
{
    // 直接调用底层检查方法
    bool supported = m_meter->checkIfMsrSupported();

    // 应该与构造函数设置的状态一致
    QCOMPARE(supported, m_meter->m_isMsrSupported);

    // 查看MSR接口是否有效
    std::string msrInterface = m_meter->getMsrInterface();
    QCOMPARE(supported, !msrInterface.empty());
}

void TestCpuRaplEnergyMeter::testGetMsrInterface()
{
    std::string msrInterface = m_meter->getMsrInterface();

    // 如果系统支持MSR，接口不应该为空
    if (m_meter->m_isMsrSupported) {
        QVERIFY(!msrInterface.empty());

        // 检查返回的接口是否是有效的文件
        QVERIFY(msrInterface == "/dev/cpu/0/msr" || msrInterface == "/dev/cpu/msr0");
        QVERIFY(QFile::exists(QString::fromStdString(msrInterface)));
    } else {
        QVERIFY(msrInterface.empty());
    }
}

void TestCpuRaplEnergyMeter::testReadEnergyFromPowercap()
{
    // 只有在系统支持powercap时才测试
    if (!m_meter->m_isPowercapSupported) {
        QSKIP("系统不支持powercap，跳过相关测试");
    }

    double energy = m_meter->readEnergyFromPowercap();

    // 能源值应该是正数
    QVERIFY(energy >= 0.0);
}

void TestCpuRaplEnergyMeter::testReadEnergyFromMsr()
{
    // 只有在系统支持MSR时才测试
    if (!m_meter->m_isMsrSupported) {
        QSKIP("系统不支持MSR，跳过相关测试");
    }

    double energy = m_meter->readEnergyFromMsr();

    // 能源值应该是正数
    QVERIFY(energy >= 0.0);
}

void TestCpuRaplEnergyMeter::testReadRaplPowerUnit()
{
    // 只有在系统支持MSR时才测试
    if (!m_meter->m_isMsrSupported) {
        QSKIP("系统不支持MSR，跳过相关测试");
    }

    double powerUnit = m_meter->readRaplPowerUnit();

    // 能源单位应该是正数且小于1
    QVERIFY(powerUnit > 0.0);
    QVERIFY(powerUnit < 1.0);
}

void TestCpuRaplEnergyMeter::testReadPowercap()
{
    // 只有在系统支持powercap时才测试
    if (!m_meter->m_isPowercapSupported) {
        QSKIP("系统不支持powercap，跳过相关测试");
    }

    double value = 0.0;
    int result = m_meter->readPowercap("/sys/devices/virtual/powercap/intel-rapl/intel-rapl:0/energy_uj", &value);

    // 读取应该成功
    QCOMPARE(result, 0);

    // 值应该是正数
    QVERIFY(value > 0.0);
}

void TestCpuRaplEnergyMeter::testReadMsr()
{
    // 只有在系统支持MSR时才测试
    if (!m_meter->m_isMsrSupported) {
        QSKIP("系统不支持MSR，跳过相关测试");
    }

    uint64_t value = 0;
    std::string msrInterface = m_meter->getMsrInterface();

    // 读取电源单位寄存器
    int result = m_meter->readMsr(msrInterface, 0x606, &value);

    // 读取应该成功
    QCOMPARE(result, 0);

    // 值应该不为0
    QVERIFY(value != 0);
}

QTEST_MAIN(TestCpuRaplEnergyMeter)
#include "tst_cpuraplenergymeter.moc"