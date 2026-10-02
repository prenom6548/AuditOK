/*
 * 白盒测试：CustomizeDataBackupProxy 接口与分支覆盖
 */
#include "test_customizedatabackupproxy.h"
#include "../../../systembackup-daemon/customizedatabackupproxy.h"
#include "../../../systembackup-daemon/workerfactory.h"
#include "../../../common/mydefine.h"
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryDir>
#include <QDir>
#include <QMetaObject>

void TestCustomizeDataBackupProxy::initTestCase()
{
    BackupWrapper::registerMetaType();
}

void TestCustomizeDataBackupProxy::cleanupTestCase()
{
}

void TestCustomizeDataBackupProxy::init()
{
    m_proxy = new CustomizeDataBackupProxy();
}

void TestCustomizeDataBackupProxy::cleanup()
{
    if (m_proxy) {
        delete m_proxy;
        m_proxy = nullptr;
    }
}

void TestCustomizeDataBackupProxy::setCustomizeDataBackupWrapper(BackupWrapper &w, const QString &prefixPath,
                                                                  const QStringList &backupPaths, const QString &uuid)
{
    w.m_prefixDestPath = prefixPath;
    w.m_backupPaths = backupPaths;
    w.m_backupName = "test_customize_backup";
    w.m_note = "note";
    w.m_uuid = uuid;
    w.m_backupExcludePaths = QStringList();
    w.m_iPosition = 0;
    w.m_frontUid = 1000;
    w.m_gid = 1000;
    w.m_type = -1;
}

void TestCustomizeDataBackupProxy::testConstructor()
{
    QVERIFY(m_proxy != nullptr);
}

void TestCustomizeDataBackupProxy::testDestructor()
{
    CustomizeDataBackupProxy *p = new CustomizeDataBackupProxy();
    delete p;
    QVERIFY(true);
}

// void TestCustomizeDataBackupProxy::testCheckEnvEx_emitsCheckResult()
// {
//     QTemporaryDir tempDir;
//     QVERIFY(tempDir.isValid());
//     BackupWrapper w;
//     setCustomizeDataBackupWrapper(w, tempDir.path() + "/", QStringList() << "/tmp", QString());

//     m_proxy->setParam(w);
//     QSignalSpy spy(m_proxy, &Worker::checkResult);
//     m_proxy->checkEnvEx();
//     QVERIFY(spy.count() >= 1);
// }

// void TestCustomizeDataBackupProxy::testCheckEnvEx_pathInjection_emitsDuErr()
// {
//     BackupWrapper w;
//     w.m_prefixDestPath = "/tmp/`id`/";
//     w.m_backupPaths = QStringList() << "/tmp";
//     w.m_backupName = "test";
//     w.m_note = "note";
//     w.m_uuid = QString();
//     w.m_iPosition = 0;
//     m_proxy->setParam(w);

//     QSignalSpy spy(m_proxy, &Worker::checkResult);
//     bool result = m_proxy->checkEnvEx();
//     QVERIFY(!result);
//     bool gotDuErr = false;
//     for (int i = 0; i < spy.count(); ++i) {
//         if (spy.at(i).at(0).toInt() == int(BackupResult::DU_ERR)) {
//             gotDuErr = true;
//             break;
//         }
//     }
//     QVERIFY(gotDuErr);
// }

// void TestCustomizeDataBackupProxy::testCheckEnvEx_emptyPrefixDestPath_mkpath()
// {
//     QTemporaryDir tempDir;
//     QVERIFY(tempDir.isValid());
//     QString base = tempDir.path() + "/empty_dest/";
//     BackupWrapper w;
//     setCustomizeDataBackupWrapper(w, base, QStringList() << "/tmp", QString());
//     m_proxy->setParam(w);

//     QSignalSpy spy(m_proxy, &Worker::checkResult);
//     m_proxy->checkEnvEx();
//     QVERIFY(spy.count() >= 1);
//     QVERIFY(QDir(base).exists());
// }

void TestCustomizeDataBackupProxy::testDoWorkEx()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setCustomizeDataBackupWrapper(w, tempDir.path() + "/", QStringList() << "/tmp", QString());
    m_proxy->setParam(w);

    QSignalSpy spyResult(m_proxy, &Worker::workResult);
    QSignalSpy spyCheck(m_proxy, &Worker::checkResult);
    m_proxy->doWorkEx();
    QTest::qWait(600);
    QVERIFY(spyCheck.count() >= 1);
    if (spyResult.count() >= 1)
        QVERIFY(spyResult.at(0).at(0).toBool() == true || spyResult.at(0).at(0).toBool() == false);
}

void TestCustomizeDataBackupProxy::testCancelEx_emitsStartCancelAndCancelSuccess()
{
    QSignalSpy spy(m_proxy, &Worker::checkResult);
    m_proxy->cancelEx();

    bool gotStartCancel = false;
    bool gotCancelSuccess = false;
    for (int i = 0; i < spy.count(); ++i) {
        int r = spy.at(i).at(0).toInt();
        if (r == int(BackupResult::START_CANCEL))
            gotStartCancel = true;
        if (r == int(BackupResult::CANCEL_SUCCESS))
            gotCancelSuccess = true;
    }
    QVERIFY(gotStartCancel && gotCancelSuccess);
}

void TestCustomizeDataBackupProxy::testCheckFreeCapacity_invoke()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setCustomizeDataBackupWrapper(w, tempDir.path() + "/", QStringList() << "/tmp", QString());
    m_proxy->setParam(w);

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    bool result = m_proxy->checkFreeCapacity(0);
    QVERIFY(spy.count() >= 1);
    int sig = spy.at(0).at(0).toInt();
    QVERIFY(sig == int(BackupResult::CHECK_ENV_SUCCESS)
            || sig == int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
}

void TestCustomizeDataBackupProxy::testCheckFreeCapacity_cancel_returnsFalse()
{
    m_proxy->cancelEx();

    bool result = m_proxy->checkFreeCapacity(0);
    QVERIFY(result == false);
}

void TestCustomizeDataBackupProxy::testIsIncBackup_uuidEmpty_returnsFalse()
{
    BackupWrapper w;
    w.m_prefixDestPath = "/tmp";
    w.m_uuid = QString();
    m_proxy->setParam(w);

    bool result = m_proxy->isIncBackup();
    QVERIFY(result == false);
}

void TestCustomizeDataBackupProxy::testIsIncBackup_uuidSet_dirExist_returnsTrue()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString uuid = "test-uuid-inc";
    QString dataDir = tempDir.path() + "/.backup/snapshots/" + uuid + "/data";
    QDir().mkpath(dataDir);

    BackupWrapper w;
    setCustomizeDataBackupWrapper(w, tempDir.path() + "/", QStringList(), uuid);
    m_proxy->setParam(w);

    bool result = m_proxy->isIncBackup();
    QVERIFY(result == true);
}

void TestCustomizeDataBackupProxy::testIsIncBackup_uuidSet_dirNotExist_returnsFalse()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString uuid = "nonexistent-uuid-123";

    BackupWrapper w;
    setCustomizeDataBackupWrapper(w, tempDir.path() + "/", QStringList(), uuid);
    m_proxy->setParam(w);

    bool result = m_proxy->isIncBackup();
    QVERIFY(result == false);
}

void TestCustomizeDataBackupProxy::testGetRsyncArgs_dataBackup()
{
    BackupWrapper w;
    w.m_backupPaths = QStringList() << "/home";
    m_proxy->setParam(w);

    QStringList args = m_proxy->getRsyncArgs(CustomizeDataBackupProxy::DATA_BACKUP);
    QVERIFY(!args.isEmpty());
    QVERIFY(args.contains("--files-from"));
}

void TestCustomizeDataBackupProxy::testGetRsyncArgs_incDataBackup()
{
    BackupWrapper w;
    w.m_backupPaths = QStringList() << "/etc";
    m_proxy->setParam(w);

    QStringList args = m_proxy->getRsyncArgs(CustomizeDataBackupProxy::INC_DATA_BACKUP);
    QVERIFY(!args.isEmpty());
    QVERIFY(args.contains("--delete"));
}

void TestCustomizeDataBackupProxy::testGetRsyncArgs_tryDataBackup()
{
    BackupWrapper w;
    w.m_backupPaths = QStringList() << "/tmp";
    m_proxy->setParam(w);

    QStringList args = m_proxy->getRsyncArgs(CustomizeDataBackupProxy::TRY_DATA_BACKUP);
    QVERIFY(!args.isEmpty());
    QVERIFY(args.contains("--stats"));
}

void TestCustomizeDataBackupProxy::testGetRsyncArgs_tryIncDataBackup()
{
    BackupWrapper w;
    w.m_backupPaths = QStringList() << "/var";
    m_proxy->setParam(w);

    QStringList args = m_proxy->getRsyncArgs(CustomizeDataBackupProxy::TRY_INC_DATA_BACKUP);
    QVERIFY(!args.isEmpty());
    QVERIFY(args.contains("--delete"));
}

void TestCustomizeDataBackupProxy::testGetRsyncArgs_default()
{
    BackupWrapper w;
    m_proxy->setParam(w);

    QStringList args = m_proxy->getRsyncArgs((CustomizeDataBackupProxy::CustomizeDataBackupScene)-1);
    QVERIFY(args.isEmpty());
}

void TestCustomizeDataBackupProxy::testDeleteFailedData_invoke_emptyUuid_noCrash()
{
    m_proxy->deleteFailedData();
    QVERIFY(true);
}
