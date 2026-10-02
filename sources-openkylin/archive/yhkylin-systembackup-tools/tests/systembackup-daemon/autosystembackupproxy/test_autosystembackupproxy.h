#ifndef TEST_AUTOSYSTEMBACKUPPROXY_H
#define TEST_AUTOSYSTEMBACKUPPROXY_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/autosystembackupproxy.h"

struct BackupWrapper;

class TestAutoSystemBackupProxy : public QObject
{
    Q_OBJECT

public:
    TestAutoSystemBackupProxy() : m_proxy(nullptr) {}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testLoadExistsBackupPoint();
    void testDestructor();
    void testCheckEnvEx_emitsCheckResult();
    void testCheckEnvEx_withAutoBackupUuid();
    void testDoWorkEx();
    void testDoWorkEx_waitForAsyncChain();
    void testCancelEx_emitsStartCancelAndCancelSuccess();
    void testCheckFreeCapacity_emitsCheckResult();
    void testCheckFreeCapacity_capacityNotEnough_emitsWorkResultFalse();
    void testBackupSystem_runsWithoutCrash();

    void testDoPrepare();
    void testCheckAndRemoveBackupPoint();
    void testRecordBackupPoint();
    void testRemoveAutoBackupPoint();
    void testExtraDataPaths();
    void testExtraDataBackup();
    void testCalcSizeForExtraBackup();
    void testDeleteBackupPointCommits();
    void testDoMksqushfs();
    void testGetRsyncArgs_allScenes();
    void testCalcSizeForHome();

private:
    void setAutoSystemBackupWrapper(BackupWrapper &w, const QString &prefixPath, const QString &uuid);

    AutoSystemBackupProxy *m_proxy;
};

#endif // TEST_AUTOSYSTEMBACKUPPROXY_H
