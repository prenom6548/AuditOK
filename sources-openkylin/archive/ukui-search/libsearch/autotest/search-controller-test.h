#ifndef SEARCH_CONTROLLER_TEST_H
#define SEARCH_CONTROLLER_TEST_H

#include <QObject>

class SearchControllerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaultsAndConditionSetters();
    void testClearMethods();
    void testSearchIdAndBeginFinishCheck();
    void testDataQueueInitRefreshAndCopySharing();
    void testRefreshDataQueueCreatesQueueWhenMissing();
    void testMoveAssignmentTransfersSharedState();
    void testResultPropertyMaps();
    void testResultLimits();
};

#endif // SEARCH_CONTROLLER_TEST_H
