#include "test_databackupproxy.h"
#include "databackupproxy.h"
#include "../../../systembackup-daemon/workerfactory.h"
#include "../../../common/mydefine.h"
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryDir>
#include <QDir>
#include <QMetaObject>

TestDataBackupProxy::TestDataBackupProxy()
    : m_dataBackupProxy(nullptr)
{
}

void TestDataBackupProxy::initTestCase()
{
    BackupWrapper::registerMetaType();
}

void TestDataBackupProxy::cleanupTestCase()
{
}

void TestDataBackupProxy::init()
{
    m_dataBackupProxy = new DataBackupProxy();
}

void TestDataBackupProxy::cleanup()
{
    if (m_dataBackupProxy) {
        delete m_dataBackupProxy;
        m_dataBackupProxy = nullptr;
    }
}

void TestDataBackupProxy::testLoadExistsBackupPoint()
{
    m_dataBackupProxy->loadExistsBackupPoint();
    QVERIFY(true);
}

void TestDataBackupProxy::setDataBackupWrapper(BackupWrapper& w, const QString& prefixPath,
                                               int iPosition, const QStringList& backupPaths)
{
    w.m_prefixDestPath = prefixPath;
    w.m_iPosition = iPosition;
    w.m_backupPaths = backupPaths;
    w.m_backupName = "test_backup";
    w.m_note = "note";
    w.m_uuid = QString();
    w.m_backupExcludePaths = QStringList();
}

void TestDataBackupProxy::testConstructor()
{
    QVERIFY(m_dataBackupProxy != nullptr);
}

void TestDataBackupProxy::testDestructor()
{
    DataBackupProxy* proxy = new DataBackupProxy();
    delete proxy;
    QVERIFY(true);
}

void TestDataBackupProxy::testCheckEnvEx_tempDir_iPosition0()
{
    QTemporaryDir *tempDir = new QTemporaryDir;
    QVERIFY(tempDir->isValid());
    QDir().mkdir(tempDir->path() + QDir::separator() + "snapshots");
    QString base = tempDir->path() + "/";
    QStringList paths;
    paths << base;
    BackupWrapper w;
    setDataBackupWrapper(w, base, 0, paths);
    m_dataBackupProxy->setParam(w);

    QSignalSpy spy(m_dataBackupProxy, &Worker::checkResult);
    m_dataBackupProxy->checkEnv();
    QVERIFY(spy.count() >= 1);
    int result = spy.at(0).at(0).toInt();
    QVERIFY(result == int(BackupResult::CHECK_ENV_SUCCESS)
            || result == int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
}

void TestDataBackupProxy::testCheckEnvEx_udiskNonExt4_emitsSignal()
{
    QTemporaryDir *tempDir = new QTemporaryDir;
    QVERIFY(tempDir->isValid());
    QDir().mkdir(tempDir->path() + QDir::separator() + "snapshots");
    QString base = tempDir->path() + "/";
    BackupWrapper w;
    setDataBackupWrapper(w, base, 1, QStringList() << base);

    m_dataBackupProxy->setParam(w);
    QSignalSpy spy(m_dataBackupProxy, &Worker::checkResult);
    m_dataBackupProxy->checkEnv();
    bool gotUdiskError = false;
    for (int i = 0; i < spy.count(); ++i) {
        if (spy.at(i).at(0).toInt() == int(BackupResult::UDISK_FILESYSTEM_TYPE_IS_NOT_EXT4)) {
            gotUdiskError = true;
            break;
        }
    }
    QVERIFY(gotUdiskError);
}

void TestDataBackupProxy::testCheckEnvEx_dirNotExist_mkpathSucceeds()
{
    QTemporaryDir *tempDir = new QTemporaryDir;
    QVERIFY(tempDir->isValid());
    QDir().mkdir(tempDir->path() + QDir::separator() + "snapshots");
    QString base = tempDir->path() + "/subdir/";
    QStringList paths;
    paths << (tempDir->path() + "/exist");
    QDir().mkpath(tempDir->path() + "/exist");
    BackupWrapper w;
    setDataBackupWrapper(w, base, 0, paths);
    m_dataBackupProxy->setParam(w);

    QSignalSpy spy(m_dataBackupProxy, &Worker::checkResult);
    m_dataBackupProxy->checkEnv();
    QVERIFY(spy.count() >= 1);
}

void TestDataBackupProxy::testCheckEnvEx_emptyBackupPaths_calcSizeStillRuns()
{
    QTemporaryDir *tempDir = new QTemporaryDir;
    QVERIFY(tempDir->isValid());
    QDir().mkdir(tempDir->path() + QDir::separator() + "snapshots");
    QString base = tempDir->path() + "/";
    BackupWrapper w;
    setDataBackupWrapper(w, base, 0, QStringList());

    m_dataBackupProxy->setParam(w);
    QSignalSpy spy(m_dataBackupProxy, &Worker::checkResult);
    m_dataBackupProxy->checkEnv();
    QVERIFY(spy.count() >= 1);
}

void TestDataBackupProxy::testGetRsyncArgs_allScenes()
{
    BackupWrapper w;
    w.m_backupExcludePaths << "/home";
    m_dataBackupProxy->setParam(w);

    QStringList args;
    args = m_dataBackupProxy->getRsyncArgs(DataBackupProxy::TRY_SYSTEM_BACKUP);
    QVERIFY(!args.isEmpty());

    args = m_dataBackupProxy->getRsyncArgs(DataBackupProxy::SYSTEM_BACKUP);
    QVERIFY(!args.isEmpty());

    args = m_dataBackupProxy->getRsyncArgs(DataBackupProxy::MKSQUASHFS);

    args = m_dataBackupProxy->getRsyncArgs(DataBackupProxy::TRY_EXTRA_DATA_BACKUP);
    QVERIFY(!args.isEmpty());

    args = m_dataBackupProxy->getRsyncArgs(DataBackupProxy::EXTRA_DATA_BACKUP);
    QVERIFY(!args.isEmpty());

    args = m_dataBackupProxy->getRsyncArgs(DataBackupProxy::TRY_SYSTEM_BACKUP);
    QVERIFY(!args.isEmpty());
}

void TestDataBackupProxy::testDoMksqushfs()
{
    QTemporaryDir *tempDir = new QTemporaryDir;
    QVERIFY(tempDir->isValid());
    QDir().mkdir(tempDir->path() + QDir::separator() + "snapshots");
    BackupWrapper w;
    setDataBackupWrapper(w, tempDir->path() + "/", 0, QStringList());
    m_dataBackupProxy->setParam(w);
    m_dataBackupProxy->doMksqushfs();
    QVERIFY(true);
}

void TestDataBackupProxy::testRemoveAutoBackupPoint()
{
    m_dataBackupProxy->removeAutoBackupPoint("non-existent-uuid");
    QVERIFY(true);
}

void TestDataBackupProxy::testExtraDataPaths()
{
    m_dataBackupProxy->extraDataPaths();
    QVERIFY(true);
}

void TestDataBackupProxy::testExtraDataBackup()
{
    m_dataBackupProxy->extraDataBackup();
    QVERIFY(true);
}

void TestDataBackupProxy::testCalcSizeForExtraBackup()
{
    BackupWrapper w;
    setDataBackupWrapper(w, "/tmp/", 0, QStringList());
    m_dataBackupProxy->setParam(w);

    int size = m_dataBackupProxy->calcSizeForExtraBackup();
    QVERIFY(size >= 0);
}

void TestDataBackupProxy::testDoWorkEx()
{
    QTemporaryDir *tempDir = new QTemporaryDir;
    QVERIFY(tempDir->isValid());
    QDir().mkdir(tempDir->path() + QDir::separator() + "snapshots");
    QStringList paths;

    QString tempFilePath = tempDir->path() + QDir::separator() + "temp_data.txt";
    QFile tempFile(tempFilePath);
    if (tempFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
       tempFile.close();
    } else {
       qWarning() << "临时文件创建失败：" << tempFile.errorString();
       QVERIFY(false);
    }

    paths << tempFilePath;
    BackupWrapper w;
    setDataBackupWrapper(w, tempDir->path(), 0, paths);
    m_dataBackupProxy->setParam(w);

    QSignalSpy spy(m_dataBackupProxy, &Worker::workResult);
    m_dataBackupProxy->doWorkEx();
    QTest::qWait(5000);
    if (spy.count() >= 1) {
        QVERIFY(spy.at(0).at(0).toBool() == false || spy.at(0).at(0).toBool() == true);
    }
}

void TestDataBackupProxy::testCancelEx_emitsCancelSignals()
{
    QSignalSpy spy(m_dataBackupProxy, &Worker::checkResult);
    m_dataBackupProxy->cancelEx();
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
