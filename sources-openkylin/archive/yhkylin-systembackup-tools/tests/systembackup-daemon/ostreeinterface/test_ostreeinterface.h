#ifndef TEST_OSTREEINTERFACE_H
#define TEST_OSTREEINTERFACE_H

#include <QObject>
#include <QtTest/QtTest>

class OstreeInterface;

class TestOstreeInterface : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testGetRepoPath_emptyBackupPath();
    void testGetRepoPath_allBackupTypes();
    void testGetRepoPath_withQuotes();
    void testCheckOstreeRepo_existAndNotExist();
    void testCheckOstreeRepo_withQuotes();
    void testInitOstreeRepo();
    void testGetSystemHashArgs();
    void testGetPendingHashArgs();
    void testGetDeployArgs();
    void testGetUndeployArgs();
    void testGetNeedUndeployHashArgs();
    void testGetHashNumberArgs();
    void testGetSystemRollBackArgs();
    void testGetCommitArgs();
    void testGetCheckoutArgs();
    void testGetCheckoutFullSyncArgs();
    void testGetRefsArgs();
    void testGetPruneArgs();
    void testGetRsyncRepoArgs_allScenes();
    void testGetCpArgs();
    void testGetRmArgs();
    void testGetFindArgs_allTypes();
    void testSetGetOperationPath();
    void testSetGetBackUpStep();
    void testSetGetOperation();
    void testSetGetTaskType();
    void testSetGetRsyncScense();
    void testStop();
};

#endif // TEST_OSTREEINTERFACE_H
