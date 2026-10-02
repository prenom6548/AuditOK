#ifndef PENDING_APP_INFO_QUEUE_TEST_H
#define PENDING_APP_INFO_QUEUE_TEST_H

#include <QObject>

class PendingAppInfoQueueTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSingletonAndTimerSignals();
    void testEnqueueMergesUpdateFlagsForSameDesktopFile();
    void testDeleteReplacesEarlierOperations();
    void testDeleteThenNonInsertKeepsOnlyDelete();
    void testRefreshDatabaseDuplicateRules();
    void testInsertAfterDeleteKeepsBothOperations();
    void testPriorityOperationsKeepMostCompleteRefresh();
    void testSymLinkOperationsAreNotMerged();
    void testMaxTimerAlreadyActiveDoesNotEmitMaxStartAgain();
    void testProcessCacheStopsTimersAndClearsPendingItems();
    void testProcessCacheNoopsWhenQueueIsEmpty();
    void testProcessCacheIgnoresCombinedPriorityHandleTypeButStillFinishes();
    void testProcessCacheDispatchesSingleOperationTypes();
    void testProcessCacheDispatchesMergedFlagOperations();
    void testProcessCacheDispatchesMergedPriorityAndValueFlags();
    void testProcessCacheDispatchesSparseMergedFlags();
    void testSetValueOperationsAreMerged();
    void cleanupTestCase();
};

#endif // PENDING_APP_INFO_QUEUE_TEST_H
