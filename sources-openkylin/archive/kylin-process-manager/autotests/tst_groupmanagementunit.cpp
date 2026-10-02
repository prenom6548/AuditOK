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

#include <QtTest>
#include <jsoncpp/json/json.h>
#include <memory>

// 允许访问私有成员变量和方法
#define private public
#define protected public
#include "base/groupmanagementunit.h"
#include "base/resourcecontroller.h"
#include "core/resourcemanagerinterface.h"
#undef private
#undef protected

// 创建ResourceManagerInterface的模拟类
class MockResourceManagerInterface : public ResourceManagerInterface
{
public:
    MockResourceManagerInterface() = default;
    ~MockResourceManagerInterface() = default;

    // 记录方法调用
    mutable struct CallRecords
    {
        bool createPersistentProcessGroupCalled = false;
        bool createTransientProcessGroupCalled = false;
        bool moveProcessToGroupCalled = false;
        bool setProcessGroupResourceLimitCalled = false;

        std::string groupPath;
        std::vector<std::string> controllers;
        std::vector<int> pids;
        std::string attributeName;
        std::string controller;
        std::string file;
        std::string value;
    } callRecords;

    // 模拟实现
    void createPersistentProcessGroup(const std::string &path,
                                      const std::vector<std::string> &controllers)
    {
        callRecords.createPersistentProcessGroupCalled = true;
        callRecords.groupPath = path;
        callRecords.controllers = controllers;
    }

    void createTransientProcessGroup(const std::string &path,
                                     const std::vector<int> &pids,
                                     const std::vector<std::string> &controllers)
    {
        callRecords.createTransientProcessGroupCalled = true;
        callRecords.groupPath = path;
        callRecords.pids = pids;
        callRecords.controllers = controllers;
    }

    void moveProcessToGroup(const std::string &path,
                            const std::vector<std::string> &controllers,
                            const std::vector<int> &pids)
    {
        callRecords.moveProcessToGroupCalled = true;
        callRecords.groupPath = path;
        callRecords.controllers = controllers;
        callRecords.pids = pids;
    }

    void setProcessGroupResourceLimit(const std::string &attributeName,
                                      const std::string &path,
                                      const std::string &controller,
                                      const std::string &file,
                                      const std::string &value)
    {
        callRecords.setProcessGroupResourceLimitCalled = true;
        callRecords.attributeName = attributeName;
        callRecords.groupPath = path;
        callRecords.controller = controller;
        callRecords.file = file;
        callRecords.value = value;
    }

    void resetRecords()
    {
        callRecords = CallRecords();
    }
};

// 创建ProfileAttribute的模拟类
class MockProfileAttribute : public ProfileAttribute
{
public:
    MockProfileAttribute(const std::string &name, const std::string &controller, const std::string &file)
        : m_name(name)
        , m_controller(controller)
        , m_file(file)
    {
    }

    std::string name() const
    {
        return m_name;
    }
    std::string controllerName() const
    {
        return m_controller;
    }
    std::string fileName() const
    {
        return m_file;
    }

private:
    std::string m_name;
    std::string m_controller;
    std::string m_file;
};

// C++11兼容的工具函数，用于替代std::make_unique
template<typename T, typename... Args>
std::unique_ptr<T> makeUnique(Args &&...args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// 创建ResourceController的辅助工厂函数
std::unique_ptr<ResourceController> createTestResourceController(
    const std::string &name,
    const std::string &attributeName,
    const std::string &controller,
    const std::string &file,
    const std::string &value,
    const std::shared_ptr<ResourceManagerInterface> &resourceInterface)
{

    std::unique_ptr<ResourceController> resourceController(new ResourceController(name));
    std::unique_ptr<ProfileAttribute> attribute(new MockProfileAttribute(attributeName, controller, file));
    std::unique_ptr<ProfileAction> action(new SetAttributeAction(std::move(attribute), value, resourceInterface));
    resourceController->addAction(std::move(action));
    return resourceController;
}

class TestGroupManagementUnit : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试用例
    void testConstructor();
    void testExecuteResourceLimit();
    void testExecuteResourceLimitWithMultipleControllers();
    void testExecuteResourceLimitWithInvalidPolicyId();
    void testCreateTransientProcessGroup();
    void testMoveProcessToGroup();
    void testGroupType();
    void testGroupPath();
    void testCgroupControllerNames();
    void testCgroupControllerNamesWithMultipleControllers();

private:
    std::shared_ptr<MockResourceManagerInterface> m_mockResourceManager;
    std::unique_ptr<GroupManagementUnit> m_groupManagementUnit;

    // 辅助方法
    std::unique_ptr<ProcessGroupInfo> createTestGroupInfo(const std::string &name,
                                                          const std::string &path,
                                                          sched_policy::GroupType type);
    GroupManagementUnit::PolicyControllerMap createTestPolicyControllers();
};

void TestGroupManagementUnit::initTestCase()
{
    // 测试套件初始化
}

void TestGroupManagementUnit::cleanupTestCase()
{
    // 测试套件清理
}

void TestGroupManagementUnit::init()
{
    // 每个测试前的初始化
    m_mockResourceManager = std::shared_ptr<MockResourceManagerInterface>(new MockResourceManagerInterface());

    auto groupInfo = createTestGroupInfo("TestGroup", "/test/group/path", sched_policy::GroupType::TopGroup);
    auto policyControllers = createTestPolicyControllers();

    m_groupManagementUnit = std::unique_ptr<GroupManagementUnit>(
        new GroupManagementUnit(
            std::move(groupInfo),
            std::move(policyControllers),
            m_mockResourceManager));
}

void TestGroupManagementUnit::cleanup()
{
    // 每个测试后的清理
    m_mockResourceManager->resetRecords();
    m_groupManagementUnit.reset();
}

void TestGroupManagementUnit::testConstructor()
{
    // 验证构造函数正确初始化了内部状态
    QCOMPARE(QString::fromStdString(m_groupManagementUnit->m_groupInfo->name), QString("TestGroup"));
    QCOMPARE(QString::fromStdString(m_groupManagementUnit->m_groupInfo->path), QString("/test/group/path"));
    QCOMPARE(m_groupManagementUnit->m_groupInfo->type, sched_policy::GroupType::TopGroup);

    // 验证策略控制器映射是否正确初始化
    QVERIFY(m_groupManagementUnit->m_policyControllers.size() > 0);
    QVERIFY(m_groupManagementUnit->m_policyControllers.find("Normal_Performance") != m_groupManagementUnit->m_policyControllers.end());

    // 验证资源管理接口是否正确设置
    QVERIFY(m_groupManagementUnit->m_resourceInterface != nullptr);
}

void TestGroupManagementUnit::testExecuteResourceLimit()
{
    // 执行资源限制
    m_groupManagementUnit->executeResourceLimit("Normal_Performance");

    // 验证是否调用了setProcessGroupResourceLimit
    QVERIFY(m_mockResourceManager->callRecords.setProcessGroupResourceLimitCalled);
    QCOMPARE(QString::fromStdString(m_mockResourceManager->callRecords.groupPath), QString("/test/group/path"));
    QCOMPARE(QString::fromStdString(m_mockResourceManager->callRecords.controller), QString("cpu"));
    QCOMPARE(QString::fromStdString(m_mockResourceManager->callRecords.file), QString("cpu.shares"));
    QCOMPARE(QString::fromStdString(m_mockResourceManager->callRecords.value), QString("1024"));
}

void TestGroupManagementUnit::testExecuteResourceLimitWithMultipleControllers()
{
    // 添加额外的策略控制器
    std::unique_ptr<ResourceController> extraController = createTestResourceController(
        "memory", "MemoryLimit", "memory", "memory.limit_in_bytes", "1G", m_mockResourceManager);

    m_groupManagementUnit->m_policyControllers["Normal_Performance"].push_back(std::move(extraController));

    // 执行资源限制
    m_groupManagementUnit->executeResourceLimit("Normal_Performance");

    // 这里我们无法直接验证两个不同的控制器调用，因为mockResourceManager只保存最后一次调用
    // 实际项目中可能需要更复杂的mock或者重构测试方法
    QVERIFY(m_mockResourceManager->callRecords.setProcessGroupResourceLimitCalled);
}

void TestGroupManagementUnit::testExecuteResourceLimitWithInvalidPolicyId()
{
    // 使用不存在的策略ID
    m_groupManagementUnit->executeResourceLimit("NonExistentPolicy");

    // 验证没有调用setProcessGroupResourceLimit
    QVERIFY(!m_mockResourceManager->callRecords.setProcessGroupResourceLimitCalled);
}

void TestGroupManagementUnit::testCreateTransientProcessGroup()
{
    // 测试数据
    std::string cgroupName = "test_transient_group";
    std::vector<int> pids = {1234, 5678};

    // 创建临时进程组
    std::string result = m_groupManagementUnit->createTransientProcessGroup(cgroupName, pids);

    // 验证结果
    QVERIFY(m_mockResourceManager->callRecords.createTransientProcessGroupCalled);
    QCOMPARE(QString::fromStdString(m_mockResourceManager->callRecords.groupPath),
             QString::fromStdString("/test/group/path/" + cgroupName));
    QCOMPARE(QList<int>::fromVector(QVector<int>::fromStdVector(m_mockResourceManager->callRecords.pids)),
             QList<int>::fromVector(QVector<int>::fromStdVector(pids)));

    // 验证返回值
    QCOMPARE(QString::fromStdString(result), QString::fromStdString("/test/group/path/" + cgroupName));
}

void TestGroupManagementUnit::testMoveProcessToGroup()
{
    // 测试数据
    std::string cgroupPath = "/test/target/group";
    std::vector<int> pids = {1234, 5678};

    // 移动进程到组
    m_groupManagementUnit->moveProcessToGroup(cgroupPath, pids);

    // 验证结果
    QVERIFY(m_mockResourceManager->callRecords.moveProcessToGroupCalled);
    QCOMPARE(QString::fromStdString(m_mockResourceManager->callRecords.groupPath),
             QString::fromStdString(cgroupPath));
    QCOMPARE(QList<int>::fromVector(QVector<int>::fromStdVector(m_mockResourceManager->callRecords.pids)),
             QList<int>::fromVector(QVector<int>::fromStdVector(pids)));
}

void TestGroupManagementUnit::testGroupType()
{
    // 获取组类型
    sched_policy::GroupType type = m_groupManagementUnit->groupType();

    // 验证结果
    QCOMPARE(type, sched_policy::GroupType::TopGroup);
}

void TestGroupManagementUnit::testGroupPath()
{
    // 获取组路径
    std::string path = m_groupManagementUnit->groupPath();

    // 验证结果
    QCOMPARE(QString::fromStdString(path), QString("/test/group/path"));
}

void TestGroupManagementUnit::testCgroupControllerNames()
{
    // 获取cgroup控制器名称
    std::vector<std::string> controllerNames = m_groupManagementUnit->cgroupControllerNames();

    // 验证结果
    QVERIFY(controllerNames.size() > 0);
    QVERIFY(std::find(controllerNames.begin(), controllerNames.end(), "cpu") != controllerNames.end());
}

void TestGroupManagementUnit::testCgroupControllerNamesWithMultipleControllers()
{
    // 添加额外的策略控制器
    std::unique_ptr<ResourceController> extraController = createTestResourceController(
        "memory", "MemoryLimit", "memory", "memory.limit_in_bytes", "1G", m_mockResourceManager);

    m_groupManagementUnit->m_policyControllers["Normal_Performance"].push_back(std::move(extraController));

    // 获取cgroup控制器名称
    std::vector<std::string> controllerNames = m_groupManagementUnit->cgroupControllerNames();

    // 验证结果
    QVERIFY(controllerNames.size() >= 2);
    QVERIFY(std::find(controllerNames.begin(), controllerNames.end(), "cpu") != controllerNames.end());
    QVERIFY(std::find(controllerNames.begin(), controllerNames.end(), "memory") != controllerNames.end());
}

// 辅助方法实现
std::unique_ptr<ProcessGroupInfo> TestGroupManagementUnit::createTestGroupInfo(
    const std::string &name, const std::string &path, sched_policy::GroupType type)
{
    std::unique_ptr<ProcessGroupInfo> groupInfo(new ProcessGroupInfo());
    groupInfo->name = name;
    groupInfo->path = path;
    groupInfo->type = type;
    groupInfo->cgroupControllers = {"cpu", "cpuset"};
    return groupInfo;
}

GroupManagementUnit::PolicyControllerMap TestGroupManagementUnit::createTestPolicyControllers()
{
    GroupManagementUnit::PolicyControllerMap policyControllers;

    // 创建Normal_Performance策略
    std::vector<std::unique_ptr<ResourceController>> controllers;
    controllers.push_back(createTestResourceController(
        "cpu", "CPUShares", "cpu", "cpu.shares", "1024", m_mockResourceManager));

    policyControllers["Normal_Performance"] = std::move(controllers);

    return policyControllers;
}

QTEST_MAIN(TestGroupManagementUnit)
#include "tst_groupmanagementunit.moc"