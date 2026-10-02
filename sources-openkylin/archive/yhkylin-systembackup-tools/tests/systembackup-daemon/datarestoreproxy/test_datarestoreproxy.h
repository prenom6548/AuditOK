#ifndef TEST_DATARESTOREPROXY_H
#define TEST_DATARESTOREPROXY_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/datarestoreproxy.h"

struct BackupWrapper;

class TestDataRestoreProxy : public QObject
{
    Q_OBJECT

public:
    TestDataRestoreProxy() : m_proxy(nullptr) {}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();

    void testCheckEnvEx_backupPointMissing_emitsIncNotFoundDir();
    void testCheckEnvEx_commitHashMissing_emitsCheckCommitHashFail();
    void testCheckEnvEx_repoMissing_returnsFalse();
    void testCheckEnvEx_success_emitsCheckEnvSuccess();

    void testDoWorkEx_whenCheckEnvFails_noWorkResult();

    void testGetRsyncArgs_factoryAndAutoUuid_branches();
    void testGenerate16DigitRandomNumber_is16Digits();
    void testUpdateProgress_emitsPercent();
    void testClearDirectory_clearsFiles();
    void testRsyncCopy_nonexistentSource_returnsFalse();
    void testOstreeCheckout_invalidRepo_returnsFalse();
    void testMountImg_nonexistentImg_emitsRestoreDirPrepareFailed();
    void testRestoreExtraData_emptyOrMissing_returnsEarly();
    void testLinkFile_nonexistentSrc_returnsMinus1();

private:
    void setWrapper(BackupWrapper &w, const QString &prefixDestPath, const QString &uuid,
                    int position = 0, const QString &frontUser = QStringLiteral("testuser"));
    bool writeUserBackupXml(const QString &prefixDestPath, const QString &uuid,
                            const QString &systemHash, bool withCommitMap);
    void ensureRepoUserDirExists(const QString &prefixDestPath);

    DataRestoreProxy *m_proxy;
};

#endif // TEST_DATARESTOREPROXY_H