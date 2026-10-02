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

#include <QDBusInterface>
#include <QDBusReply>
#include <QSignalSpy>
#include <QtTest/QtTest>

// 定义private和protected为public以访问私有成员
#define private public
#define protected public
#include "../core/devicestate.h"
#undef private
#undef protected

// 创建测试用DeviceModeManager子类，以便模拟平板模式
class TestableDeviceModeManager : public DeviceModeManager
{
public:
    TestableDeviceModeManager(bool softFreezeEnabled)
        : DeviceModeManager(softFreezeEnabled)
    {
        // 重置为初始状态
        resetDeviceMode();
    }

    // 模拟平板模式检测结果
    bool isTabletMode()
    {
        return m_mockTabletMode;
    }

    // 设置平板模式
    void setMockTabletMode(bool isTablet)
    {
        m_mockTabletMode = isTablet;
    }

private:
    bool m_mockTabletMode = false;
};

// 测试DeviceModeManager类
class TestDeviceModeManager : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试用例
    void testCurrentDeviceMode();
    void testTabletModeDetection();
    void testSoftFreezeModeEnabled();
    void testDeviceModeChange();
    void testUpdateDeviceMode();

private:
    TestableDeviceModeManager *m_deviceModeManager;
};

void TestDeviceModeManager::initTestCase()
{
    // 什么都不做，每个测试前会创建新实例
}

void TestDeviceModeManager::cleanupTestCase()
{
    // 什么都不做，每个测试后会清理实例
}

void TestDeviceModeManager::init()
{
    // 创建DeviceModeManager实例，初始状态禁用软冻结模式
    m_deviceModeManager = new TestableDeviceModeManager(false);
}

void TestDeviceModeManager::cleanup()
{
    delete m_deviceModeManager;
}

void TestDeviceModeManager::testCurrentDeviceMode()
{
    // 默认应该是PC模式（因为非平板且软冻结禁用）
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::PC);

    // 手动设置为平板模式
    m_deviceModeManager->m_currentDeviceMode = sched_policy::DeviceMode::Tablet;
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::Tablet);

    // 手动设置为软冻结模式
    m_deviceModeManager->m_currentDeviceMode = sched_policy::DeviceMode::SoftFreeze;
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::SoftFreeze);
}

void TestDeviceModeManager::testTabletModeDetection()
{
    // 默认应该是PC模式
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::PC);

    // 为deviceModeChanged信号创建监听器
    QSignalSpy spy(m_deviceModeManager, &DeviceModeManager::deviceModeChanged);

    // 模拟切换到平板模式
    m_deviceModeManager->setMockTabletMode(true);
    m_deviceModeManager->updateDeviceMode();

    // 验证信号是否发出以及设备模式是否正确更新
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::Tablet);

    // 清空信号记录
    spy.clear();

    // 模拟切换回非平板模式
    m_deviceModeManager->setMockTabletMode(false);
    m_deviceModeManager->updateDeviceMode();

    // 验证信号是否发出以及设备模式是否正确更新
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::PC);
}

void TestDeviceModeManager::testSoftFreezeModeEnabled()
{
    // 默认软冻结模式禁用，应该是PC模式
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::PC);

    // 为deviceModeChanged信号创建监听器
    QSignalSpy spy(m_deviceModeManager, &DeviceModeManager::deviceModeChanged);

    // 模拟启用软冻结模式
    m_deviceModeManager->handleSoftFreezeModeEnabledChanged(true);

    // 验证信号是否发出以及设备模式是否正确更新
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::SoftFreeze);
    QCOMPARE(m_deviceModeManager->m_softFreezeModeEnabled, true);

    // 清空信号记录
    spy.clear();

    // 模拟禁用软冻结模式
    m_deviceModeManager->handleSoftFreezeModeEnabledChanged(false);

    // 验证信号是否发出以及设备模式是否正确更新
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::PC);
    QCOMPARE(m_deviceModeManager->m_softFreezeModeEnabled, false);
}

void TestDeviceModeManager::testDeviceModeChange()
{
    // 为deviceModeChanged信号创建监听器
    QSignalSpy spy(m_deviceModeManager, &DeviceModeManager::deviceModeChanged);

    // 测试模式优先级：平板模式 > 软冻结模式 > PC模式

    // 1. 先设置为软冻结模式
    m_deviceModeManager->m_softFreezeModeEnabled = true;
    m_deviceModeManager->updateDeviceMode();
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::SoftFreeze);
    QCOMPARE(spy.count(), 1);
    spy.clear();

    // 2. 设置为平板模式（优先级更高）
    m_deviceModeManager->setMockTabletMode(true);
    m_deviceModeManager->updateDeviceMode();
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::Tablet);
    QCOMPARE(spy.count(), 1);
    spy.clear();

    // 3. 禁用软冻结模式（不应该改变当前平板模式）
    m_deviceModeManager->m_softFreezeModeEnabled = false;
    m_deviceModeManager->updateDeviceMode();
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::Tablet);
    QCOMPARE(spy.count(), 0); // 没有变化，不会发信号

    // 4. 切换回非平板模式（应该变为PC模式）
    m_deviceModeManager->setMockTabletMode(false);
    m_deviceModeManager->updateDeviceMode();
    QCOMPARE(m_deviceModeManager->currentDeviceMode(), sched_policy::DeviceMode::PC);
    QCOMPARE(spy.count(), 1);
}

void TestDeviceModeManager::testUpdateDeviceMode()
{
    // 为deviceModeChanged信号创建监听器
    QSignalSpy spy(m_deviceModeManager, &DeviceModeManager::deviceModeChanged);

    // 测试没有变化的情况
    m_deviceModeManager->updateDeviceMode();
    QCOMPARE(spy.count(), 0); // 没有变化，不会发信号

    // 测试有变化的情况
    m_deviceModeManager->setMockTabletMode(true);
    m_deviceModeManager->updateDeviceMode();
    QCOMPARE(spy.count(), 1); // 有变化，会发信号

    // 再次调用，没有变化
    m_deviceModeManager->updateDeviceMode();
    QCOMPARE(spy.count(), 1); // 仍然是1，没有新信号
}

QTEST_MAIN(TestDeviceModeManager)
#include "tst_devicemodemanager.moc"