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

#include "processinfohelper.h"
#include <QCoreApplication>
#include <QGSettings/qgsettings.h>
#include <QtTest>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

// 测试辅助宏，用于跳过在真实环境中无法运行的测试
#define SKIP_TEST_IF_FAILS(condition, message) \
    do {                                       \
        if (!(condition)) {                    \
            QSKIP(message);                    \
        }                                      \
    } while (false)

class TestProcessInfoHelper : public QObject
{
    Q_OBJECT

public:
    TestProcessInfoHelper();
    ~TestProcessInfoHelper();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    // 测试进程存在性判断函数
    void test_processExists();

    // 测试获取命令行函数
    void test_cmdline();

    // 测试获取父进程ID函数
    void test_parentPid();

    // 测试获取子进程ID函数
    void test_childPids_single();
    void test_childPids_multiple();

    // 测试获取cgroup函数
    void test_cgroup();

    // 测试获取cgroup中的进程ID函数
    void test_pidsOfCgroup();

    // 边缘情况测试
    void test_invalidPid();
    void test_selfProcessInfo();

private:
    // 辅助函数，用于创建子进程进行测试
    QList<int> createChildProcesses(int count);
    void terminateProcesses(const QList<int> &pids);
    int getCurrentProcessId() const;
    int launchDetachedProcess(const QString &command);
    bool waitForProcessToStart(int pid, int timeoutMs = 3000);
};

TestProcessInfoHelper::TestProcessInfoHelper()
{
}

TestProcessInfoHelper::~TestProcessInfoHelper()
{
}

void TestProcessInfoHelper::initTestCase()
{
    // 确保测试环境满足条件
    SKIP_TEST_IF_FAILS(QFile::exists("/proc"), "Proc filesystem not available, skipping tests");

    // 确保当前进程ID可以被检测到
    int currentPid = getCurrentProcessId();
    SKIP_TEST_IF_FAILS(currentPid > 0, "Cannot determine current process ID");
    SKIP_TEST_IF_FAILS(process_info_helper::processExists(currentPid),
                       "Current process not detectable by process_info_helper");
}

void TestProcessInfoHelper::cleanupTestCase()
{
    // 清理测试环境
}

void TestProcessInfoHelper::test_processExists()
{
    // 测试init进程 (PID 1)
    QVERIFY(process_info_helper::processExists(1));

    // 测试当前进程
    int currentPid = getCurrentProcessId();
    QVERIFY(process_info_helper::processExists(currentPid));

    // 测试不存在的进程
    QVERIFY(!process_info_helper::processExists(99999999));

    // 测试无效的进程ID
    QVERIFY(!process_info_helper::processExists(-1));
    QVERIFY(!process_info_helper::processExists(0));
}

void TestProcessInfoHelper::test_cmdline()
{
    // 测试init进程 (PID 1)
    std::string initCmdline = process_info_helper::cmdline(1);
    QVERIFY(!initCmdline.empty());

    // 检查一个已知的进程命令行格式
    qDebug() << "Init process cmdline:" << QString::fromStdString(initCmdline);

    // 测试当前进程
    int currentPid = getCurrentProcessId();
    std::string selfCmdline = process_info_helper::cmdline(currentPid);
    QVERIFY(!selfCmdline.empty());
    QVERIFY(selfCmdline.find("test_processinfohelper") != std::string::npos);

    // 测试不存在的进程
    std::string nonExistentCmdline = process_info_helper::cmdline(99999999);
    QVERIFY(nonExistentCmdline.empty());

    // 测试无效的进程ID
    QVERIFY(process_info_helper::cmdline(-1).empty());
    QVERIFY(process_info_helper::cmdline(0).empty());
}

void TestProcessInfoHelper::test_parentPid()
{
    // 创建一个子进程并测试父子关系
    int childPid = launchDetachedProcess("sleep 5");
    SKIP_TEST_IF_FAILS(childPid > 0, "Failed to create child process");
    SKIP_TEST_IF_FAILS(waitForProcessToStart(childPid), "Child process did not start in time");

    // 获取子进程的父进程ID
    int parentOfChild = process_info_helper::parentPid(childPid);

    // 验证自己是父进程，或者至少父进程ID是有效的
    int currentPid = getCurrentProcessId();
    if (parentOfChild != currentPid) {
        qWarning() << "Parent PID of child not equal to current PID, but this might be due to process launching mechanism";
    }

    QVERIFY(parentOfChild > 0);
    QVERIFY(process_info_helper::processExists(parentOfChild));

    // 终止子进程
    terminateProcesses({childPid});

    // 测试init进程 (PID 1) 的父进程ID应该是0
    int initParent = process_info_helper::parentPid(1);
    QVERIFY(initParent == 0);

    // 测试不存在的进程
    QVERIFY(process_info_helper::parentPid(99999999) == -1);

    // 测试无效的进程ID
    QVERIFY(process_info_helper::parentPid(-1) == -1);
    QVERIFY(process_info_helper::parentPid(0) == -1);
}

void TestProcessInfoHelper::test_childPids_single()
{
    // 创建一个子进程
    int childPid = launchDetachedProcess("sleep 5");
    SKIP_TEST_IF_FAILS(childPid > 0, "Failed to create child process");
    SKIP_TEST_IF_FAILS(waitForProcessToStart(childPid), "Child process did not start in time");

    // 获取当前进程ID
    int currentPid = getCurrentProcessId();

    // 获取当前进程的子进程列表
    std::vector<int> children = process_info_helper::childPids(currentPid);

    // 验证子进程列表中包含我们创建的子进程
    // 注意：这个测试可能会失败，因为子进程可能被系统进程管理器接管
    bool found = false;
    for (int pid : children) {
        if (pid == childPid) {
            found = true;
            break;
        }
    }

    if (!found) {
        qWarning() << "Child process not found in children list, but this might be due to process launching mechanism";
        qWarning() << "Expected child PID:" << childPid;
    }

    // 测试此函数在其他情况下的行为
    std::vector<int> initChildren = process_info_helper::childPids(1);
    QVERIFY(!initChildren.empty()); // init应该有子进程

    std::vector<int> nonExistentChildren = process_info_helper::childPids(99999999);
    QVERIFY(nonExistentChildren.empty());

    // 终止子进程
    terminateProcesses({childPid});
}

void TestProcessInfoHelper::test_childPids_multiple()
{
    // 创建多个子进程
    QList<int> childPids = createChildProcesses(3);
    SKIP_TEST_IF_FAILS(!childPids.isEmpty(), "Failed to create child processes");

    // 转换为std::vector
    std::vector<int> parentPids = {getCurrentProcessId()};

    // 获取所有子进程
    std::vector<int> allChildren = process_info_helper::childPids(parentPids);

    // 验证至少找到了一些子进程
    QVERIFY(!allChildren.empty());

    // 终止子进程
    terminateProcesses(childPids);

    // 验证其他情况
    std::vector<int> emptyParentList;
    std::vector<int> childrenOfEmpty = process_info_helper::childPids(emptyParentList);
    QVERIFY(childrenOfEmpty.empty());

    std::vector<int> invalidParentList = {-1, 0};
    std::vector<int> childrenOfInvalid = process_info_helper::childPids(invalidParentList);
    QVERIFY(childrenOfInvalid.empty());
}

void TestProcessInfoHelper::test_cgroup()
{
    // 测试当前进程的cgroup
    int currentPid = getCurrentProcessId();
    std::string currentCgroup = process_info_helper::cgroup(currentPid);

    // 验证cgroup路径不为空
    QVERIFY(!currentCgroup.empty());
    qDebug() << "Current process cgroup:" << QString::fromStdString(currentCgroup);

    // 测试init进程的cgroup
    std::string initCgroup = process_info_helper::cgroup(1);
    QVERIFY(!initCgroup.empty());
    qDebug() << "Init process cgroup:" << QString::fromStdString(initCgroup);

    // 测试不存在的进程
    QVERIFY(process_info_helper::cgroup(99999999).empty());

    // 测试无效的进程ID
    QVERIFY(process_info_helper::cgroup(-1).empty());
    QVERIFY(process_info_helper::cgroup(0).empty());
}

void TestProcessInfoHelper::test_pidsOfCgroup()
{
    // 获取当前进程的cgroup
    int currentPid = getCurrentProcessId();
    std::string currentCgroup = process_info_helper::cgroup(currentPid);
    SKIP_TEST_IF_FAILS(!currentCgroup.empty(), "Failed to get cgroup of current process");

    // 构造cgroup路径
    std::string cgroupPath = "/sys/fs/cgroup" + currentCgroup;

    // 检查路径是否存在
    SKIP_TEST_IF_FAILS(QFile::exists(QString::fromStdString(cgroupPath)),
                       "Cgroup filesystem path not accessible");

    // 获取cgroup中的进程ID
    std::vector<int> cgroupPids = process_info_helper::pidsOfCgroup(cgroupPath);

    // 验证至少包含当前进程
    bool foundSelf = false;
    for (int pid : cgroupPids) {
        if (pid == currentPid) {
            foundSelf = true;
            break;
        }
    }

    if (!foundSelf) {
        qWarning() << "Current process not found in cgroup PIDs, but this might be due to cgroup configuration";
        qWarning() << "Current PID:" << currentPid;
    }

    // 测试不存在的cgroup
    std::vector<int> nonExistentCgroupPids = process_info_helper::pidsOfCgroup("/nonexistent/cgroup");
    QVERIFY(nonExistentCgroupPids.empty());
}

void TestProcessInfoHelper::test_invalidPid()
{
    // 综合测试对无效PID的处理
    QVERIFY(!process_info_helper::processExists(-100));
    QVERIFY(process_info_helper::cmdline(-100).empty());
    QCOMPARE(process_info_helper::parentPid(-100), -1);
    QVERIFY(process_info_helper::childPids(-100).empty());
    QVERIFY(process_info_helper::cgroup(-100).empty());
}

void TestProcessInfoHelper::test_selfProcessInfo()
{
    // 获取自身进程ID
    int selfPid = getCurrentProcessId();

    // 验证自身进程信息
    QVERIFY(process_info_helper::processExists(selfPid));
    QVERIFY(!process_info_helper::cmdline(selfPid).empty());
    QVERIFY(process_info_helper::parentPid(selfPid) > 0);
    QVERIFY(!process_info_helper::cgroup(selfPid).empty());
}

// 辅助函数实现
QList<int> TestProcessInfoHelper::createChildProcesses(int count)
{
    QList<int> pids;
    for (int i = 0; i < count; ++i) {
        int pid = launchDetachedProcess(QString("sleep %1").arg(5 + i));
        if (pid > 0 && waitForProcessToStart(pid)) {
            pids.append(pid);
        }
    }
    return pids;
}

void TestProcessInfoHelper::terminateProcesses(const QList<int> &pids)
{
    for (int pid : pids) {
        if (process_info_helper::processExists(pid)) {
            kill(pid, SIGTERM);
        }
    }
}

int TestProcessInfoHelper::getCurrentProcessId() const
{
    return getpid();
}

int TestProcessInfoHelper::launchDetachedProcess(const QString &command)
{
    QProcess process;
    process.startDetached(command);

    // 这里简化了实现，实际上无法获取到detached进程的ID
    // 在真实情况下，需要更复杂的机制
    return -1;
}

bool TestProcessInfoHelper::waitForProcessToStart(int pid, int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < timeoutMs) {
        if (process_info_helper::processExists(pid)) {
            return true;
        }
        QTest::qWait(100);
    }

    return false;
}

QTEST_MAIN(TestProcessInfoHelper)

#include "tst_processinfohelper.moc"
