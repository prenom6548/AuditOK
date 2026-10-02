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

// 定义private为public以便访问私有成员
#define private public
#define protected public
#include "../core/devicestate.h"
#undef private
#undef protected

// 创建QGSettings模拟类
class MockQGSettings : public QGSettings
{
public:
    MockQGSettings(const QByteArray &schemaId, QObject *parent = nullptr)
        : QGSettings(schemaId)
    {
    }

    // 重写get方法以便测试
    QVariant get(const QString &key) const
    {
        if (m_mockValues.contains(key)) {
            return m_mockValues[key];
        }
        return QVariant();
    }

    // 设置模拟值
    void setMockValue(const QString &key, const QVariant &value)
    {
        m_mockValues[key] = value;
        Q_EMIT changed(key);
    }

private:
    QMap<QString, QVariant> m_mockValues;
};

// 测试PowerManager类
class TestPowerManager : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试用例
    void testCurrentPowerType();
    void testCurrentPowerMode();
    void testPowerTypeChange();
    void testPowerModeChange();
    void testHandlePowerModeChanged();

private:
    PowerManager *m_powerManager;
    MockQGSettings *m_mockGSettings;
};

void TestPowerManager::initTestCase()
{
    // 什么都不做，每个测试前会创建新实例
}

void TestPowerManager::cleanupTestCase()
{
    // 什么都不做，每个测试后会清理实例
}

void TestPowerManager::init()
{
    // 创建PowerManager实例
    m_powerManager = new PowerManager();

    // 替换内部的QGSettings为MockQGSettings
    m_mockGSettings = new MockQGSettings("org.ukui.power-manager");
    m_powerManager->m_gsettings.reset(m_mockGSettings);

    // 设置初始状态
    m_mockGSettings->setMockValue("powerPolicyAc", 1); // Balance
    m_mockGSettings->setMockValue("powerPolicyBattery", 2); // Save
}

void TestPowerManager::cleanup()
{
    delete m_powerManager;
}

void TestPowerManager::testCurrentPowerType()
{
    // 设置为电池供电
    m_powerManager->m_currentPowerType = sched_policy::PowerType::Battery;
    QCOMPARE(m_powerManager->currentPowerType(), sched_policy::PowerType::Battery);

    // 设置为交流电源
    m_powerManager->m_currentPowerType = sched_policy::PowerType::Ac;
    QCOMPARE(m_powerManager->currentPowerType(), sched_policy::PowerType::Ac);
}

void TestPowerManager::testCurrentPowerMode()
{
    // 设置为性能模式
    m_powerManager->m_currentPowerMode = sched_policy::PowerMode::Performance;
    QCOMPARE(m_powerManager->currentPowerMode(), sched_policy::PowerMode::Performance);

    // 设置为平衡模式
    m_powerManager->m_currentPowerMode = sched_policy::PowerMode::Balance;
    QCOMPARE(m_powerManager->currentPowerMode(), sched_policy::PowerMode::Balance);

    // 设置为省电模式
    m_powerManager->m_currentPowerMode = sched_policy::PowerMode::Save;
    QCOMPARE(m_powerManager->currentPowerMode(), sched_policy::PowerMode::Save);
}

void TestPowerManager::testPowerTypeChange()
{
    // 为powerTypeChanged信号创建监听
    QSignalSpy spy(m_powerManager, &PowerManager::powerTypeChanged);

    // 模拟DBus属性变更 - 切换到电池供电
    QMap<QString, QVariant> properties;
    properties["OnBattery"] = true;
    m_powerManager->onPropertiesChanged("org.freedesktop.UPower", properties, QStringList());

    // 验证信号是否发出以及电源类型是否正确更新
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_powerManager->currentPowerType(), sched_policy::PowerType::Battery);

    // 验证电源模式是否根据电池设置更新
    QCOMPARE(m_powerManager->currentPowerMode(), sched_policy::PowerMode::Save);

    // 清空信号记录
    spy.clear();

    // 模拟DBus属性变更 - 切换到交流电源
    properties["OnBattery"] = false;
    m_powerManager->onPropertiesChanged("org.freedesktop.UPower", properties, QStringList());

    // 验证信号是否发出以及电源类型是否正确更新
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_powerManager->currentPowerType(), sched_policy::PowerType::Ac);

    // 验证电源模式是否根据交流电源设置更新
    QCOMPARE(m_powerManager->currentPowerMode(), sched_policy::PowerMode::Balance);
}

void TestPowerManager::testPowerModeChange()
{
    // 为powerModeChanged信号创建监听
    QSignalSpy spy(m_powerManager, &PowerManager::powerModeChanged);

    // 设置当前为交流电源
    m_powerManager->m_currentPowerType = sched_policy::PowerType::Ac;

    // 修改交流电源设置为性能模式
    m_mockGSettings->setMockValue("powerPolicyAc", 0); // Performance

    // 验证信号是否发出以及电源模式是否正确更新
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_powerManager->currentPowerMode(), sched_policy::PowerMode::Performance);

    // 清空信号记录
    spy.clear();

    // 设置当前为电池供电
    m_powerManager->m_currentPowerType = sched_policy::PowerType::Battery;

    // 修改电池设置为平衡模式
    m_mockGSettings->setMockValue("powerPolicyBattery", 1); // Balance

    // 验证信号是否发出以及电源模式是否正确更新
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_powerManager->currentPowerMode(), sched_policy::PowerMode::Balance);
}

void TestPowerManager::testHandlePowerModeChanged()
{
    // 为powerModeChanged信号创建监听
    QSignalSpy spy(m_powerManager, &PowerManager::powerModeChanged);

    // 测试性能模式
    m_powerManager->handlePowerModeChanged(0);
    QCOMPARE(m_powerManager->m_currentPowerMode, sched_policy::PowerMode::Performance);
    QCOMPARE(spy.count(), 1);
    spy.clear();

    // 测试平衡模式
    m_powerManager->handlePowerModeChanged(1);
    QCOMPARE(m_powerManager->m_currentPowerMode, sched_policy::PowerMode::Balance);
    QCOMPARE(spy.count(), 1);
    spy.clear();

    // 测试省电模式
    m_powerManager->handlePowerModeChanged(2);
    QCOMPARE(m_powerManager->m_currentPowerMode, sched_policy::PowerMode::Save);
    QCOMPARE(spy.count(), 1);
    spy.clear();

    // 测试未知模式
    m_powerManager->handlePowerModeChanged(99);
    QCOMPARE(m_powerManager->m_currentPowerMode, sched_policy::PowerMode::Unknown);
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestPowerManager)
#include "tst_powermanager.moc"