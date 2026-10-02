#ifndef PENDING_FILE_TEST_H
#define PENDING_FILE_TEST_H

#include <QObject>

class PendingFileTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaultStateAndSetters();
    void testMergeKeepsLatestDeleteAndFlags();
    void testTrailingSlashAndBooleanMergeBranches();
    void testEqualityUsesPathOnly();
};

#endif // PENDING_FILE_TEST_H
