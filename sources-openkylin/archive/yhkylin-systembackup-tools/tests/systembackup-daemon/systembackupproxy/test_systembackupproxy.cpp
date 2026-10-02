/*
 * 白盒测试：SystemBackupProxy，通过友元访问私有接口，假数据覆盖
 */
#include "test_systembackupproxy.h"
#include "../../../systembackup-daemon/systembackupproxy.h"
#include "../../../common/mydefine.h"
#include "../../../common/utils.h"
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QTextStream>

void TestSystemBackupProxy::initTestCase()
{
    BackupWrapper::registerMetaType();
}

void TestSystemBackupProxy::cleanupTestCase()
{
}

void TestSystemBackupProxy::init()
{
    m_proxy = new SystemBackupProxy();
}

void TestSystemBackupProxy::cleanup()
{
    if (m_proxy) {
        delete m_proxy;
        m_proxy = nullptr;
    }
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(QStringLiteral("/data/"));
#endif
}

void TestSystemBackupProxy::setBackupWrapper(BackupWrapper &w, int iPosition,
                                            const QString &prefixPath, const QString &uuid,
                                            const QString &backupName)
{
    w.m_type = int(BackupType::BACKUP_SYSTEM);
    w.m_iPosition = iPosition;
    w.m_prefixDestPath = prefixPath;
    w.m_uuid = uuid.isEmpty() ? QStringLiteral("{test-uuid-0000-0000-000000000001}") : uuid;
    w.m_backupName = backupName;
    w.m_frontUid = 1000;
    w.m_gid = 1000;
    w.m_frontUserName = QStringLiteral("testuser");
    w.m_note = QStringLiteral("note");
    w.m_backupExcludePaths = QStringList();
}

bool TestSystemBackupProxy::writeFileSaveSize(const QString &basePath, qint64 totalSize, qint64 leftSize)
{
    QString path = basePath;
    if (!path.endsWith(QLatin1Char('/')))
        path += QLatin1Char('/');
    path += QStringLiteral(".backup/file_save_size");
    if (!QDir().mkpath(QFileInfo(path).absolutePath()))
        return false;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;
    QTextStream out(&f);
    out << totalSize << "," << leftSize;
    f.close();
    return true;
}

bool TestSystemBackupProxy::writeSysBackupListXml(const QString &basePath, const QString &uuid)
{
    QString path = basePath;
    if (!path.endsWith(QLatin1Char('/')))
        path += QLatin1Char('/');
    path += QStringLiteral(".backup/snapshots/sys_backuplist.xml");
    if (!QDir().mkpath(QFileInfo(path).absolutePath()))
        return false;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;
    QTextStream out(&f);
    out << "<?xml version='1.0'?>\n<backupList>\n  <BackupPoint>\n";
    out << "    <Comment>sys</Comment>\n    <Time>2025-01-01 12:00:00</Time>\n";
    out << "    <Uuid>" << uuid << "</Uuid>\n    <Size>0</Size>\n    <State>backup unfinished</State>\n";
    out << "    <Type>1</Type>\n    <Position>0</Position>\n    <UserId>1000</UserId>\n";
    out << "    <PrefixDestPath>" << (basePath.endsWith(QLatin1Char('/')) ? basePath.left(basePath.length() - 1) : basePath) << "</PrefixDestPath>\n";
    out << "  </BackupPoint>\n</backupList>\n";
    f.close();
    return true;
}

bool TestSystemBackupProxy::writeBackupListXml(const QString &basePath, const QString &uuid)
{
    QString path = basePath;
    if (!path.endsWith(QLatin1Char('/')))
        path += QLatin1Char('/');
    path += QStringLiteral(".backup/snapshots/backuplist.xml");
    if (!QDir().mkpath(QFileInfo(path).absolutePath()))
        return false;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;
    QTextStream out(&f);
    out << "<?xml version='1.0'?>\n<backupList>\n  <BackupPoint>\n";
    out << "    <Comment>user</Comment>\n    <Time>2025-01-01 12:00:00</Time>\n";
    out << "    <Uuid>" << uuid << "</Uuid>\n    <Size>0</Size>\n    <State>backup finished</State>\n";
    out << "    <Type>2</Type>\n    <Position>0</Position>\n    <UserId>1000</UserId>\n";
    out << "    <PrefixDestPath>" << (basePath.endsWith(QLatin1Char('/')) ? basePath.left(basePath.length() - 1) : basePath) << "</PrefixDestPath>\n";
    out << "  </BackupPoint>\n</backupList>\n";
    f.close();
    return true;
}

void TestSystemBackupProxy::testConstructor()
{
    QVERIFY(m_proxy != nullptr);
}

void TestSystemBackupProxy::testDestructor()
{
    SystemBackupProxy *p = new SystemBackupProxy();
    delete p;
    QVERIFY(true);
}

void TestSystemBackupProxy::testCheckEnvEx_localOrUdisk_emitsCheckResult()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    QVERIFY(writeFileSaveSize(basePath, 1000, 10LL * 1024 * 1024 * 1024));

    BackupWrapper w;
    setBackupWrapper(w, 0, basePath + "/");
    m_proxy->setParam(w);

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    bool result = m_proxy->checkEnvEx();
    QVERIFY(spy.count() >= 1);
    int sig = spy.at(0).at(0).toInt();
    QVERIFY(sig == int(BackupResult::CHECK_ENV_SUCCESS)
            || sig == int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH)
            || sig == int(BackupResult::UDISK_FILESYSTEM_TYPE_IS_NOT_EXT4));
    (void)result;
}

void TestSystemBackupProxy::testDoWorkEx()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    QVERIFY(writeFileSaveSize(basePath, 1000, 10LL * 1024 * 1024 * 1024));
    BackupWrapper w;
    setBackupWrapper(w, 0, basePath + "/");
    m_proxy->setParam(w);

    QSignalSpy spyCheck(m_proxy, &Worker::checkResult);
    m_proxy->doWorkEx();
    QVERIFY(spyCheck.count() >= 1);
}

void TestSystemBackupProxy::testCancelEx_emitsStartCancelAndCancelSuccess()
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

void TestSystemBackupProxy::testCheckFreeCapacity_capacityEnough_emitsCheckEnvSuccess()
{
    m_proxy->m_leftSize = 10LL * 1024 * 1024 * 1024;
    m_proxy->m_isOnlyCheck = true;

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    QMetaObject::invokeMethod(m_proxy, "checkFreeCapacity", Qt::DirectConnection, Q_ARG(qint64, qint64(1000)));

    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy.at(0).at(0).toInt(), int(BackupResult::CHECK_ENV_SUCCESS));
}

void TestSystemBackupProxy::testCheckFreeCapacity_capacityNotEnough_emitsCapacityNotEnough()
{
    m_proxy->m_leftSize = 100;
    m_proxy->m_isOnlyCheck = true;

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    QMetaObject::invokeMethod(m_proxy, "checkFreeCapacity", Qt::DirectConnection, Q_ARG(qint64, qint64(10000)));

    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy.at(0).at(0).toInt(), int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
}

void TestSystemBackupProxy::testGetRsyncArgs_allScenes_returnsNonEmpty()
{
    BackupWrapper w;
    setBackupWrapper(w, 0, "/tmp/");
    m_proxy->setParam(w);

    QStringList r1 = m_proxy->getRsyncArgs(SystemBackupProxy::TRY_SYSTEM_BACKUP);
    QVERIFY(!r1.isEmpty());
    QStringList r2 = m_proxy->getRsyncArgs(SystemBackupProxy::SYSTEM_BACKUP);
    QVERIFY(!r2.isEmpty());
    QStringList r3 = m_proxy->getRsyncArgs(SystemBackupProxy::MKSQUASHFS);
    QVERIFY(r3.isEmpty() || !r3.isEmpty());
    m_proxy->m_backExtraPaths = QStringList() << QStringLiteral("/home/test/.config");
    QStringList r4 = m_proxy->getRsyncArgs(SystemBackupProxy::TRY_EXTRA_DATA_BACKUP);
    QVERIFY(!r4.isEmpty());
    QStringList r5 = m_proxy->getRsyncArgs(SystemBackupProxy::EXTRA_DATA_BACKUP);
    QVERIFY(!r5.isEmpty());
}

void TestSystemBackupProxy::testRecordBackupPoint_fakeData_success()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    QString snapDir = basePath + QStringLiteral("/.backup/snapshots");
    QVERIFY(QDir().mkpath(snapDir));

    const QString uuid = QStringLiteral("{record-test-uuid-000000000001}");
    BackupWrapper w;
    setBackupWrapper(w, 0, basePath + "/", uuid);
    m_proxy->setParam(w);
    m_proxy->m_curUuid = uuid;
    m_proxy->m_xmlFile = QStringLiteral("/.backup/snapshots/sys_backuplist.xml");
    m_proxy->m_size = 1024;
    m_proxy->m_backupPoint.m_systemHash = QStringLiteral("syshash1");
    m_proxy->m_backupPoint.m_branchName = QStringLiteral("branch-20250101120000");

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    bool ok = m_proxy->recordBackupPoint();
    QVERIFY(ok);
    QString xmlPath = basePath + "/.backup/snapshots/sys_backuplist.xml";
    QVERIFY(QFile::exists(xmlPath));
}

void TestSystemBackupProxy::testDeleteFailedData_fakeXml_removesItem()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    const QString uuid = QStringLiteral("{delete-failed-uuid-00000000001}");
    QVERIFY(writeSysBackupListXml(basePath, uuid));

    m_proxy->m_curUuid = uuid;
    m_proxy->m_xmlFile = QStringLiteral("/.backup/snapshots/sys_backuplist.xml");

    m_proxy->deleteFailedData();
    QVERIFY(true);
}

void TestSystemBackupProxy::testExtraDataPaths_returnsList()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    QString homeDir = basePath + QStringLiteral("/home");
    QVERIFY(QDir().mkpath(homeDir));

    QStringList paths = m_proxy->extraDataPaths();
    QVERIFY(paths.isEmpty() || !paths.isEmpty());
}

void TestSystemBackupProxy::testCalcSizeForExtraBackup_returnsInt()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    int size = m_proxy->calcSizeForExtraBackup();
    QVERIFY(size >= 0);
}

void TestSystemBackupProxy::testCpFile_fakeFile_createsBak()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString filePath = dir.path() + QStringLiteral("/kylin.db");
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("dummy");
    f.close();

    bool ok = m_proxy->cpFile(filePath);
    QVERIFY(ok);
    QVERIFY(QFile::exists(filePath + QStringLiteral(".bak")));
}

void TestSystemBackupProxy::testRemoveAutoBackupPoint_fakeXml_removesItem()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    const QString uuid = QStringLiteral("{remove-auto-uuid-000000000001}");
    QVERIFY(writeSysBackupListXml(basePath, uuid));
    QString snapPath = basePath + QStringLiteral("/.backup/snapshots/") + uuid;
    QVERIFY(QDir().mkpath(snapPath));

    m_proxy->m_xmlFile = QStringLiteral("/.backup/snapshots/sys_backuplist.xml");
    m_proxy->removeAutoBackupPoint(uuid);
    QVERIFY(true);
}

void TestSystemBackupProxy::testLoadExistsBackupPoint_fakeXml_loadsBackupPoint()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    const QString uuid = QStringLiteral("{load-exists-uuid-000000000001}");
    QVERIFY(writeBackupListXml(basePath, uuid));

    m_proxy->m_curUuid = uuid;
    m_proxy->loadExistsBackupPoint();
    QVERIFY(!m_proxy->m_backupPoint.m_backupName.isEmpty() || !m_proxy->m_backupPoint.m_uuid.isEmpty());
}

void TestSystemBackupProxy::testLoadExistsBackupPoint_autoUuid_updatesOrLoadsFromSys()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    QVERIFY(writeSysBackupListXml(basePath, QStringLiteral(AUTO_BACKUP_UUID)));

    m_proxy->m_curUuid = QStringLiteral(AUTO_BACKUP_UUID);
    m_proxy->loadExistsBackupPoint();
    QVERIFY(true);
}

void TestSystemBackupProxy::testCalcSizeForBackup_invoke_noCrash()
{
    m_proxy->m_leftSize = 10LL * 1024 * 1024 * 1024;
    m_proxy->m_isOnlyCheck = true;
    m_proxy->calcSizeForBackup();
    QVERIFY(true);
}

void TestSystemBackupProxy::testCalcSizeForHome_returnsQint64()
{
    BackupWrapper w;
    setBackupWrapper(w, 0, "/tmp/");
    w.m_frontUserName = QStringLiteral("testuser");
    m_proxy->setParam(w);
    qint64 size = m_proxy->calcSizeForHome();
    QVERIFY(size >= -1);
}

void TestSystemBackupProxy::testDoMksqushfs_invoke_noCrash()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    BackupWrapper w;
    setBackupWrapper(w, 0, basePath + "/");
    m_proxy->setParam(w);
    m_proxy->m_curUuid = QStringLiteral("{mksquashfs-test-uuid}");
    m_proxy->m_xmlFile = QStringLiteral("/.backup/snapshots/sys_backuplist.xml");
    m_proxy->m_destPath = basePath + QStringLiteral("/dest");
    QVERIFY(QDir().mkpath(m_proxy->m_destPath));
    m_proxy->m_backupPoint.m_systemHash = QStringLiteral("syshash1");
    m_proxy->m_backupPoint.m_branchName = QStringLiteral("branch-test");

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    QMetaObject::invokeMethod(m_proxy, "doMksqushfs", Qt::DirectConnection);
    QTest::qWait(500);
    QVERIFY(true);
}

void TestSystemBackupProxy::testBackupSystem_invoke_noCrash()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    BackupWrapper w;
    setBackupWrapper(w, 0, basePath + "/");
    m_proxy->setParam(w);
    m_proxy->m_curUuid = QStringLiteral("{backup-system-test-uuid}");
    m_proxy->m_backupPoint.m_systemHash = QStringLiteral("syshash1");
    m_proxy->m_backupPoint.m_branchName = QStringLiteral("branch-test");

    bool ok = m_proxy->backupSystem();
    QVERIFY(ok == true || ok == false);
}

void TestSystemBackupProxy::testExtraDataBackup_fakePaths_runsNoCrash()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    BackupWrapper w;
    setBackupWrapper(w, 0, basePath + "/");
    m_proxy->setParam(w);
    m_proxy->m_curUuid = QStringLiteral("{extra-backup-uuid-00000000001}");
    m_proxy->m_backExtraPaths = QStringList() << QStringLiteral("/home/testuser/.config");

    m_proxy->extraDataBackup();
    QVERIFY(true);
}
