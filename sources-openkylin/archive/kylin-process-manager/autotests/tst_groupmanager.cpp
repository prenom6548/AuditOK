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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "base/groupmanagementunit.h"
#include <QSignalSpy>
#include <QtTest/QtTest>
#include <memory>
// 为了访问私有成员变量和方法
// #define private public
// #define protected public

#include "core/groupmanager.h"
#include "core/resourcemanagerinterface.h"
// #undef private
// #undef protected

// 创建一个ResourceManagerInterface的模拟类
// class MockResourceManagerInterface : public ResourceManagerInterface {
// public:
//   MockResourceManagerInterface() = default;

//   // 用于记录方法调用
//   mutable struct CallRecords {
//     bool setCurrentUserServiceUnitPropertyEnabled = false;
//     bool reclaimProcessGroups = false;
//     bool setProcessGroupResourceLimit = false;
//     std::string limitController;
//     std::string limitFile;
//     std::string limitValue;
//     std::vector<std::string> reclaimedGroups;
//   } callRecords;

//   bool
//   setCurrentUserServiceUnitPropertyEnabled(const std::string & /*propertyName*/,
//                                            bool /*enabled*/) const {
//     callRecords.setCurrentUserServiceUnitPropertyEnabled = true;
//     return true;
//   }

//   bool reclaimProcessGroups(
//       const std::vector<std::string> &groupNames) const {
//     callRecords.reclaimProcessGroups = true;
//     callRecords.reclaimedGroups = groupNames;
//     return true;
//   }

//   bool setProcessGroupResourceLimit(const std::string & /*attributeName*/,
//                                     const std::string & /*groupPath*/,
//                                     const std::string &controller,
//                                     const std::string &file,
//                                     const std::string &value) const {
//     callRecords.setProcessGroupResourceLimit = true;
//     callRecords.limitController = controller;
//     callRecords.limitFile = file;
//     callRecords.limitValue = value;
//     return true;
//   }

//   void resetRecords() { callRecords = CallRecords(); }
// };

// // 创建一个GroupManagementUnit的模拟类
// class MockGroupManagementUnit : public GroupManagementUnit {
// public:
//   MockGroupManagementUnit(sched_policy::GroupType type)
//       : GroupManagementUnit({}, {}, {}) {}

//   // 用于记录方法调用
//   mutable struct CallRecords {
//     bool executeResourceLimit = false;
//     bool createTransientProcessGroup = false;
//     bool moveProcessToGroup = false;
//     std::string policyId;
//     std::string createdGroupName;
//     std::vector<int> movedPids;
//     std::string targetGroupPath;
//   } callRecords;

//   void executeResourceLimit(const std::string &policyId) {
//     callRecords.executeResourceLimit = true;
//     callRecords.policyId = policyId;
//   }

//   std::string
//   createTransientProcessGroup(const std::string &cgroupName,
//                               const std::vector<int> & /*pids*/) {
//     callRecords.createTransientProcessGroup = true;
//     callRecords.createdGroupName = cgroupName;
//     return "/test/" + cgroupName;
//   }

//   void moveProcessToGroup(const std::string &targetGroupPath,
//                           const std::vector<int> &pids) {
//     callRecords.moveProcessToGroup = true;
//     callRecords.movedPids = pids;
//     callRecords.targetGroupPath = targetGroupPath;
//   }

//   std::string groupPath() const { return "/test/group"; }

//   void resetRecords() { callRecords = CallRecords(); }
// };

// // 创建GroupManagementUnitCreator的模拟类
// class MockGroupManagementUnitCreator : public GroupManagementUnitCreator {
// public:
//   MockGroupManagementUnitCreator()
//       : GroupManagementUnitCreator(std::make_shared<ResourceManagerInterface>(),
//                                    Json::Value(), Json::Value()) {}

//   std::unique_ptr<GroupManagementUnit>
//   createGroupManagementUnit(const Json::Value & /*groupConfig*/) {
//     // 根据测试需要返回不同类型的MockGroupManagementUnit
//     return std::unique_ptr<GroupManagementUnit>(new MockGroupManagementUnit(
//         sched_policy::GroupType::TopGroup));
//   }
// };

// 测试GroupManager类
class TestGroupManager : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    // 测试用例
    void testSetGroupConfigs();
    void testExecutePolicy();
    // void testCreateProcessGroup();
    // void testMoveProcessToGroup();
    void testGroupPath();
    void testFreezeGroups();
    void testReclaimProcessGroups();
    void testGetFreezerAttribute();

private:
    GroupManager *m_groupManager;
    // MockResourceManagerInterface *m_mockResourceManager;
    // MockGroupManagementUnitCreator *m_mockUnitCreator;
    // MockGroupManagementUnit *m_mockUnit;

    // 辅助方法
    Json::Value createTestConfig();
};

void TestGroupManager::initTestCase()
{
    // 创建测试配置
}

void TestGroupManager::cleanupTestCase()
{
    // 清理资源
}

void TestGroupManager::init()
{
    // 每个测试前的初始化
    m_groupManager = new GroupManager();
    //   m_mockResourceManager = new MockResourceManagerInterface();
    //   m_mockUnitCreator = new MockGroupManagementUnitCreator();
    //   m_mockUnit = new MockGroupManagementUnit(sched_policy::GroupType::TopGroup);

    //   MockResourceManagerInterface mrci;
    //   // 替换GroupManager内部的成员为Mock对象
    // //   m_groupManager->m_resourceManagerInterface = mrci;
    //   m_groupManager->m_unitCreator.reset(m_mockUnitCreator);

    //   // 添加测试用的GroupManagementUnit
    //   m_groupManager->m_groupUnits[sched_policy::GroupType::TopGroup].reset(
    //       m_mockUnit);

    //   // 设置测试配置
    //   m_groupManager->m_groupConfigs = createTestConfig();
}

void TestGroupManager::cleanup()
{
    // 每个测试后的清理
    // m_mockResourceManager->resetRecords();
    // m_mockUnit->resetRecords();

    // // GroupManager会在析构时删除m_unitCreator和m_groupUnits中的对象，所以在此先释放所有权
    // m_groupManager->m_unitCreator.release();
    // m_groupManager->m_groupUnits[sched_policy::GroupType::TopGroup].release();

    delete m_groupManager;
    // delete m_mockResourceManager;
}

Json::Value TestGroupManager::createTestConfig()
{
    Json::Value config;

    // 创建AggregateProfiles数组
    Json::Value aggregateProfiles(Json::arrayValue);
    Json::Value profile;
    profile["Name"] = "TopApp";
    profile["Group"] = "TopApp";
    profile["Type"] = static_cast<int>(sched_policy::GroupType::TopGroup);
    aggregateProfiles.append(profile);

    // 创建ControllerList数组
    Json::Value controllers(Json::arrayValue);
    controllers.append("cpu");
    controllers.append("memory");

    // 创建Attributes数组
    Json::Value attributes(Json::arrayValue);
    Json::Value freezerAttr;
    freezerAttr["Name"] = "Freezer";
    freezerAttr["Controller"] = "freezer";
    freezerAttr["File"] = "freezer.state";
    attributes.append(freezerAttr);

    // 添加到配置中
    config["AggregateProfiles"] = aggregateProfiles;
    config["ControllerList"] = controllers;
    config["Attributes"] = attributes;

    return config;
}

void TestGroupManager::testSetGroupConfigs()
{
    // 创建测试配置
    Json::Value testConfig = createTestConfig();

    // 调用被测试方法
    m_groupManager->setGroupConfigs(testConfig);
}

void TestGroupManager::testExecutePolicy()
{
    // 调用被测试方法
    m_groupManager->excutePolicy("test-policy");
}

// void TestGroupManager::testCreateProcessGroup() {
//   // 准备测试数据
//   std::string groupName = "test-group";
//   std::vector<int> pids = {1001, 1002, 1003};

//   // 调用被测试方法
//   std::string result = m_groupManager->createProcessGroup(
//       groupName, pids, sched_policy::GroupType::TopGroup);

//   // 验证结果
// //  QCOMPARE(m_mockUnit->callRecords.createTransientProcessGroup, true);
// //  QCOMPARE(m_mockUnit->callRecords.createdGroupName, groupName);
// //  QCOMPARE(result, std::string("/test/") + groupName);

//   // 测试不存在的组类型
//   result = m_groupManager->createProcessGroup(
//       groupName, pids, sched_policy::GroupType::ServiceGroup);

//   // 应该返回空字符串
// //  QCOMPARE(result, std::string(""));
// }

// void TestGroupManager::testMoveProcessToGroup() {
//   // 准备测试数据
//   int parentPid = 1000;
//   std::vector<int> childPids = {1001, 1002, 1003};

//   // 修改process_info_helper::cgroup的行为（这里需要多做一些工作，可能需要宏定义或者其他方式）
//   // 简单起见，假设调用moveProcessToGroup时已经能获取到正确的cgroup路径

//   // 调用被测试方法
//   m_groupManager->moveProcessToGroup(parentPid, childPids,
//                                      sched_policy::GroupType::TopGroup);

//   // 验证结果
//   QCOMPARE(m_mockUnit->callRecords.moveProcessToGroup, true);
//   QCOMPARE(m_mockUnit->callRecords.movedPids, childPids);

//   // 测试不存在的组类型
//   m_mockUnit->resetRecords();
//   m_groupManager->moveProcessToGroup(parentPid, childPids,
//                                      sched_policy::GroupType::ServiceGroup);

//   // 不应该调用moveProcessToGroup
//   QCOMPARE(m_mockUnit->callRecords.moveProcessToGroup, false);
// }

void TestGroupManager::testGroupPath()
{
    // 调用被测试方法
    std::string path = m_groupManager->groupPath(sched_policy::GroupType::TopGroup);

    // 验证结果
    QCOMPARE(path, std::string("/test/group"));

    // 测试不存在的组类型
    path = m_groupManager->groupPath(sched_policy::GroupType::ServiceGroup);

    // 应该返回空字符串
    QCOMPARE(path, std::string(""));
}

void TestGroupManager::testFreezeGroups()
{
    // 准备测试数据
    std::vector<std::string> groupNames = {"group1", "group2"};

    // 调用被测试方法
    m_groupManager->freezeGroups(groupNames);

    // 验证结果
    // QCOMPARE(m_mockResourceManager->callRecords.setProcessGroupResourceLimit,
    //          true);
    // QCOMPARE(m_mockResourceManager->callRecords.limitController,
    //          std::string("freezer"));
    // QCOMPARE(m_mockResourceManager->callRecords.limitFile,
    //          std::string("freezer.state"));
    // QCOMPARE(m_mockResourceManager->callRecords.limitValue, std::string("1"));
}

void TestGroupManager::testReclaimProcessGroups()
{
    // 准备测试数据
    std::vector<std::string> groupNames = {"group1", "group2"};

    // 调用被测试方法
    m_groupManager->reclaimProcessGroups(groupNames);

    // 验证结果
    // QCOMPARE(m_mockResourceManager->callRecords.reclaimProcessGroups, true);
    // QCOMPARE(m_mockResourceManager->callRecords.reclaimedGroups, groupNames);
}

void TestGroupManager::testGetFreezerAttribute()
{
    // // 调用被测试方法
    // Json::Value freezerAttr = m_groupManager->getFreezerAttribute();

    // // 验证结果
    // QCOMPARE(freezerAttr["Name"].asString(), std::string("Freezer"));
    // QCOMPARE(freezerAttr["Controller"].asString(), std::string("freezer"));
    // QCOMPARE(freezerAttr["File"].asString(), std::string("freezer.state"));
}

QTEST_MAIN(TestGroupManager)
#include "tst_groupmanager.moc"
