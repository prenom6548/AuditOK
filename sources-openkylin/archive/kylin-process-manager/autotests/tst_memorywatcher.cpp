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
#include <QFile>
#include <QSignalSpy>
#include <QtTest>

// 为了能够访问私有方法
#define private public
#define protected public
#include "../daemon/src/memorywatcher.h"
#undef private
#undef protected

class TestMemoryWatcher : public QObject
{
    Q_OBJECT

public:
    TestMemoryWatcher();
    ~TestMemoryWatcher();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();

    void testCheckMemoryRemaining();

    void testStartAndStop();

    void testResourceWarningSignal();

private:
    MemoryWatcher *m_watcher;
    std::map<resource::Resource, resource::ResourceUrgencyThreshold> createTestResourceThresholds();
};

TestMemoryWatcher::TestMemoryWatcher()
{
}

TestMemoryWatcher::~TestMemoryWatcher()
{
}

void TestMemoryWatcher::initTestCase()
{
}

void TestMemoryWatcher::cleanupTestCase()
{
}

void TestMemoryWatcher::init()
{
    m_watcher = new MemoryWatcher(createTestResourceThresholds(), 1024, 700);
}

void TestMemoryWatcher::cleanup()
{
    delete m_watcher;
    m_watcher = nullptr;
}

void TestMemoryWatcher::testConstructor()
{
    QVERIFY(m_watcher != nullptr);
    QVERIFY(m_watcher->m_timer != nullptr);
    QCOMPARE(m_watcher->m_meminfoGateSwapFreeMb, 1024);
    QCOMPARE(m_watcher->m_meminfoGateReclaimHeadroomMb, 700);
}

void TestMemoryWatcher::testCheckMemoryRemaining()
{
    QSignalSpy spy(m_watcher, &MemoryWatcher::ResourceThresholdWarning);
    m_watcher->checkMemoryRemaining();
    for (int i = 0; i < spy.count(); ++i) {
        QList<QVariant> arguments = spy.at(i);
        QCOMPARE(arguments.at(0).toString(), QString("Memory"));
        QCOMPARE(arguments.at(1).toInt(), 3);
    }
}

void TestMemoryWatcher::testStartAndStop()
{
    QVERIFY(!m_watcher->m_timer->isActive());

    if (QFile::exists(QStringLiteral("/proc/pressure/memory"))) {
        QSKIP("Memory PSI mode blocks in start() until stop(); timer start/stop is not used.");
    }

    m_watcher->start();
    QVERIFY(m_watcher->m_timer->isActive());
    m_watcher->stop();
    QVERIFY(!m_watcher->m_timer->isActive());
}

void TestMemoryWatcher::testResourceWarningSignal()
{
    if (QFile::exists(QStringLiteral("/proc/pressure/memory"))) {
        QSKIP("Memory PSI mode uses epoll instead of QTimer; signal timing differs from polling test.");
    }

    QSignalSpy spy(m_watcher, &MemoryWatcher::ResourceThresholdWarning);
    m_watcher->start();
    QTest::qWait(2500);
    m_watcher->stop();

    for (int i = 0; i < spy.count(); ++i) {
        QCOMPARE(spy.at(i).at(0).toString(), QString("Memory"));
        QCOMPARE(spy.at(i).at(1).toInt(), 3);
    }
}

std::map<resource::Resource, resource::ResourceUrgencyThreshold> TestMemoryWatcher::createTestResourceThresholds()
{
    std::map<resource::Resource, resource::ResourceUrgencyThreshold> thresholds;

    resource::ResourceUrgencyThreshold cpuThresholds;
    cpuThresholds[resource::ResourceUrgency::Low] = 20;
    cpuThresholds[resource::ResourceUrgency::Medium] = 50;
    cpuThresholds[resource::ResourceUrgency::High] = 80;
    thresholds[resource::Resource::CPU] = cpuThresholds;

    return thresholds;
}

QTEST_MAIN(TestMemoryWatcher)
#include "tst_memorywatcher.moc"
