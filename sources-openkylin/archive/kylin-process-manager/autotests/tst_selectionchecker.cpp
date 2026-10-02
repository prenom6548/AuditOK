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

#define private public
#define protected public
#include "core/selection/selectionchecker.h"
#undef private
#undef protected

// 定义测试类的自定义测试版本以注入mock行为
class TestableSelectionChecker : public SelectionChecker
{
public:
    TestableSelectionChecker()
        : SelectionChecker()
        , m_mockPlatformIsWayland(false)
        , m_mockSelectionPids({})
    {
    }

    // 设置平台类型模拟
    void setMockPlatformIsWayland(bool isWayland)
    {
        m_mockPlatformIsWayland = isWayland;
        m_isWaylandPlatform = isWayland;
    }

    // 设置模拟的选择器进程ID
    void setMockSelectionPids(const std::vector<int> &pids)
    {
        m_mockSelectionPids = pids;
    }

    // 重写方法以返回预设的模拟数据
    void checkPlatform() override
    {
        m_isWaylandPlatform = m_mockPlatformIsWayland;
    }

    std::vector<int> getX11SelectionProcesses() const override
    {
        m_methodCalls.push_back("getX11SelectionProcesses");
        return m_mockSelectionPids;
    }

    std::vector<unsigned long> queryX11SelectionOwners() const override
    {
        m_methodCalls.push_back("queryX11SelectionOwners");

        // 为了测试而返回一些模拟窗口ID
        std::vector<unsigned long> owners;
        for (size_t i = 0; i < m_mockSelectionPids.size(); ++i) {
            owners.push_back(1000 + i); // 模拟窗口ID
        }
        return owners;
    }

    std::vector<int> getWaylandSelectionProcesses() const override
    {
        m_methodCalls.push_back("getWaylandSelectionProcesses");
        return m_mockSelectionPids;
    }

    int getSelectionPidFromKwin(const std::string &method) const override
    {
        m_methodCalls.push_back(method);

        if (m_mockSelectionPids.empty()) {
            return 0;
        }

        // 根据方法名称返回不同的PID
        if (method == "getClipboardSelectionPid" && m_mockSelectionPids.size() > 0) {
            return m_mockSelectionPids[0];
        } else if (method == "getPrimarySelectionPid" && m_mockSelectionPids.size() > 1) {
            return m_mockSelectionPids[1];
        }

        return 0;
    }

    // 检查哪些方法被调用了
    bool wasMethodCalled(const std::string &methodName) const
    {
        return std::find(m_methodCalls.begin(), m_methodCalls.end(), methodName) != m_methodCalls.end();
    }

    // 清除方法调用历史
    void clearMethodCalls()
    {
        m_methodCalls.clear();
    }

private:
    bool m_mockPlatformIsWayland;
    std::vector<int> m_mockSelectionPids;
    mutable std::vector<std::string> m_methodCalls;
};

class TestSelectionChecker : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试用例
    void testNoSelection();
    void testHasSelectionX11();
    void testHasSelectionWayland();
    void testPlatformDetection();
    void testMultiplePids();
    void testEmptyPids();
    void testMethodCalls();

private:
    TestableSelectionChecker *m_selectionChecker;
};

void TestSelectionChecker::initTestCase()
{
    qDebug() << "Starting SelectionChecker tests";
}

void TestSelectionChecker::cleanupTestCase()
{
    qDebug() << "SelectionChecker tests completed";
}

void TestSelectionChecker::init()
{
    m_selectionChecker = new TestableSelectionChecker();
}

void TestSelectionChecker::cleanup()
{
    delete m_selectionChecker;
    m_selectionChecker = nullptr;
}

void TestSelectionChecker::testNoSelection()
{
    // 测试无选择时的行为
    m_selectionChecker->setMockSelectionPids({});

    // 检查没有任何进程时的返回值
    QVERIFY(!m_selectionChecker->hasSelection({1234, 5678}));
}

void TestSelectionChecker::testHasSelectionX11()
{
    // 测试X11平台下有选择的情况
    m_selectionChecker->setMockPlatformIsWayland(false);
    m_selectionChecker->setMockSelectionPids({5678, 9012});

    // 检查有进程时的返回值
    QVERIFY(m_selectionChecker->hasSelection({1234, 5678}));
    QVERIFY(!m_selectionChecker->hasSelection({1234, 4321}));

    // 验证调用了X11相关方法
    QVERIFY(m_selectionChecker->wasMethodCalled("getX11SelectionProcesses"));
    QVERIFY(!m_selectionChecker->wasMethodCalled("getWaylandSelectionProcesses"));
}

void TestSelectionChecker::testHasSelectionWayland()
{
    // 测试Wayland平台下有选择的情况
    m_selectionChecker->setMockPlatformIsWayland(true);
    m_selectionChecker->setMockSelectionPids({5678, 9012});

    // 检查有进程时的返回值
    QVERIFY(m_selectionChecker->hasSelection({1234, 5678}));
    QVERIFY(!m_selectionChecker->hasSelection({1234, 4321}));

    // 验证调用了Wayland相关方法
    QVERIFY(m_selectionChecker->wasMethodCalled("getWaylandSelectionProcesses"));
    QVERIFY(!m_selectionChecker->wasMethodCalled("getX11SelectionProcesses"));
}

void TestSelectionChecker::testPlatformDetection()
{
    // 测试平台检测
    m_selectionChecker->setMockPlatformIsWayland(true);
    QVERIFY(m_selectionChecker->m_isWaylandPlatform);

    m_selectionChecker->setMockPlatformIsWayland(false);
    QVERIFY(!m_selectionChecker->m_isWaylandPlatform);
}

void TestSelectionChecker::testMultiplePids()
{
    // 测试多个进程ID的情况
    m_selectionChecker->setMockSelectionPids({1001, 1002, 1003, 1004});

    // 不在列表中的进程
    QVERIFY(!m_selectionChecker->hasSelection({2001, 2002}));

    // 部分在列表中的进程
    QVERIFY(m_selectionChecker->hasSelection({2001, 1003}));

    // 全部在列表中的进程
    QVERIFY(m_selectionChecker->hasSelection({1001, 1002}));
}

void TestSelectionChecker::testEmptyPids()
{
    // 测试空进程ID列表的情况
    m_selectionChecker->setMockSelectionPids({1001, 1002});

    // 空的进程列表应该返回false
    QVERIFY(!m_selectionChecker->hasSelection({}));
}

void TestSelectionChecker::testMethodCalls()
{
    // 测试在Wayland平台上的方法调用
    m_selectionChecker->setMockPlatformIsWayland(true);
    m_selectionChecker->setMockSelectionPids({1001, 1002});
    m_selectionChecker->clearMethodCalls();

    m_selectionChecker->hasSelection({1001});

    // 验证调用了正确的方法
    QVERIFY(m_selectionChecker->wasMethodCalled("getWaylandSelectionProcesses"));
    QVERIFY(m_selectionChecker->wasMethodCalled("getClipboardSelectionPid"));
    QVERIFY(m_selectionChecker->wasMethodCalled("getPrimarySelectionPid"));

    // 测试在X11平台上的方法调用
    m_selectionChecker->setMockPlatformIsWayland(false);
    m_selectionChecker->clearMethodCalls();

    m_selectionChecker->hasSelection({1001});

    // 验证调用了正确的方法
    QVERIFY(m_selectionChecker->wasMethodCalled("getX11SelectionProcesses"));
    QVERIFY(m_selectionChecker->wasMethodCalled("queryX11SelectionOwners"));
    QVERIFY(!m_selectionChecker->wasMethodCalled("getClipboardSelectionPid"));
}

QTEST_MAIN(TestSelectionChecker)
#include "tst_selectionchecker.moc"