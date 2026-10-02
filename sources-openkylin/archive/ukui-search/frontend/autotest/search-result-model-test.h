#ifndef SEARCH_RESULT_MODEL_TEST_H
#define SEARCH_RESULT_MODEL_TEST_H

#include <QObject>

class SearchResultModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testAppendInfoIgnoredWhenSearchIdMismatch();
    void testAppendInfoAndDataRolesForGenericPlugin();
    void testInvalidIndexParentColumnAndRefreshBranches();
    void testSetDataShowToolTipRole();
    void testNoBestListSignalForWebPagePlugin();
    void testExpandedAndCollapsedRowCount();
    void testCollapsedLargeListCachesWithoutImmediateSignals();
    void testExpandedLargeListTriggersTimerDrivenItemListChanged();
    void testStopSearchSlotClearsItemsAndEmitsStopSearch();
    void testRoleNamesContainAdditionalRoles();
};

#endif // SEARCH_RESULT_MODEL_TEST_H
