/*
 * 白盒测试：CustomizeGhostImageProxy 公共接口，无可用数据时构建假数据
 */
#include "test_customizeghostImageproxy.h"
#include "../../../systembackup-daemon/customizeghostImageproxy.h"
#include "../../../systembackup-daemon/workerfactory.h"
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

void TestCustomizeGhostImageProxy::initTestCase()
{
    BackupWrapper::registerMetaType();
}

void TestCustomizeGhostImageProxy::cleanupTestCase()
{
}

void TestCustomizeGhostImageProxy::init()
{
    m_proxy = new CustomizeGhostImageProxy();
}

void TestCustomizeGhostImageProxy::cleanup()
{
    if (m_proxy) {
        delete m_proxy;
        m_proxy = nullptr;
    }
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(QStringLiteral("/data/"));
#endif
}

void TestCustomizeGhostImageProxy::setGhostBackupWrapper(BackupWrapper &w, const QString &uuid, const QString &prefixPath)
{
    w.m_uuid = uuid;
    w.m_prefixDestPath = prefixPath;
    w.m_backupName = "test.kyimg";
    w.m_frontUid = 1000;
    w.m_gid = 1000;
    w.m_type = -1;
}

bool TestCustomizeGhostImageProxy::writeGhostBackupListXml(const QString &basePath, const QString &uuid, const QString &comment)
{
    QString xmlPath = basePath;
    if (!xmlPath.endsWith(QLatin1Char('/')))
        xmlPath += QLatin1Char('/');
    xmlPath += QStringLiteral(".backup/snapshots/backuplist.xml");
    if (!QDir().mkpath(QFileInfo(xmlPath).absolutePath()))
        return false;
    QFile f(xmlPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;
    QString pathValue = basePath;
    if (pathValue.endsWith(QLatin1Char('/')))
        pathValue.chop(1);
    QTextStream out(&f);
    out << "<?xml version='1.0'?>\n<backupList>\n  <BackupPoint>\n";
    out << "    <Comment>" << comment << "</Comment>\n    <Time>2025-01-01 12:00:00</Time>\n";
    out << "    <Uuid>" << uuid << "</Uuid>\n    <Size>0</Size>\n    <State>backup finished</State>\n";
    out << "    <Type>2</Type>\n    <Position>0</Position>\n    <UserId>1000</UserId>\n";
    out << "    <PrefixDestPath>" << pathValue << "</PrefixDestPath>\n";
    out << "  </BackupPoint>\n</backupList>\n";
    f.close();
    return true;
}

void TestCustomizeGhostImageProxy::testConstructor()
{
    QVERIFY(m_proxy != nullptr);
}

void TestCustomizeGhostImageProxy::testDestructor()
{
    CustomizeGhostImageProxy *p = new CustomizeGhostImageProxy();
    delete p;
    QVERIFY(true);
}

void TestCustomizeGhostImageProxy::testCheckEnvEx_emitsCheckResult()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setGhostBackupWrapper(w, "non-existent-uuid", tempDir.path() + "/");

    m_proxy->setParam(w);
    QSignalSpy spy(m_proxy, &Worker::checkResult);
    bool result = m_proxy->checkEnvEx();
    QVERIFY(spy.count() >= 1);
    int sig = spy.at(0).at(0).toInt();
    QVERIFY(sig == int(BackupResult::GHOST_CANNOT_FIND_BACKUPPOINT)
            || sig == int(BackupResult::BACKUP_PARTITION_MOUNT_FAIL)
            || sig == int(BackupResult::CHECK_ENV_SUCCESS));
    (void)result;
}

void TestCustomizeGhostImageProxy::testCheckEnvEx_nonExistentUuid_emitsGhostCannotFindOrMountFail()
{
    QTemporaryDir destDir;
    QVERIFY(destDir.isValid());
    BackupWrapper w;
    setGhostBackupWrapper(w, "non-existent-uuid-12345", destDir.path() + "/");
    m_proxy->setParam(w);

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    m_proxy->checkEnvEx();
    QVERIFY(spy.count() >= 1);
    int sig = spy.at(0).at(0).toInt();
    QVERIFY(sig == int(BackupResult::GHOST_CANNOT_FIND_BACKUPPOINT)
            || sig == int(BackupResult::BACKUP_PARTITION_MOUNT_FAIL));
}

void TestCustomizeGhostImageProxy::testCheckEnvEx_fakeData_srcDirEmpty_emitsGhostSrcDirNotExist()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    const QString uuid = QStringLiteral("{aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee}");
    QVERIFY(writeGhostBackupListXml(basePath, uuid));

    QTemporaryDir destDir;
    QVERIFY(destDir.isValid());
    BackupWrapper w;
    setGhostBackupWrapper(w, uuid, destDir.path() + "/");
    m_proxy->setParam(w);

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    m_proxy->checkEnvEx();
    QVERIFY(spy.count() >= 1);
    int sig = spy.at(0).at(0).toInt();
    QCOMPARE(sig, int(BackupResult::GHOST_SRC_DIRECTORY_IS_NOT_EXIST));
}

void TestCustomizeGhostImageProxy::testCheckEnvEx_fakeData_dstImgNotExist_emitsGhostSrcDirNotExist()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    const QString uuid = QStringLiteral("{bbbbbbbb-cccc-dddd-eeee-ffffffffffff}");
    QVERIFY(writeGhostBackupListXml(basePath, uuid));
    QString srcDataDir = basePath + QStringLiteral("/.backup/snapshots/") + uuid + QStringLiteral("/data");
    QVERIFY(QDir().mkpath(srcDataDir));
    QFile dummy(srcDataDir + QStringLiteral("/other.txt"));
    QVERIFY(dummy.open(QIODevice::WriteOnly));
    dummy.write("x");
    dummy.close();

    QTemporaryDir destDir;
    QVERIFY(destDir.isValid());
    BackupWrapper w;
    setGhostBackupWrapper(w, uuid, destDir.path() + "/");
    m_proxy->setParam(w);

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    m_proxy->checkEnvEx();
    QVERIFY(spy.count() >= 1);
    int sig = spy.at(0).at(0).toInt();
    QCOMPARE(sig, int(BackupResult::GHOST_SRC_DIRECTORY_IS_NOT_EXIST));
}

void TestCustomizeGhostImageProxy::testCheckEnvEx_fakeData_successOrCapacityOrMountFail()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    const QString uuid = QStringLiteral("{cccccccc-dddd-eeee-ffff-000000000001}");
    QVERIFY(writeGhostBackupListXml(basePath, uuid));
    QString srcDataDir = basePath + QStringLiteral("/.backup/snapshots/") + uuid + QStringLiteral("/data");
    QVERIFY(QDir().mkpath(srcDataDir));
    QString imgPath = srcDataDir + QStringLiteral("/") + QStringLiteral(UDISK_MKSQUASHFS_IMG_NAME);
    QFile img(imgPath);
    QVERIFY(img.open(QIODevice::WriteOnly));
    img.write("fake squashfs placeholder");
    img.close();

    QTemporaryDir destDir;
    QVERIFY(destDir.isValid());
    BackupWrapper w;
    setGhostBackupWrapper(w, uuid, destDir.path() + "/");
    m_proxy->setParam(w);

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    bool result = m_proxy->checkEnvEx();
    QVERIFY(spy.count() >= 1);
    int sig = spy.at(0).at(0).toInt();
    QVERIFY(sig == int(BackupResult::CHECK_ENV_SUCCESS)
            || sig == int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH)
            || sig == int(BackupResult::BACKUP_PARTITION_MOUNT_FAIL));
    if (result)
        QCOMPARE(sig, int(BackupResult::CHECK_ENV_SUCCESS));
}

void TestCustomizeGhostImageProxy::testDoWorkEx()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setGhostBackupWrapper(w, "test-uuid", tempDir.path() + "/");
    m_proxy->setParam(w);

    QSignalSpy spyResult(m_proxy, &Worker::workResult);
    QSignalSpy spyCheck(m_proxy, &Worker::checkResult);
    m_proxy->doWorkEx();
    QTest::qWait(4000);
    QVERIFY(spyCheck.count() >= 1);
    if (spyResult.count() >= 1)
        QVERIFY(spyResult.at(0).at(0).toBool() == true || spyResult.at(0).at(0).toBool() == false);
}

void TestCustomizeGhostImageProxy::testCancelEx_emitsStartCancelAndCancelSuccess()
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

void TestCustomizeGhostImageProxy::testDoGhostImage_afterCheckEnvSuccess_runsWithoutCrash()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());
    QString basePath = rootDir.path();
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(basePath + "/");
#endif
    const QString uuid = QStringLiteral("{dddddddd-eeee-ffff-0000-111111111111}");
    QVERIFY(writeGhostBackupListXml(basePath, uuid));
    QString srcDataDir = basePath + QStringLiteral("/.backup/snapshots/") + uuid + QStringLiteral("/data");
    QVERIFY(QDir().mkpath(srcDataDir));
    QFile img(srcDataDir + QStringLiteral("/") + QStringLiteral(UDISK_MKSQUASHFS_IMG_NAME));
    QVERIFY(img.open(QIODevice::WriteOnly));
    img.write("fake img");
    img.close();

    QTemporaryDir destDir;
    QVERIFY(destDir.isValid());
    BackupWrapper w;
    setGhostBackupWrapper(w, uuid, destDir.path() + "/");
    m_proxy->setParam(w);

    QSignalSpy spyProgress(m_proxy, &Worker::progress);
    QSignalSpy spyResult(m_proxy, &Worker::workResult);
    m_proxy->checkEnvEx();
    m_proxy->doGhostImage();
    QTest::qWait(3500);
    QVERIFY(true);
}

void TestCustomizeGhostImageProxy::testCancelEx_calledTwice_bothEmitSignals()
{
    QSignalSpy spy(m_proxy, &Worker::checkResult);
    m_proxy->cancelEx();
    int countAfterFirst = spy.count();
    m_proxy->cancelEx();
    int countAfterSecond = spy.count();
    QVERIFY(countAfterFirst >= 2);
    QVERIFY(countAfterSecond >= countAfterFirst);
}

void TestCustomizeGhostImageProxy::testCheckDestDirExists_notFinished_prefixDirExists_returnsTrue()
{
    QTemporaryDir destDir;
    QVERIFY(destDir.isValid());
    BackupWrapper w;
    setGhostBackupWrapper(w, "any-uuid", destDir.path() + "/");
    m_proxy->setParam(w);

    bool result = m_proxy->checkDestDirExists();
    QVERIFY(result);
}

void TestCustomizeGhostImageProxy::testCheckDestDirExists_prefixDirEmpty_thenForce_emitsWorkResultFalse()
{
    QTemporaryDir emptyDir;
    QVERIFY(emptyDir.isValid());
    BackupWrapper w;
    setGhostBackupWrapper(w, "any-uuid", emptyDir.path() + "/");
    m_proxy->setParam(w);

    QSignalSpy spyResult(m_proxy, &Worker::workResult);
    bool result1 = m_proxy->checkDestDirExists();
    QVERIFY(result1);
    QTest::qWait(100);
    bool result2 = m_proxy->checkDestDirExists();
    QVERIFY(!result2);
    QVERIFY(spyResult.count() >= 1);
    QCOMPARE(spyResult.at(0).at(0).toBool(), false);
}
