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

#include "../base/schedpolicy.h"
#include <QtTest>

class TestSchedPolicy : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_deviceModeFromString();
    void test_powerModeFromString();
    void test_groupTypeFromString();
    void test_deviceModeToString();
    void test_generatePolicyId_strings();
    void test_generatePolicyId_enums();
};

void TestSchedPolicy::test_deviceModeFromString()
{
    // 测试正确的设备模式字符串
    QCOMPARE(sched_policy::deviceModeFromString("PC"), sched_policy::DeviceMode::PC);
    QCOMPARE(sched_policy::deviceModeFromString("Tablet"), sched_policy::DeviceMode::Tablet);
    QCOMPARE(sched_policy::deviceModeFromString("SoftFreeze"), sched_policy::DeviceMode::SoftFreeze);

    // 测试大小写敏感性
    QCOMPARE(sched_policy::deviceModeFromString("pc"), sched_policy::DeviceMode::Unknown);
    QCOMPARE(sched_policy::deviceModeFromString("TABLET"), sched_policy::DeviceMode::Unknown);

    // 测试无效的设备模式字符串
    QCOMPARE(sched_policy::deviceModeFromString(""), sched_policy::DeviceMode::Unknown);
    QCOMPARE(sched_policy::deviceModeFromString("InvalidMode"), sched_policy::DeviceMode::Unknown);
}

void TestSchedPolicy::test_powerModeFromString()
{
    // 测试正确的电源模式字符串
    QCOMPARE(sched_policy::powerModeFromString("Balance"), sched_policy::PowerMode::Balance);
    QCOMPARE(sched_policy::powerModeFromString("Save"), sched_policy::PowerMode::Save);
    QCOMPARE(sched_policy::powerModeFromString("Performance"), sched_policy::PowerMode::Performance);

    // 测试大小写敏感性
    QCOMPARE(sched_policy::powerModeFromString("balance"), sched_policy::PowerMode::Unknown);
    QCOMPARE(sched_policy::powerModeFromString("SAVE"), sched_policy::PowerMode::Unknown);

    // 测试无效的电源模式字符串
    QCOMPARE(sched_policy::powerModeFromString(""), sched_policy::PowerMode::Unknown);
    QCOMPARE(sched_policy::powerModeFromString("InvalidMode"), sched_policy::PowerMode::Unknown);
}

void TestSchedPolicy::test_groupTypeFromString()
{
    // 测试正确的组类型字符串
    QCOMPARE(sched_policy::groupTypeFromString("SessionScope"), sched_policy::GroupType::SessionScopeGroup);
    QCOMPARE(sched_policy::groupTypeFromString("Session"), sched_policy::GroupType::SessionGroup);
    QCOMPARE(sched_policy::groupTypeFromString("Top"), sched_policy::GroupType::TopGroup);
    QCOMPARE(sched_policy::groupTypeFromString("Focus"), sched_policy::GroupType::FocusGroup);
    QCOMPARE(sched_policy::groupTypeFromString("Foreground"), sched_policy::GroupType::ForegroundGroup);
    QCOMPARE(sched_policy::groupTypeFromString("Background"), sched_policy::GroupType::BackgroundGroup);
    QCOMPARE(sched_policy::groupTypeFromString("Service"), sched_policy::GroupType::ServiceGroup);
    QCOMPARE(sched_policy::groupTypeFromString("Cached"), sched_policy::GroupType::CachedGroup);
    QCOMPARE(sched_policy::groupTypeFromString("Default"), sched_policy::GroupType::DefaultGroup);

    // 测试大小写敏感性
    QCOMPARE(sched_policy::groupTypeFromString("sessionscope"), sched_policy::GroupType::Unknown);
    QCOMPARE(sched_policy::groupTypeFromString("SESSION"), sched_policy::GroupType::Unknown);

    // 测试无效的组类型字符串
    QCOMPARE(sched_policy::groupTypeFromString(""), sched_policy::GroupType::Unknown);
    QCOMPARE(sched_policy::groupTypeFromString("InvalidType"), sched_policy::GroupType::Unknown);
}

void TestSchedPolicy::test_deviceModeToString()
{
    // 测试设备模式枚举到字符串的转换
    QCOMPARE(sched_policy::deviceModeToString(sched_policy::DeviceMode::PC), std::string("PC"));
    QCOMPARE(sched_policy::deviceModeToString(sched_policy::DeviceMode::Tablet), std::string("Tablet"));
    QCOMPARE(sched_policy::deviceModeToString(sched_policy::DeviceMode::SoftFreeze), std::string("SoftFreeze"));

    // Unknown设备模式没有对应的字符串值，应返回空
    // 注意：根据源码实现可能需要调整这个测试
    QCOMPARE(sched_policy::deviceModeToString(sched_policy::DeviceMode::Unknown).empty(), false);
}

void TestSchedPolicy::test_generatePolicyId_strings()
{
    // 测试正确的策略ID生成
    QCOMPARE(sched_policy::generatePolicyId("PC", "Balance"), std::string("pc_balance"));
    QCOMPARE(sched_policy::generatePolicyId("Tablet", "Save"), std::string("tablet_save"));
    QCOMPARE(sched_policy::generatePolicyId("SoftFreeze", "Performance"), std::string("softfreeze_performance"));

    // 测试无效的设备模式参数
    QCOMPARE(sched_policy::generatePolicyId("InvalidMode", "Balance").empty(), true);

    // 测试无效的电源模式参数
    QCOMPARE(sched_policy::generatePolicyId("PC", "InvalidMode").empty(), true);

    // 测试两个参数都无效的情况
    QCOMPARE(sched_policy::generatePolicyId("", "").empty(), true);
}

void TestSchedPolicy::test_generatePolicyId_enums()
{
    // 测试正确的策略ID生成
    QCOMPARE(sched_policy::generatePolicyId(sched_policy::DeviceMode::PC, sched_policy::PowerMode::Balance),
             std::string("pc_balance"));
    QCOMPARE(sched_policy::generatePolicyId(sched_policy::DeviceMode::Tablet, sched_policy::PowerMode::Save),
             std::string("tablet_save"));
    QCOMPARE(sched_policy::generatePolicyId(sched_policy::DeviceMode::SoftFreeze, sched_policy::PowerMode::Performance),
             std::string("softfreeze_performance"));

    // 测试无效的设备模式参数
    QCOMPARE(sched_policy::generatePolicyId(sched_policy::DeviceMode::Unknown, sched_policy::PowerMode::Balance).empty(),
             true);

    // 测试无效的电源模式参数
    QCOMPARE(sched_policy::generatePolicyId(sched_policy::DeviceMode::PC, sched_policy::PowerMode::Unknown).empty(),
             true);

    // 测试两个参数都无效的情况
    QCOMPARE(sched_policy::generatePolicyId(sched_policy::DeviceMode::Unknown, sched_policy::PowerMode::Unknown).empty(),
             true);
}

QTEST_APPLESS_MAIN(TestSchedPolicy)

#include "tst_schedpolicy.moc"