#ifndef SEARCH_RESULT_MANAGER_TEST_H
#define SEARCH_RESULT_MANAGER_TEST_H

#include <QObject>

class SearchResultManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testReceiveResultThreadEmitsQueuedResults();
    void testReceiveResultThreadStopClearsQueue();
    void testReceiveResultThreadStartStopWithEmptyQueue();
    void testReceiveResultThreadProcessesAtMostOneBatch();
    void testReceiveResultThreadTimeoutClearsQueue();
    void testReceiveResultThreadStartConsumeWhileAlreadyRunning();
    void testSearchResultManagerForwardsThreadResults();
};

#endif // SEARCH_RESULT_MANAGER_TEST_H
