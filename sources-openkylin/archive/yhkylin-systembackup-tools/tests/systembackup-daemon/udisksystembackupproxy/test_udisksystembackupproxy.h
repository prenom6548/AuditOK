#ifndef TEST_UDISKSYSTEMBACKUPPROXY_H
#define TEST_UDISKSYSTEMBACKUPPROXY_H

#include <QObject>
#include <QtTest/QtTest>

class UDiskSystemBackupProxy;

class TestUDiskSystemBackupProxy : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();
    void testCheckEnvEx();
    void testDoWorkEx();
    // void testDoWorkEx_doPrepareFails_emitsFalse();
    void testCancelEx_emitsSignals();
    void testCheckEnvEx_emitsCheckEnvSuccess();
    void testCheckFreeCapacity_invoke_emitsSignal();
    void testCheckDestDirExists_invoke();
    void testIsIncBackup_uuidEmpty_noLastPoint_returnsFalse();
    void testIsIncBackup_uuidNonEmpty_dirExist_returnsTrue();
    void testCalcSizeForBackup_invoke();
    void testGetRsyncArgs_systemBackup();
    void testGetRsyncArgs_trySystemBackup();
    void testGetRsyncArgs_mksquashfs();
    void testGetRsyncArgs_imgBackup();
    void testDoMksqushfs_invoke();
    void testDoBackup_invoke();
    void testBackupSystem_invoke();
    void testBackupImg_invoke();
    void testBackup_invoke();

private:
    UDiskSystemBackupProxy* m_proxy;
};

#endif // TEST_UDISKSYSTEMBACKUPPROXY_H
