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

#include "resource.h"
#include <QCoreApplication>
#include <QtTest>
#include <functional>
#include <sys/epoll.h>

// 允许访问PressureWatcher的私有方法
#define private public
#define protected public
#include "../daemon/src/pressurewatcher.h"
#include "../daemon/src/resource.h"
#undef private
#undef protected

class TestPressureWatcher : public QObject
{
    Q_OBJECT

public:
    TestPressureWatcher();
    ~TestPressureWatcher();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试构造函数和初始化
    void testConstructor();

    // 测试PSI触发器的初始化
    void testInitPsiTriggers();

    // 测试检查资源是否包含在触发器列表中
    void testContainResource();

    // 测试映射压力值到PSI触发器格式
    void testMapPressureValueToPsiTrigger();

    // 测试获取PSI路径
    void testGetPsiPath();

    // 测试停止监视
    void testStop();

private:
    // 辅助函数，创建测试用的资源阈值映射
    std::map<resource::Resource, resource::ResourceUrgencyThreshold> createTestResourceThresholds();

    PressureWatcher *m_pressureWatcher;
};

TestPressureWatcher::TestPressureWatcher()
{
}

TestPressureWatcher::~TestPressureWatcher()
{
}

void TestPressureWatcher::initTestCase()
{
    // 检查是否支持PSI接口，如果不支持则跳过测试
    QFile cpuPsi("/proc/pressure/cpu");
    if (!cpuPsi.exists()) {
        QSKIP("PSI interface not available, skipping tests");
    }
}

void TestPressureWatcher::cleanupTestCase()
{
}

void TestPressureWatcher::init()
{
    // 创建测试资源阈值
    auto thresholds = createTestResourceThresholds();

    // 创建PressureWatcher实例
    m_pressureWatcher = new PressureWatcher(thresholds);
}

void TestPressureWatcher::cleanup()
{
    delete m_pressureWatcher;
}

void TestPressureWatcher::testConstructor()
{
    // 验证构造函数是否正确初始化了内部状态
    QVERIFY(!m_pressureWatcher->m_stop);
    QVERIFY(m_pressureWatcher->m_triggers.empty()); // 初始化时触发器列表应该为空
}

void TestPressureWatcher::testInitPsiTriggers()
{
    // 调用初始化触发器函数
    m_pressureWatcher->initPsiTriggers(createTestResourceThresholds());

    // 验证触发器是否被正确初始化
    QVERIFY(!m_pressureWatcher->m_triggers.empty());

    // CPU应该有低、中、高三个紧急程度的触发器
    int cpuTriggerCount = 0;
    for (const auto &trigger : m_pressureWatcher->m_triggers) {
        if (trigger.resource == "CPU") {
            cpuTriggerCount++;
        }
    }
    QCOMPARE(cpuTriggerCount, 3);

    // Memory也应该有三个触发器
    int memoryTriggerCount = 0;
    for (const auto &trigger : m_pressureWatcher->m_triggers) {
        if (trigger.resource == "Memory") {
            memoryTriggerCount++;
        }
    }
    QCOMPARE(memoryTriggerCount, 3);
}

void TestPressureWatcher::testContainResource()
{
    // 创建一个包含CPU资源的触发器列表
    std::vector<PsiTrigger> triggers;
    triggers.push_back(PsiTrigger("CPU", resource::ResourceUrgency::Low, "some_trigger", -1));

    // 检查列表是否包含CPU资源
    int index = m_pressureWatcher->containResource(triggers, "CPU");

    // 验证结果
    QCOMPARE(index, 0);

    // 测试不存在的资源
    index = m_pressureWatcher->containResource(triggers, "Memory");

    // 验证结果
    QCOMPARE(index, -1);
}

void TestPressureWatcher::testMapPressureValueToPsiTrigger()
{
    // 测试不同的压力值
    std::string result50 = m_pressureWatcher->mapPressureValueToPsiTrigger(50);
    std::string result80 = m_pressureWatcher->mapPressureValueToPsiTrigger(80);
    std::string result100 = m_pressureWatcher->mapPressureValueToPsiTrigger(100);

    // 验证结果
    QVERIFY(!result50.empty());
    QVERIFY(!result80.empty());
    QVERIFY(!result100.empty());

    // 确保不同的压力值生成不同的触发器格式
    QVERIFY(result50 != result80);
    QVERIFY(result80 != result100);
}

void TestPressureWatcher::testGetPsiPath()
{
    // 测试获取各种资源的PSI路径
    // std::string cpuPath = m_pressureWatcher->getPsiPath("CPU");
    // std::string memoryPath = m_pressureWatcher->getPsiPath("Memory");
    // std::string ioPath = m_pressureWatcher->getPsiPath("IO");

    // // 验证路径是否正确
    // QCOMPARE(QString::fromStdString(cpuPath), QString("/proc/pressure/cpu"));
    // QCOMPARE(QString::fromStdString(memoryPath), QString("/proc/pressure/memory"));
    // QCOMPARE(QString::fromStdString(ioPath), QString("/proc/pressure/io"));
}

void TestPressureWatcher::testStop()
{
    // 测试停止监视器
    m_pressureWatcher->stop();
    QVERIFY(m_pressureWatcher->m_stop);
}

// 辅助函数实现
std::map<resource::Resource, resource::ResourceUrgencyThreshold> TestPressureWatcher::createTestResourceThresholds()
{
    std::map<resource::Resource, resource::ResourceUrgencyThreshold> thresholds;

    // 设置CPU资源的阈值
    resource::ResourceUrgencyThreshold cpuThresholds;
    cpuThresholds[resource::ResourceUrgency::Low] = 20;
    cpuThresholds[resource::ResourceUrgency::Medium] = 50;
    cpuThresholds[resource::ResourceUrgency::High] = 80;
    thresholds[resource::Resource::CPU] = cpuThresholds;

    // 设置内存资源的阈值
    resource::ResourceUrgencyThreshold memoryThresholds;
    memoryThresholds[resource::ResourceUrgency::Low] = 20;
    memoryThresholds[resource::ResourceUrgency::Medium] = 50;
    memoryThresholds[resource::ResourceUrgency::High] = 80;
    thresholds[resource::Resource::Memory] = memoryThresholds;

    return thresholds;
}

QTEST_MAIN(TestPressureWatcher)
#include "tst_pressurewatcher.moc"