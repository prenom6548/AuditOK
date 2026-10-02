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

#include <QCoreApplication>
#include <QSignalSpy>
#include <QtTest>

#include "../daemon/src/resourcewatcher.h"

// 创建一个ResourceWatcher的派生类用于测试
class MockResourceWatcher : public ResourceWatcher
{
    Q_OBJECT
public:
    explicit MockResourceWatcher(QObject *parent = nullptr)
        : ResourceWatcher(parent)
        , m_isRunning(false)
    {
    }

    bool isRunning() const
    {
        return m_isRunning;
    }

public Q_SLOTS:
    void start() override
    {
        m_isRunning = true;

        // 发出一个资源警告信号
        emit ResourceThresholdWarning("MockResource", 1);
    }

    void stop() override
    {
        m_isRunning = false;
        emit finished();
    }

private:
    bool m_isRunning;
};

class TestResourceWatcher : public QObject
{
    Q_OBJECT

public:
    TestResourceWatcher();
    ~TestResourceWatcher();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试基本构造函数
    void testConstructor();

    // 测试开始和停止监视
    void testStartAndStop();

    // 测试信号连接和发送
    void testSignals();

private:
    MockResourceWatcher *m_watcher;
};

TestResourceWatcher::TestResourceWatcher()
{
}

TestResourceWatcher::~TestResourceWatcher()
{
}

void TestResourceWatcher::initTestCase()
{
}

void TestResourceWatcher::cleanupTestCase()
{
}

void TestResourceWatcher::init()
{
    m_watcher = new MockResourceWatcher();
}

void TestResourceWatcher::cleanup()
{
    delete m_watcher;
    m_watcher = nullptr;
}

void TestResourceWatcher::testConstructor()
{
    // 验证构造函数是否正确初始化了对象
    QVERIFY(m_watcher != nullptr);

    // 验证初始状态
    QVERIFY(!m_watcher->isRunning());
}

void TestResourceWatcher::testStartAndStop()
{
    // 验证开始监视
    m_watcher->start();
    QVERIFY(m_watcher->isRunning());

    // 验证停止监视
    m_watcher->stop();
    QVERIFY(!m_watcher->isRunning());
}

void TestResourceWatcher::testSignals()
{
    // 验证ResourceThresholdWarning信号
    QSignalSpy warningSignalSpy(m_watcher, &ResourceWatcher::ResourceThresholdWarning);
    m_watcher->start();

    // 应该发出一个警告信号
    QCOMPARE(warningSignalSpy.count(), 1);

    // 验证信号参数
    QList<QVariant> warningArguments = warningSignalSpy.takeFirst();
    QCOMPARE(warningArguments.at(0).toString(), QString("MockResource"));
    QCOMPARE(warningArguments.at(1).toInt(), 1);

    // 验证finished信号
    QSignalSpy finishedSignalSpy(m_watcher, &ResourceWatcher::finished);
    m_watcher->stop();

    // 应该发出一个finished信号
    QCOMPARE(finishedSignalSpy.count(), 1);
}

QTEST_MAIN(TestResourceWatcher)
#include "tst_resourcewatcher.moc"