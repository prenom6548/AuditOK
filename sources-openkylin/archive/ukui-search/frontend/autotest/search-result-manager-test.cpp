#include "search-result-manager-test.h"

#include <QSignalSpy>
#include <QTest>

#define private public
#include "search-result-manager.h"
#undef private

using namespace UkuiSearch;
Q_DECLARE_METATYPE(UkuiSearch::SearchPluginIface::ResultInfo)

namespace {
SearchPluginIface::ResultInfo makeInfo(const QString &name)
{
    SearchPluginIface::ResultInfo info;
    info.name = name;
    return info;
}
}

void SearchResultManagerTest::testReceiveResultThreadEmitsQueuedResults()
{
    qRegisterMetaType<SearchPluginIface::ResultInfo>("SearchPluginIface::ResultInfo");

    DataQueue<SearchPluginIface::ResultInfo> queue;
    queue.enqueue(makeInfo(QStringLiteral("first")));
    queue.enqueue(makeInfo(QStringLiteral("second")));

    ReceiveResultThread thread(&queue);
    QSignalSpy spy(&thread, SIGNAL(gotResultInfo(SearchPluginIface::ResultInfo)));
    QVERIFY(spy.isValid());

    thread.startConsume();
    QVERIFY(spy.wait(500));
    QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 2, 500);
    thread.stop();

    QCOMPARE(spy.at(0).at(0).value<SearchPluginIface::ResultInfo>().name, QStringLiteral("first"));
    QCOMPARE(spy.at(1).at(0).value<SearchPluginIface::ResultInfo>().name, QStringLiteral("second"));
    QVERIFY(queue.isEmpty());
}

void SearchResultManagerTest::testReceiveResultThreadStopClearsQueue()
{
    DataQueue<SearchPluginIface::ResultInfo> queue;
    queue.enqueue(makeInfo(QStringLiteral("leftover")));

    ReceiveResultThread thread(&queue);
    thread.stop();

    QVERIFY(queue.isEmpty());
}

void SearchResultManagerTest::testReceiveResultThreadStartStopWithEmptyQueue()
{
    DataQueue<SearchPluginIface::ResultInfo> queue;
    ReceiveResultThread thread(&queue);
    QSignalSpy spy(&thread, SIGNAL(gotResultInfo(SearchPluginIface::ResultInfo)));
    QVERIFY(spy.isValid());

    thread.startConsume();
    QTest::qWait(80);
    thread.stop();

    QCOMPARE(spy.count(), 0);
    QVERIFY(queue.isEmpty());
}

void SearchResultManagerTest::testReceiveResultThreadProcessesAtMostOneBatch()
{
    qRegisterMetaType<SearchPluginIface::ResultInfo>("SearchPluginIface::ResultInfo");

    DataQueue<SearchPluginIface::ResultInfo> queue;
    for (int i = 0; i < 105; ++i) {
        queue.enqueue(makeInfo(QStringLiteral("item-%1").arg(i)));
    }

    ReceiveResultThread thread(&queue);
    QSignalSpy spy(&thread, SIGNAL(gotResultInfo(SearchPluginIface::ResultInfo)));
    QVERIFY(spy.isValid());

    thread.start();
    QVERIFY(QMetaObject::invokeMethod(thread.m_timeoutTimer, [&thread] {
        thread.processQueue();
    }, Qt::BlockingQueuedConnection));
    thread.quit();
    thread.wait();

    QCOMPARE(spy.count(), 100);
    QCOMPARE(spy.first().at(0).value<SearchPluginIface::ResultInfo>().name, QStringLiteral("item-0"));
    QCOMPARE(spy.last().at(0).value<SearchPluginIface::ResultInfo>().name, QStringLiteral("item-99"));
    QCOMPARE(queue.tryDequeue().name, QStringLiteral("item-100"));
}

void SearchResultManagerTest::testReceiveResultThreadTimeoutClearsQueue()
{
    DataQueue<SearchPluginIface::ResultInfo> queue;
    queue.enqueue(makeInfo(QStringLiteral("timeout-leftover")));

    ReceiveResultThread thread(&queue);
    thread.start();
    QVERIFY(QMetaObject::invokeMethod(thread.m_timeoutTimer, [&thread] {
        thread.m_timeoutTimer->setInterval(1);
        thread.m_timeoutTimer->start();
    }, Qt::BlockingQueuedConnection));
    QTRY_VERIFY_WITH_TIMEOUT(queue.isEmpty(), 500);
    thread.quit();
    thread.wait();

    QVERIFY(queue.isEmpty());
}

void SearchResultManagerTest::testReceiveResultThreadStartConsumeWhileAlreadyRunning()
{
    DataQueue<SearchPluginIface::ResultInfo> queue;
    ReceiveResultThread thread(&queue);
    QSignalSpy timerStartSpy(&thread, SIGNAL(timerStart()));
    QVERIFY(timerStartSpy.isValid());

    thread.start();
    QTRY_VERIFY(thread.isRunning());
    thread.startConsume();
    QCOMPARE(timerStartSpy.count(), 1);
    thread.stop();
    thread.quit();
    thread.wait();
}

void SearchResultManagerTest::testSearchResultManagerForwardsThreadResults()
{
    qRegisterMetaType<SearchPluginIface::ResultInfo>("SearchPluginIface::ResultInfo");

    SearchResultManager manager(QStringLiteral("unit-test-plugin"));
    QSignalSpy spy(&manager, SIGNAL(gotResultInfo(SearchPluginIface::ResultInfo)));
    QVERIFY(spy.isValid());

    Q_EMIT manager.m_getResultThread->gotResultInfo(makeInfo(QStringLiteral("forwarded")));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).value<SearchPluginIface::ResultInfo>().name, QStringLiteral("forwarded"));
}

QTEST_MAIN(SearchResultManagerTest)
