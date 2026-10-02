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

#include <QList>
#include <QtTest/QtTest>

// 定义private为public以便访问私有成员和方法
#define private public
#define protected public
#include "../base/daemondbusinterface.h"
#include "../base/systemddbusinterface.h"
#include "../core/resourcemanagerinterface.h"
#include "../utils/misc.h"
#undef private
#undef protected

// 模拟DaemonDbusInterface类
class MockDaemonDbusInterface
{
public:
    // 记录方法调用参数
    struct CallRecord
    {
        std::string path;
        std::vector<std::string> controllers;
        std::vector<int> pids;
        std::string controller;
        std::string file;
        std::string value;
        std::string unitName;
        std::string property;
        bool enabled;
        std::vector<std::string> groupNames;
    };

    static CallRecord lastCall;
    static void reset()
    {
        lastCall = CallRecord();
    }

    // 模拟方法
    void createProcessGroup(
        const std::string &path,
        const std::vector<std::string> &controllers,
        const std::vector<int> &pids)
    {
        lastCall.path = path;
        lastCall.controllers = controllers;
        lastCall.pids = pids;
    }

    void setProcessGroupResourceLimit(
        const std::string &path, const std::string &controller,
        const std::string &file, const std::string &value)
    {
        lastCall.path = path;
        lastCall.controller = controller;
        lastCall.file = file;
        lastCall.value = value;
    }

    void moveProcessToGroup(
        const std::string &path,
        const std::vector<std::string> &controllers,
        const std::vector<int> &pids)
    {
        lastCall.path = path;
        lastCall.controllers = controllers;
        lastCall.pids = pids;
    }

    void setSystemdUnitPropertyEnabled(
        const std::string &unitName,
        const std::string &property,
        bool enabled)
    {
        lastCall.unitName = unitName;
        lastCall.property = property;
        lastCall.enabled = enabled;
    }

    void reclaimProcessGroups(const std::vector<std::string> &groupNames)
    {
        lastCall.groupNames = groupNames;
    }
};

// 初始化静态成员
MockDaemonDbusInterface::CallRecord MockDaemonDbusInterface::lastCall;

// 模拟SystemdDbusInterface类
class MockSystemdDbusInterface
{
public:
    // 记录方法调用参数
    struct CallRecord
    {
        std::string unitName;
        std::vector<int> pids;
        std::string parentSlice;
        std::string attributeName;
        std::string value;
    };

    static CallRecord lastCall;
    static std::string lastUserService;

    static void reset()
    {
        lastCall = CallRecord();
        lastUserService = "user@1000.service"; // 默认用户服务名
    }

    // 模拟方法
    std::string currentUserServiceName() const
    {
        return lastUserService;
    }

    void createPersistentProcessGroup(const std::string &unitName)
    {
        lastCall.unitName = unitName;
    }

    void createTransientProcessGroup(
        const std::string &unitName,
        const std::vector<int> &pids,
        const std::string &parentSlice)
    {
        lastCall.unitName = unitName;
        lastCall.pids = pids;
        lastCall.parentSlice = parentSlice;
    }

    void setProcessGroupResourceLimit(
        const std::string &unitName,
        const std::string &attributeName,
        const std::string &value)
    {
        lastCall.unitName = unitName;
        lastCall.attributeName = attributeName;
        lastCall.value = value;
    }
};

// 初始化静态成员
MockSystemdDbusInterface::CallRecord MockSystemdDbusInterface::lastCall;
std::string MockSystemdDbusInterface::lastUserService = "user@1000.service";

// 模拟misc::version::cgroupVersion函数，不覆盖原函数
class CgroupVersionMocker
{
public:
    static int s_cgroupVersion;

    static void setCgroupVersion(int version)
    {
        s_cgroupVersion = version;
    }
};

int CgroupVersionMocker::s_cgroupVersion = 2; // 默认为 cgroup v2

// 替换原始类为模拟类
#define DaemonDbusInterface MockDaemonDbusInterface
#define SystemdDbusInterface MockSystemdDbusInterface

// 在测试类中重写misc::version::cgroupVersion函数
namespace misc
{
namespace version
{
// 利用函数弱符号链接特性，测试中的实现会覆盖原有实现
__attribute__((weak)) int cgroupVersion()
{
    return CgroupVersionMocker::s_cgroupVersion;
}
}
}

// 测试类
class TestResourceManagerInterface : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试用例
    void testCreatePersistentProcessGroup_CgroupV1();
    void testCreatePersistentProcessGroup_CgroupV2();
    void testCreateTransientProcessGroup_CgroupV1();
    void testCreateTransientProcessGroup_CgroupV2();
    void testSetProcessGroupResourceLimit_CgroupV1();
    void testSetProcessGroupResourceLimit_CgroupV2();
    void testMoveProcessToGroup();
    void testSetCurrentUserServiceUnitPropertyEnabled();
    void testReclaimProcessGroups();
    void testGetGroupNameFromPath();
    void testGetGroupParentSliceFromPath();

private:
    ResourceManagerInterface *m_resourceManager;
};

void TestResourceManagerInterface::initTestCase()
{
    // 测试套件初始化
}

void TestResourceManagerInterface::cleanupTestCase()
{
    // 测试套件清理
}

void TestResourceManagerInterface::init()
{
    // 每个测试前初始化
    MockDaemonDbusInterface::reset();
    MockSystemdDbusInterface::reset();
    CgroupVersionMocker::setCgroupVersion(2); // 默认设置为 cgroup v2
    m_resourceManager = new ResourceManagerInterface();
}

void TestResourceManagerInterface::cleanup()
{
    // 每个测试后清理
    delete m_resourceManager;
}

void TestResourceManagerInterface::testCreatePersistentProcessGroup_CgroupV1()
{
    // 设置cgroup版本为v1
    CgroupVersionMocker::setCgroupVersion(1);

    // 测试数据
    std::string path = "/test/group/path";
    std::vector<std::string> controllers = {"cpu", "memory"};

    // 执行测试
    m_resourceManager->createPersistentProcessGroup(path, controllers);

    // 验证结果
    QCOMPARE(QString::fromStdString(MockDaemonDbusInterface::lastCall.path), QString::fromStdString(path));
    QVERIFY(MockDaemonDbusInterface::lastCall.pids.empty());
}

void TestResourceManagerInterface::testCreatePersistentProcessGroup_CgroupV2()
{
    // 设置cgroup版本为v2
    CgroupVersionMocker::setCgroupVersion(2);

    // 测试数据
    std::string path = "/test/group/path";
    std::vector<std::string> controllers = {"cpu", "memory"};

    // 执行测试
    m_resourceManager->createPersistentProcessGroup(path, controllers);

    // 验证结果 - 应该调用SystemdDbusInterface的createPersistentProcessGroup
    QCOMPARE(QString::fromStdString(MockSystemdDbusInterface::lastCall.unitName), QString::fromStdString("path"));
}

void TestResourceManagerInterface::testCreateTransientProcessGroup_CgroupV1()
{
    // 设置cgroup版本为v1
    CgroupVersionMocker::setCgroupVersion(1);

    // 测试数据
    std::string path = "/test/group/path";
    std::vector<int> pids = {1234, 5678};
    std::vector<std::string> controllers = {"cpu", "memory"};

    // 执行测试
    m_resourceManager->createTransientProcessGroup(path, pids, controllers);

    // 验证结果 - 应该调用DaemonDbusInterface和SystemdDbusInterface
    QCOMPARE(QString::fromStdString(MockDaemonDbusInterface::lastCall.path), QString::fromStdString(path));
    QCOMPARE(QList<int>::fromVector(QVector<int>::fromStdVector(MockDaemonDbusInterface::lastCall.pids)),
             QList<int>::fromVector(QVector<int>::fromStdVector(pids)));

    // 也应该调用SystemdDbusInterface
    QCOMPARE(QString::fromStdString(MockSystemdDbusInterface::lastCall.unitName), QString::fromStdString("path"));
    QCOMPARE(QList<int>::fromVector(QVector<int>::fromStdVector(MockSystemdDbusInterface::lastCall.pids)),
             QList<int>::fromVector(QVector<int>::fromStdVector(pids)));
    QCOMPARE(QString::fromStdString(MockSystemdDbusInterface::lastCall.parentSlice), QString::fromStdString("group"));
}

void TestResourceManagerInterface::testCreateTransientProcessGroup_CgroupV2()
{
    // 设置cgroup版本为v2
    CgroupVersionMocker::setCgroupVersion(2);

    // 测试数据
    std::string path = "/test/group/path";
    std::vector<int> pids = {1234, 5678};
    std::vector<std::string> controllers = {"cpu", "memory"};

    // 执行测试
    m_resourceManager->createTransientProcessGroup(path, pids, controllers);

    // 验证结果 - 只应该调用SystemdDbusInterface
    QCOMPARE(QString::fromStdString(MockSystemdDbusInterface::lastCall.unitName), QString::fromStdString("path"));
    QCOMPARE(QList<int>::fromVector(QVector<int>::fromStdVector(MockSystemdDbusInterface::lastCall.pids)),
             QList<int>::fromVector(QVector<int>::fromStdVector(pids)));
    QCOMPARE(QString::fromStdString(MockSystemdDbusInterface::lastCall.parentSlice), QString::fromStdString("group"));
}

void TestResourceManagerInterface::testSetProcessGroupResourceLimit_CgroupV1()
{
    // 设置cgroup版本为v1
    CgroupVersionMocker::setCgroupVersion(1);

    // 测试数据
    std::string attributeName = "CPUWeight";
    std::string path = "/test/group/path";
    std::string controller = "cpu";
    std::string file = "cpu.weight";
    std::string value = "100";

    // 执行测试
    m_resourceManager->setProcessGroupResourceLimit(attributeName, path, controller, file, value);

    // 验证结果 - 应该调用DaemonDbusInterface
    QCOMPARE(QString::fromStdString(MockDaemonDbusInterface::lastCall.path), QString::fromStdString(path));
    QCOMPARE(QString::fromStdString(MockDaemonDbusInterface::lastCall.controller), QString::fromStdString(controller));
    QCOMPARE(QString::fromStdString(MockDaemonDbusInterface::lastCall.file), QString::fromStdString(file));
    QCOMPARE(QString::fromStdString(MockDaemonDbusInterface::lastCall.value), QString::fromStdString(value));
}

void TestResourceManagerInterface::testSetProcessGroupResourceLimit_CgroupV2()
{
    // 设置cgroup版本为v2
    CgroupVersionMocker::setCgroupVersion(2);

    // 测试数据
    std::string attributeName = "CPUWeight";
    std::string path = "/test/group/path";
    std::string controller = "cpu";
    std::string file = "cpu.weight";
    std::string value = "100";

    // 执行测试
    m_resourceManager->setProcessGroupResourceLimit(attributeName, path, controller, file, value);

    // 验证结果 - 应该调用SystemdDbusInterface
    QCOMPARE(QString::fromStdString(MockSystemdDbusInterface::lastCall.unitName), QString::fromStdString("path"));
    QCOMPARE(QString::fromStdString(MockSystemdDbusInterface::lastCall.attributeName), QString::fromStdString(attributeName));
    QCOMPARE(QString::fromStdString(MockSystemdDbusInterface::lastCall.value), QString::fromStdString(value));
}

void TestResourceManagerInterface::testMoveProcessToGroup()
{
    // 测试数据
    std::string path = "/test/group/path";
    std::vector<std::string> controllers = {"cpu", "memory"};
    std::vector<int> pids = {1234, 5678};

    // 执行测试
    m_resourceManager->moveProcessToGroup(path, controllers, pids);

    // 验证结果
    QCOMPARE(QString::fromStdString(MockDaemonDbusInterface::lastCall.path), QString::fromStdString(path));
    QCOMPARE(QList<int>::fromVector(QVector<int>::fromStdVector(MockDaemonDbusInterface::lastCall.pids)),
             QList<int>::fromVector(QVector<int>::fromStdVector(pids)));
}

void TestResourceManagerInterface::testSetCurrentUserServiceUnitPropertyEnabled()
{
    // 测试数据
    std::string property = "test.property";
    bool enabled = true;
    MockSystemdDbusInterface::lastUserService = "user@1001.service";

    // 执行测试
    m_resourceManager->setCurrentUserServiceUnitPropertyEnabled(property, enabled);

    // 验证结果
    QCOMPARE(QString::fromStdString(MockDaemonDbusInterface::lastCall.unitName), QString::fromStdString("user@1001.service"));
    QCOMPARE(QString::fromStdString(MockDaemonDbusInterface::lastCall.property), QString::fromStdString(property));
    QCOMPARE(MockDaemonDbusInterface::lastCall.enabled, enabled);
}

void TestResourceManagerInterface::testReclaimProcessGroups()
{
    // 测试数据
    std::vector<std::string> groupNames = {"group1", "group2", "group3"};

    // 执行测试
    m_resourceManager->reclaimProcessGroups(groupNames);
}

void TestResourceManagerInterface::testGetGroupNameFromPath()
{
    // 测试数据及预期结果
    struct TestCase
    {
        std::string path;
        std::string expectedResult;
    };

    std::vector<TestCase> testCases = {
        {"/test/group/path", "path"},
        {"/single", "single"},
        {"", ""},
        {"/", ""},
        {"/multiple/levels/of/nesting", "nesting"}};

    for (const auto &testCase : testCases) {
        std::string result = m_resourceManager->getGroupNameFromPath(testCase.path);
        QCOMPARE(QString::fromStdString(result), QString::fromStdString(testCase.expectedResult));
    }
}

void TestResourceManagerInterface::testGetGroupParentSliceFromPath()
{
    // 测试数据及预期结果
    struct TestCase
    {
        std::string path;
        std::string expectedResult;
    };

    std::vector<TestCase> testCases = {
        {"/test/group/path", "group"},
        {"/single", ""},
        {"", ""},
        {"/", ""},
        {"/multiple/levels/of/nesting", "of"}};

    for (const auto &testCase : testCases) {
        std::string result = m_resourceManager->getGroupParentSliceFromPath(testCase.path);
        QCOMPARE(QString::fromStdString(result), QString::fromStdString(testCase.expectedResult));
    }
}

// 取消宏定义
#undef DaemonDbusInterface
#undef SystemdDbusInterface

QTEST_MAIN(TestResourceManagerInterface)
#include "tst_resourcemanagerinterface.moc"
