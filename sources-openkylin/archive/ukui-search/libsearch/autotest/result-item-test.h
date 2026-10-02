#ifndef RESULT_ITEM_TEST_H
#define RESULT_ITEM_TEST_H

#include <QObject>

class ResultItemTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaultAndSetterGetterPaths();
    void testConstructorsAndStoredMap();
    void testCopyAssignmentAndMoveAssignment();
    void testCopyAssignmentOverwritesExistingData();
};

#endif // RESULT_ITEM_TEST_H
