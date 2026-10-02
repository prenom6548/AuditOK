#ifndef BEST_LIST_MODEL_TEST_H
#define BEST_LIST_MODEL_TEST_H

#include <QObject>

class BestListModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDataAndSetData();
    void testAdditionalRolesAndCounts();
    void testExpandedAndCollapsedRowCount();
    void testMoveInfoReordersItems();
    void testAppendInfoIgnoredWhenNotBestMatch();
    void testAppendInfoSkipsDuplicateResourceAndAction();
    void testAppendInfoReplacesExistingPluginResult();
    void testRemoveInfoAndStopSearchSlot();
    void testInvalidIndexAndParentBehaviour();
};

#endif // BEST_LIST_MODEL_TEST_H
