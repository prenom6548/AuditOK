#ifndef DATA_QUEUE_TEST_H
#define DATA_QUEUE_TEST_H

#include <QObject>

class DataQueueTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testEnqueueDequeueAndIsEmpty();
    void testTryDequeueReturnsDefaultWhenEmpty();
    void testClearRemovesAllItems();
};

#endif // DATA_QUEUE_TEST_H
