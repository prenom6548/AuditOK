#ifndef TEST_CUSTOMIZEDATABACKUPPROXY_H
#define TEST_CUSTOMIZEDATABACKUPPROXY_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/customizedatabackupproxy.h"

struct BackupWrapper;

class TestCustomizeDataBackupProxy : public QObject
{
    Q_OBJECT

public:
    TestCustomizeDataBackupProxy() : m_proxy(nullptr) {}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();

    // void testCheckEnvEx_emitsCheckResult();
    // void testCheckEnvEx_pathInjection_emitsDuErr();
    // void testCheckEnvEx_emptyPrefixDestPath_mkpath();
    void testDoWorkEx();
    void testCancelEx_emitsStartCancelAndCancelSuccess();

    void testCheckFreeCapacity_invoke();
    void testCheckFreeCapacity_cancel_returnsFalse();

    void testIsIncBackup_uuidEmpty_returnsFalse();
    void testIsIncBackup_uuidSet_dirExist_returnsTrue();
    void testIsIncBackup_uuidSet_dirNotExist_returnsFalse();

    void testGetRsyncArgs_dataBackup();
    void testGetRsyncArgs_incDataBackup();
    void testGetRsyncArgs_tryDataBackup();
    void testGetRsyncArgs_tryIncDataBackup();
    void testGetRsyncArgs_default();

    void testDeleteFailedData_invoke_emptyUuid_noCrash();

private:
    void setCustomizeDataBackupWrapper(struct BackupWrapper &w, const QString &prefixPath,
                                       const QStringList &backupPaths, const QString &uuid);

    CustomizeDataBackupProxy *m_proxy;
};

#endif // TEST_CUSTOMIZEDATABACKUPPROXY_H
