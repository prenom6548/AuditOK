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

#include "appinfo.h"
#include "configmanager.h"
#include "exceptiongroupprocesswatcher.h"
#include "schedpolicymanager.h"
#include <QtTest>

class TestSchedPolicyManager : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testHandleAppStarted();
    void testHandleAppStateChanged();
    void testSetPowerMode();

private:
    std::shared_ptr<ConfigManager> m_configManager;
    std::shared_ptr<ExceptionGroupProcessWatcher> m_exceptionGroupWatcher;
    SchedPolicyManager *m_schedPolicyManager;
};

void TestSchedPolicyManager::initTestCase()
{
    m_configManager = std::make_shared<ConfigManager>();
    m_exceptionGroupWatcher = std::make_shared<ExceptionGroupProcessWatcher>();
    m_schedPolicyManager = new SchedPolicyManager(m_configManager, m_exceptionGroupWatcher,
                                                  sched_policy::PowerMode::Performance, sched_policy::DeviceMode::PC);

    // 设置一个简单的回调函数
    m_schedPolicyManager->setGetAllAppInfoCallback([]() -> std::vector<AppInfo> {
        return {};
    });
}

void TestSchedPolicyManager::cleanupTestCase()
{
    delete m_schedPolicyManager;
}

void TestSchedPolicyManager::testHandleAppStarted()
{
    AppInfo appInfo;
    appInfo.setAppId("testApp");

    m_schedPolicyManager->handleAppStarted(appInfo);

    // 验证处理逻辑是否正确
    QVERIFY(true); // 根据实际逻辑补充验证
}

void TestSchedPolicyManager::testHandleAppStateChanged()
{
    AppInfo appInfo;
    appInfo.setAppId("testApp");

    appInfo.setAppState(sched_policy::AppState::Focus);

    m_schedPolicyManager->handleAppStateChanged(appInfo);

    // 验证处理逻辑是否正确
    QVERIFY(true); // 根据实际逻辑补充验证
}

void TestSchedPolicyManager::testSetPowerMode()
{
    m_schedPolicyManager->setPowerMode(sched_policy::PowerMode::Save);
}

QTEST_MAIN(TestSchedPolicyManager)
#include "tst_schedpolicymanager.moc"
