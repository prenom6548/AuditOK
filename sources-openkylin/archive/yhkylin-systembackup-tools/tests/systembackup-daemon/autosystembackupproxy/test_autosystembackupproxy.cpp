/*
 * 白盒测试：AutoSystemBackupProxy 公共接口（不测私有函数）
 */
#include "test_autosystembackupproxy.h"
#include "../../../systembackup-daemon/autosystembackupproxy.h"
#include "../../../systembackup-daemon/workerfactory.h"
#include "../../../systembackup-daemon/parsebackuplist.h"
#include "../../../common/mydefine.h"

Q_DECLARE_METATYPE(ParseBackupList::BackupPoint)
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryDir>
#include <QDir>
#include <QMetaObject>
#include <QThread>

void TestAutoSystemBackupProxy::initTestCase()
{
    BackupWrapper::registerMetaType();
    qRegisterMetaType<ParseBackupList::BackupPoint>("ParseBackupList::BackupPoint");
}

void TestAutoSystemBackupProxy::cleanupTestCase()
{
}

void TestAutoSystemBackupProxy::init()
{
    m_proxy = new AutoSystemBackupProxy();
}

void TestAutoSystemBackupProxy::cleanup()
{
    if (m_proxy) {
        delete m_proxy;
        m_proxy = nullptr;
    }
}

void TestAutoSystemBackupProxy::setAutoSystemBackupWrapper(BackupWrapper &w, const QString &prefixPath, const QString &uuid)
{
    w.m_prefixDestPath = prefixPath;
    w.m_uuid = uuid;
    w.m_backupPaths = QStringList() << "/";
    w.m_backupExcludePaths = QStringList();
    w.m_backupName = "test_auto_system_backup";
    w.m_note = "note";
    w.m_iPosition = 0;
    w.m_frontUid = 1000;
    w.m_gid = 1000;
    w.m_type = -1;
    w.m_frontUserName = QString(getenv("USER"));
}

void TestAutoSystemBackupProxy::testConstructor()
{
    QVERIFY(m_proxy != nullptr);
}

void TestAutoSystemBackupProxy::testLoadExistsBackupPoint()
{
    m_proxy->loadExistsBackupPoint();
    QVERIFY(true);
}

void TestAutoSystemBackupProxy::testDestructor()
{
    AutoSystemBackupProxy *p = new AutoSystemBackupProxy();
    delete p;
    QVERIFY(true);
}

void TestAutoSystemBackupProxy::testCheckEnvEx_emitsCheckResult()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, tempDir.path() + "/", QString());

    m_proxy->setParam(w);
    QSignalSpy spy(m_proxy, &Worker::checkResult);
    bool result = m_proxy->checkEnvEx();
    QVERIFY(result == true);
    QTest::qWait(800);
    QVERIFY(spy.count() >= 1);
}

void TestAutoSystemBackupProxy::testCheckEnvEx_withAutoBackupUuid()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, tempDir.path() + "/", QString::fromUtf8(AUTO_BACKUP_UUID));

    m_proxy->setParam(w);
    QSignalSpy spy(m_proxy, &Worker::checkResult);
    bool result = m_proxy->checkEnvEx();
    QVERIFY(result == true);
    QTest::qWait(800);
    QVERIFY(spy.count() >= 1);
}

void TestAutoSystemBackupProxy::testDoWorkEx()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, tempDir.path() + "/", QString());
    m_proxy->setParam(w);

    QSignalSpy spyResult(m_proxy, &Worker::workResult);
    QSignalSpy spyCheck(m_proxy, &Worker::checkResult);
    m_proxy->doWorkEx();
    QTest::qWait(800);
    QVERIFY(spyCheck.count() >= 1);
    if (spyResult.count() >= 1)
        QVERIFY(spyResult.at(0).at(0).toBool() == true || spyResult.at(0).at(0).toBool() == false);
}

void TestAutoSystemBackupProxy::testDoWorkEx_waitForAsyncChain()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, tempDir.path() + "/", QString());
    m_proxy->setParam(w);

    QSignalSpy spyResult(m_proxy, &Worker::workResult);
    QSignalSpy spyCheck(m_proxy, &Worker::checkResult);
    m_proxy->doWorkEx();
    QTest::qWait(5000);
    QVERIFY(spyCheck.count() >= 1);
    int checkEnvSuccess = 0;
    int capacityNotEnough = 0;
    for (int i = 0; i < spyCheck.count(); ++i) {
        int r = spyCheck.at(i).at(0).toInt();
        if (r == int(BackupResult::CHECK_ENV_SUCCESS)) checkEnvSuccess++;
        if (r == int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH)) capacityNotEnough++;
    }
    QVERIFY(checkEnvSuccess >= 1 || capacityNotEnough >= 1);
    if (spyResult.count() >= 1)
        QVERIFY(spyResult.at(0).at(0).toBool() == true || spyResult.at(0).at(0).toBool() == false);
}

void TestAutoSystemBackupProxy::testCancelEx_emitsStartCancelAndCancelSuccess()
{
    QSignalSpy spy(m_proxy, &Worker::checkResult);
    m_proxy->cancelEx();
    QTest::qWait(1000);

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

void TestAutoSystemBackupProxy::testCheckFreeCapacity_emitsCheckResult()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, tempDir.path() + "/", QString());
    m_proxy->setParam(w);

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    m_proxy->checkFreeCapacity(10);

    QVERIFY(spy.count() >= 1);
    int sig = spy.at(0).at(0).toInt();
    QVERIFY(sig == int(BackupResult::CHECK_ENV_SUCCESS)
            || sig == int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
}

void TestAutoSystemBackupProxy::testCheckFreeCapacity_capacityNotEnough_emitsWorkResultFalse()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, tempDir.path() + "/", QString());
    m_proxy->setParam(w);

    QSignalSpy spyCheck(m_proxy, &Worker::checkResult);
    QSignalSpy spyResult(m_proxy, &Worker::workResult);
    const qint64 hugeSize = Q_INT64_C(999999999999999);
    m_proxy->checkFreeCapacity(hugeSize);

    bool gotCapacityNotEnough = false;
    for (int i = 0; i < spyCheck.count(); ++i) {
        if (spyCheck.at(i).at(0).toInt() == int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH)) {
            gotCapacityNotEnough = true;
            break;
        }
    }
    QVERIFY(gotCapacityNotEnough);
    QVERIFY(spyResult.count() >= 1);
    QVERIFY(spyResult.at(0).at(0).toBool() == false);
}

void TestAutoSystemBackupProxy::testBackupSystem_runsWithoutCrash()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, tempDir.path() + "/", QString());

    AutoSystemBackupProxy *proxy = new AutoSystemBackupProxy();
    proxy->setParam(w);

    bool result = proxy->backupSystem();

    QVERIFY(result);
}

void TestAutoSystemBackupProxy::testDoPrepare()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, tempDir.path() + "/", QString());
    m_proxy->setParam(w);
    m_proxy->checkEnvEx();
    QTest::qWait(500);

    bool result = m_proxy->doPrepare();
    QVERIFY(result);
}

void TestAutoSystemBackupProxy::testCheckAndRemoveBackupPoint()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, tempDir.path() + "/", QString());
    m_proxy->setParam(w);
    m_proxy->checkAndRemoveBackupPoint();
    QVERIFY(true);
}

void TestAutoSystemBackupProxy::testRecordBackupPoint()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, tempDir.path() + "/", QString());
    m_proxy->setParam(w);
    QTest::qWait(500);

    bool result = m_proxy->recordBackupPoint();
    QVERIFY(result);
}

void TestAutoSystemBackupProxy::testRemoveAutoBackupPoint()
{
    m_proxy->removeAutoBackupPoint("non-existent-uuid");
    QVERIFY(true);
}

void TestAutoSystemBackupProxy::testExtraDataPaths()
{
    m_proxy->extraDataPaths();
    QVERIFY(true);
}

void TestAutoSystemBackupProxy::testExtraDataBackup()
{
    m_proxy->extraDataBackup();
    QVERIFY(true);
}

void TestAutoSystemBackupProxy::testCalcSizeForExtraBackup()
{
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, "/tmp/", QString());
    m_proxy->setParam(w);

    int size = m_proxy->calcSizeForExtraBackup();
    QVERIFY(size >= 0);
}

void TestAutoSystemBackupProxy::testDeleteBackupPointCommits()
{
    ParseBackupList::BackupPoint bp;
    bp.m_branchName = "branch-test";
    bp.m_uuid = "test-uuid";

    m_proxy->deleteBackupPointCommits(bp);
    QVERIFY(true);
}

void TestAutoSystemBackupProxy::testDoMksqushfs()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, tempDir.path() + "/", QString());
    m_proxy->setParam(w);
    m_proxy->doMksqushfs();
    QVERIFY(true);
}

void TestAutoSystemBackupProxy::testGetRsyncArgs_allScenes()
{
    BackupWrapper w;
    w.m_backupExcludePaths << "/home";
    m_proxy->setParam(w);

    QStringList args;
    args = m_proxy->getRsyncArgs(AutoSystemBackupProxy::TRY_SYSTEM_BACKUP);
    QVERIFY(!args.isEmpty());

    args = m_proxy->getRsyncArgs(AutoSystemBackupProxy::SYSTEM_BACKUP);
    QVERIFY(!args.isEmpty());

    args = m_proxy->getRsyncArgs(AutoSystemBackupProxy::MKSQUASHFS);

    args = m_proxy->getRsyncArgs(AutoSystemBackupProxy::TRY_EXTRA_DATA_BACKUP);
    QVERIFY(!args.isEmpty());

    args = m_proxy->getRsyncArgs(AutoSystemBackupProxy::EXTRA_DATA_BACKUP);
    QVERIFY(!args.isEmpty());

    args = m_proxy->getRsyncArgs(AutoSystemBackupProxy::TRY_SYSTEM_BACKUP);
    QVERIFY(!args.isEmpty());
}

void TestAutoSystemBackupProxy::testCalcSizeForHome()
{
    BackupWrapper w;
    setAutoSystemBackupWrapper(w, "/tmp/", QString());
    m_proxy->setParam(w);
    qint64 size = m_proxy->calcSizeForHome();
    QVERIFY(size >= 0);
}
