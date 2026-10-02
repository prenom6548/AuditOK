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

#include "base/schedpolicy.h"
#include "core/configmanager.h"
#include <QtTest/QtTest>

class TestConfigManagerClass : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testResourceLimitEnabled();
    void testSoftFreezeModeEnabled();
    void testAppWhitelist();
    void testBackground2CachedStateInterval();
    void testDefaultAppsInTopGroup();
    void testPolicyConfigInfo();
    void testResourceLimitEnabledCallback();
    void testSoftFreezeModeEnabledCallback();
    void testSetIncreasedBatteryRemainingTime();

private:
    ConfigManager *m_configManager;
};

void TestConfigManagerClass::initTestCase()
{
    m_configManager = new ConfigManager();
}

void TestConfigManagerClass::cleanupTestCase()
{
    delete m_configManager;
}

void TestConfigManagerClass::testResourceLimitEnabled()
{
    m_configManager->setResourceLimitEnabled(true);
    QVERIFY(m_configManager->reousrceLimitEnabled() == true);

    m_configManager->setResourceLimitEnabled(false);
    QVERIFY(m_configManager->reousrceLimitEnabled() == false);
}

void TestConfigManagerClass::testSoftFreezeModeEnabled()
{
    // m_configManager->setSoftFreezeModeEnabled(true);
    QVERIFY(m_configManager->softFreezeModeEnabled() == true);

    // m_configManager->setSoftFreezeModeEnabled(false);
    // QVERIFY(m_configManager->softFreezeModeEnabled() == false);
}

void TestConfigManagerClass::testAppWhitelist()
{
    // 测试白名单添加
    QStringList initialList;

    QString testApp1 = "org.test.app1.desktop";
    QString testApp2 = "org.test.app2.desktop";

    // 添加应用到白名单
    bool addResult1 = m_configManager->addAppToWhitelist(testApp1);
    QVERIFY(addResult1);
    QVERIFY(m_configManager->appWhitelist().contains(testApp1));

    // 再次添加相同应用，应返回成功但不重复添加
    bool addResult2 = m_configManager->addAppToWhitelist(testApp1);
    QVERIFY(addResult2);
    QCOMPARE(m_configManager->appWhitelist().count(), 1);

    // 添加另一个应用
    bool addResult3 = m_configManager->addAppToWhitelist(testApp2);
    QVERIFY(addResult3);
    QCOMPARE(m_configManager->appWhitelist().count(), 2);

    // 测试移除应用
    bool removeResult1 = m_configManager->removeAppFromWhitelist(testApp1);
    QVERIFY(removeResult1);
    QVERIFY(!m_configManager->appWhitelist().contains(testApp1));
    QCOMPARE(m_configManager->appWhitelist().count(), 1);

    // 移除不存在的应用，应返回成功
    bool removeResult2 = m_configManager->removeAppFromWhitelist("nonexistent.desktop");
    QVERIFY(removeResult2);
    QCOMPARE(m_configManager->appWhitelist().count(), 1);
}

void TestConfigManagerClass::testBackground2CachedStateInterval()
{
    // 创建测试数据
    Json::Value testIntervals;
    testIntervals["normal"] = 60;
    testIntervals["battery"] = 30;
    testIntervals["performance"] = 120;

    // m_configManager->setBackground2CachedStateInterval(testIntervals);

    // 验证不同设备模式下的间隔设置
    QCOMPARE(m_configManager->background2CachedStateInterval(sched_policy::DeviceMode::Tablet), 60);
    QCOMPARE(m_configManager->background2CachedStateInterval(sched_policy::DeviceMode::PC), 30);
    QCOMPARE(m_configManager->background2CachedStateInterval(sched_policy::DeviceMode::SoftFreeze), 120);
}

void TestConfigManagerClass::testDefaultAppsInTopGroup()
{
    QStringList testApps = {"app1.desktop", "app2.desktop", "app3.desktop"};
    // m_configManager->setDefaultAppsInTopGroup(testApps);

    QStringList result = m_configManager->defaultAppsInTopGroup();
    QCOMPARE(result, testApps);
    QCOMPARE(result.size(), 3);
}

void TestConfigManagerClass::testPolicyConfigInfo()
{
    Json::Value testPolicy;
    testPolicy["testKey"] = "testValue";
    testPolicy["group"]["subKey"] = 123;

    // m_configManager->setPolicyConfigInfo(testPolicy);

    Json::Value result = m_configManager->policyConfigInfo();
    QCOMPARE(QString::fromStdString(result["testKey"].asString()), QString("testValue"));
    QCOMPARE(result["group"]["subKey"].asInt(), 123);
}

void TestConfigManagerClass::testResourceLimitEnabledCallback()
{
    bool callbackTriggered = false;
    bool callbackValue = false;

    // 设置回调函数
    m_configManager->setResourceLimitEnabledChangedCallback([&](bool enabled) {
        callbackTriggered = true;
        callbackValue = enabled;
    });

    // 触发回调
    m_configManager->setResourceLimitEnabled(true);
    // m_configManager->triggerResourceLimitEnabledCallbacks();

    // 验证回调被正确触发
    QVERIFY(callbackTriggered);
    QVERIFY(callbackValue);

    // 重置测试状态
    callbackTriggered = false;

    // 测试禁用状态
    m_configManager->setResourceLimitEnabled(false);
    // m_configManager->triggerResourceLimitEnabledCallbacks();

    QVERIFY(callbackTriggered);
    QVERIFY(!callbackValue);
}

void TestConfigManagerClass::testSoftFreezeModeEnabledCallback()
{
    bool callbackTriggered = false;
    bool callbackValue = false;

    // 设置回调函数
    m_configManager->setSoftFreezeModeEnabledChangedCallback([&](bool enabled) {
        callbackTriggered = true;
        callbackValue = enabled;
    });

    // 触发回调
    // m_configManager->setSoftFreezeModeEnabledForTest(true);
    // m_configManager->triggerSoftFreezeModeCallback();

    // 验证回调被正确触发
    QVERIFY(callbackTriggered);
    QVERIFY(callbackValue);

    // 重置测试状态
    callbackTriggered = false;

    // 测试禁用状态
    // m_configManager->setSoftFreezeModeEnabled(false);
    // m_configManager->triggerSoftFreezeModeCallback();

    QVERIFY(callbackTriggered);
    QVERIFY(!callbackValue);
}

void TestConfigManagerClass::testSetIncreasedBatteryRemainingTime()
{
    // 这个测试只能验证函数调用不会崩溃，无法验证GSetting实际值
    // 在实际测试中需要使用QSignalSpy或其他机制验证
    m_configManager->setIncreasedBatteryRemainingTime(-1);
    m_configManager->setIncreasedBatteryRemainingTime(0);
    m_configManager->setIncreasedBatteryRemainingTime(100);
}

QTEST_MAIN(TestConfigManagerClass)
#include "tst_configmanager.moc"