#include "data-queue-test.h"

#include <QTest>

#include "data-queue.h"

using namespace UkuiSearch;

void DataQueueTest::testEnqueueDequeueAndIsEmpty()
{
    DataQueue<int> queue;
    QVERIFY(queue.isEmpty());

    queue.enqueue(1);
    queue.enqueue(2);

    QVERIFY(!queue.isEmpty());
    QCOMPARE(queue.dequeue(), 1);
    QCOMPARE(queue.tryDequeue(), 2);
    QVERIFY(queue.isEmpty());
}

void DataQueueTest::testTryDequeueReturnsDefaultWhenEmpty()
{
    DataQueue<QString> queue;

    QVERIFY(queue.tryDequeue().isEmpty());
    QVERIFY(queue.isEmpty());
}

void DataQueueTest::testClearRemovesAllItems()
{
    DataQueue<int> queue;
    queue.enqueue(10);
    queue.enqueue(20);

    queue.clear();

    QVERIFY(queue.isEmpty());
    QCOMPARE(queue.tryDequeue(), 0);
}

QTEST_MAIN(DataQueueTest)
