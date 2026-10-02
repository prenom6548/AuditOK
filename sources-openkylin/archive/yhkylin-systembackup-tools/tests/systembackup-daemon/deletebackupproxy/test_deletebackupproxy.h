#ifndef TEST_DELETEBACKUPPROXY_H
#define TEST_DELETEBACKUPPROXY_H

#include <QtTest>
#include <QMap>
#include <QString>
#include "../../../systembackup-daemon/deletebackupproxy.h"

class TestDeleteBackupProxy : public QObject
{
    Q_OBJECT

public:
    TestDeleteBackupProxy();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();
    void testDoWorkEx_emitFalse_whenDataCommitHashEmpty();
    void testDoWorkEx_emitFalse_whenNoOstreeRepo_user();
    void testDoWorkEx_emitFalse_whenNoOstreeRepo_system();
    void testDoWorkEx_emitFalse_whenSystemCommitHashEmpty();
    void testDoWorkEx_emitTrue_fullSuccess_userBackup();
    void testDoWorkEx_xmlPath_branch_iPosition1_backupType1();
    void testDoWorkEx_mobileBackupPoint_countOneAndTwo();
    void testDoWorkEx_emitFalse_whenDeleteItemFails();
    void testDoWorkEx_uuidFormatError_hitsDebugPath();
    void testDoWorkEx_userBackup_iPosition0_prefixDestPath();
    void testDoWorkEx_systemBackup_iPosition0_xmlPathFromRoot();

private:
    bool writeUserBackupXml(const QString& xmlPath, const QString& uuid,
                            const QString& branchName, bool withDataCommitHashMap);
    bool writeSystemBackupXml(const QString& xmlPath, const QString& uuid,
                              bool withSystemCommitHashMap, int position = 1);
    void setUserBackupWrapper(BackupWrapper& w, const QString& prefixPath,
                              const QString& uuid, const QString& backupName);
    void setSystemBackupWrapper(BackupWrapper& w, const QString& prefixPath,
                                 const QString& uuid, int iPosition);

    DeleteBackupProxy* m_deleteBackupProxy;
};

#endif // TEST_DELETEBACKUPPROXY_H
