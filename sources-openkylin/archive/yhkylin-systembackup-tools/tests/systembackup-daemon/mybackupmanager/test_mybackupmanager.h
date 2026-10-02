#ifndef TEST_MYBACKUPMANAGER_H
#define TEST_MYBACKUPMANAGER_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/mybackupmanager.h"

/** 跳过（还原、重启、占屏接口）：
 * MyBackupManager::autoRestoreForSystemUpdate
 * MyBackupManager::restoreSystemByUser
 * MyBackupManager::reboot
 * MyBackupManager::plymountMode
 * MyBackupManager::plymountMessage
 */

class TestMyBackupManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();
    void testMountBackupPartition();
    void testUmountBackupPartition();
    void testRemountPartitionsForCasperMode();
    void testCheckEnv();
    void testCheckEnv_workerNull_whenInvalidType();
    void testGoBackup();
    void testGoRestore();
    void testDeleteBackupPoint();
    void testGhostBackup();
    void testAutoBackUpForSystemUpdate();
    void testAutoBackUpList();
    // void testAutoRestoreForSystemUpdate();
    void testBackupSystemByUser_toLocal();
    void testBackupSystemByUser_toUdisk();
    void testSystemBackUpList();
    void testBackupDataByUser();
    void testRestoreDataByUser();
    void testGetBackupCommentForSystemUpdate();
    void testGetBackupState();
    void testCancel();
    void testFinished();
    void testExistsSystemBackupNode();
    void testSetLocale();
    void testWakeUpDaemon();
    void testGetProcessName();
    void testUmountBackup_branchNeedUmount();
    void testDoAfterBothSignals_invoke_noCrash();

private:
    MyBackupManager* m_manager;
};

#endif // TEST_MYBACKUPMANAGER_H
