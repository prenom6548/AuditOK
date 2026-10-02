#ifndef TEST_GHOSTIMAGEPROXY_H
#define TEST_GHOSTIMAGEPROXY_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/ghostimageproxy.h"

class TestGhostImageProxy : public QObject
{
    Q_OBJECT

public:
    TestGhostImageProxy() : m_proxy(nullptr) {}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();

    void testCheckEnvEx_tempDir_successOrCapacity();
    void testCheckEnvEx_dirNotExist_mkpathSucceeds();
    void testCheckEnvEx_udisk_iPosition1();
    void testCheckEnvEx_kyimgExists_removedBeforeCheck();
    void testCheckEnvEx_emitsCheckResult();

    void testDoWorkEx();
    void testCancelEx_emitsStartCancelAndCancelSuccess();
    void testCancelEx_whenNotFinished_callsUnmountAndDeleteFailedData();

    void testGetTotalDiskSpace_invoke();
    void testDeleteFailedData_invoke_removesKyimg();
    void testParseFstab_invoke();
    void testGetDeviceByUuid_invalidUuid_returnsEmpty();
    void testMountDevice_invalidDevice_returnsFalse();
    void testUnmountDevices_invoke();
    void testCreateTempDirs_invoke();
    void testCreateSystemInfoJson_invoke();
    void testGetKylinVersion_invoke();

private:
    void setGhostBackupWrapper(BackupWrapper &w, const QString &prefixPath,
                               int iPosition, const QString &backupName);

    GhostImageProxy *m_proxy;
};

#endif // TEST_GHOSTIMAGEPROXY_H
