#ifndef TEST_SYSTEMBACKUPPROXY_H
#define TEST_SYSTEMBACKUPPROXY_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/systembackupproxy.h"

struct BackupWrapper;

class TestSystemBackupProxy : public QObject
{
    Q_OBJECT

public:
    TestSystemBackupProxy() : m_proxy(nullptr) {}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();
    void testCheckEnvEx_localOrUdisk_emitsCheckResult();
    void testDoWorkEx();
    void testCancelEx_emitsStartCancelAndCancelSuccess();

    void testCheckFreeCapacity_capacityEnough_emitsCheckEnvSuccess();
    void testCheckFreeCapacity_capacityNotEnough_emitsCapacityNotEnough();
    void testGetRsyncArgs_allScenes_returnsNonEmpty();
    void testRecordBackupPoint_fakeData_success();
    void testDeleteFailedData_fakeXml_removesItem();
    void testExtraDataPaths_returnsList();
    void testCalcSizeForExtraBackup_returnsInt();
    void testCpFile_fakeFile_createsBak();
    void testRemoveAutoBackupPoint_fakeXml_removesItem();

    void testLoadExistsBackupPoint_fakeXml_loadsBackupPoint();
    void testLoadExistsBackupPoint_autoUuid_updatesOrLoadsFromSys();
    void testCalcSizeForBackup_invoke_noCrash();
    void testCalcSizeForHome_returnsQint64();
    void testDoMksqushfs_invoke_noCrash();
    void testBackupSystem_invoke_noCrash();
    void testExtraDataBackup_fakePaths_runsNoCrash();

private:
    void setBackupWrapper(BackupWrapper &w, int iPosition, const QString &prefixPath,
                         const QString &uuid = QString(), const QString &backupName = QStringLiteral("test-sys"));
    bool writeFileSaveSize(const QString &basePath, qint64 totalSize, qint64 leftSize);
    bool writeSysBackupListXml(const QString &basePath, const QString &uuid);
    bool writeBackupListXml(const QString &basePath, const QString &uuid);

    SystemBackupProxy *m_proxy;
};

#endif // TEST_SYSTEMBACKUPPROXY_H
