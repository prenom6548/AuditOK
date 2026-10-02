/*
 * 白盒测试：DataRestoreProxy（私有函数通过友元访问）
 */
#include "test_datarestoreproxy.h"
#include "../../../common/mydefine.h"
#include "../../../common/utils.h"
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

void TestDataRestoreProxy::initTestCase()
{
    BackupWrapper::registerMetaType();
}

void TestDataRestoreProxy::cleanupTestCase()
{
}

void TestDataRestoreProxy::init()
{
    m_proxy = new DataRestoreProxy();
}

void TestDataRestoreProxy::cleanup()
{
    if (m_proxy) {
        delete m_proxy;
        m_proxy = nullptr;
    }
#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(QStringLiteral("/data/"));
#endif
}

void TestDataRestoreProxy::setWrapper(BackupWrapper &w, const QString &prefixDestPath, const QString &uuid,
                                      int position, const QString &frontUser)
{
    w.m_type = int(BackupType::RESTORE_DATA);
    w.m_iPosition = position;
    w.m_prefixDestPath = prefixDestPath;
    w.m_uuid = uuid;
    w.m_frontUid = 1000;
    w.m_gid = 1000;
    w.m_frontUserName = frontUser;
    w.m_backupName = QStringLiteral("data-restore");
}

bool TestDataRestoreProxy::writeUserBackupXml(const QString &prefixDestPath, const QString &uuid,
                                             const QString &systemHash, bool withCommitMap)
{
    QString xmlPath = prefixDestPath + USER_XML; // /snapshots/user_backuplist.xml
    xmlPath.replace("//", "/");
    if (!QDir().mkpath(QFileInfo(xmlPath).absolutePath()))
        return false;

    QFile f(xmlPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;

    QTextStream out(&f);
    out << "<?xml version='1.0'?>\n<backupList>\n  <BackupPoint>\n";
    out << "    <Comment>user</Comment>\n    <Time>2025-01-01 12:00:00</Time>\n";
    out << "    <Uuid>" << uuid << "</Uuid>\n    <Size>0</Size>\n    <State>backup finished</State>\n";
    out << "    <Type>2</Type>\n    <Position>0</Position>\n    <UserId>1000</UserId>\n";
    out << "    <PrefixDestPath>" << prefixDestPath << "</PrefixDestPath>\n";
    out << "    <SystemHash>" << systemHash << "</SystemHash>\n";
    out << "    <BranchName>branch-test</BranchName>\n";
    if (withCommitMap) {
        out << "    <DataCommitHashMap>\n";
        out << "      <entry path=\"/data/testdir\" hash=\"abc123\" type=\"dir\"/>\n";
        out << "      <entry path=\"/data/testfile;\" hash=\"def456\" type=\"file\"/>\n";
        out << "    </DataCommitHashMap>\n";
    } else {
        out << "    <DataCommitHashMap></DataCommitHashMap>\n";
    }
    out << "  </BackupPoint>\n</backupList>\n";
    f.close();
    return true;
}

void TestDataRestoreProxy::ensureRepoUserDirExists(const QString &prefixDestPath)
{
    // checkEnvEx 里 checkOstreeRepo(prefixDestPath, USER) -> prefixDestPath + "/repo_user"
    QString repoUser = prefixDestPath;
    if (repoUser.endsWith(QLatin1Char('/')))
        repoUser.chop(1);
    repoUser += QStringLiteral("/repo_user");
    QDir().mkpath(repoUser);
}

void TestDataRestoreProxy::testConstructor()
{
    QVERIFY(m_proxy != nullptr);
}

void TestDataRestoreProxy::testDestructor()
{
    DataRestoreProxy *p = new DataRestoreProxy();
    delete p;
    QVERIFY(true);
}

void TestDataRestoreProxy::testCheckEnvEx_backupPointMissing_emitsIncNotFoundDir()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString prefix = dir.path() + "/";

    BackupWrapper w;
    setWrapper(w, prefix, QStringLiteral("{missing-uuid}"));
    m_proxy->setParam(w);

    // 写入一个不同 uuid 的节点
    QVERIFY(writeUserBackupXml(prefix, QStringLiteral("{other-uuid}"), QStringLiteral("syshash"), true));

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    QVERIFY(!m_proxy->checkEnvEx());
    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy.at(0).at(0).toInt(), int(BackupResult::INC_NOT_FOUND_DIR));
}

void TestDataRestoreProxy::testCheckEnvEx_commitHashMissing_emitsCheckCommitHashFail()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString prefix = dir.path() + "/";

    const QString uuid = QStringLiteral("{uuid-no-commitmap}");
    BackupWrapper w;
    setWrapper(w, prefix, uuid);
    m_proxy->setParam(w);
    QVERIFY(writeUserBackupXml(prefix, uuid, QStringLiteral("syshash"), false));

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    QVERIFY(!m_proxy->checkEnvEx());
    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy.at(0).at(0).toInt(), int(BackupResult::CHECK_COMMITHASH_FAIL));
}

void TestDataRestoreProxy::testCheckEnvEx_repoMissing_returnsFalse()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString prefix = dir.path() + "/";

    const QString uuid = QStringLiteral("{uuid-repo-missing}");
    BackupWrapper w;
    setWrapper(w, prefix, uuid);
    m_proxy->setParam(w);
    QVERIFY(writeUserBackupXml(prefix, uuid, QStringLiteral("syshash"), true));

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    QVERIFY(!m_proxy->checkEnvEx());
    // 该分支直接 return false，不保证发信号；只确保不会发 CHECK_ENV_SUCCESS
    for (int i = 0; i < spy.count(); ++i) {
        QVERIFY(spy.at(i).at(0).toInt() != int(BackupResult::CHECK_ENV_SUCCESS));
    }
}

void TestDataRestoreProxy::testCheckEnvEx_success_emitsCheckEnvSuccess()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString prefix = dir.path() + "/";

    const QString uuid = QStringLiteral("{uuid-success}");
    BackupWrapper w;
    setWrapper(w, prefix, uuid);
    m_proxy->setParam(w);
    QVERIFY(writeUserBackupXml(prefix, uuid, QStringLiteral("syshash"), true));
    ensureRepoUserDirExists(prefix);

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    QVERIFY(m_proxy->checkEnvEx());
    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy.at(0).at(0).toInt(), int(BackupResult::CHECK_ENV_SUCCESS));
}

void TestDataRestoreProxy::testDoWorkEx_whenCheckEnvFails_noWorkResult()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString prefix = dir.path() + "/";

    BackupWrapper w;
    setWrapper(w, prefix, QStringLiteral("{missing-uuid-2}"));
    m_proxy->setParam(w);
    // 不写 xml，checkEnvEx 会找不到备份点

    QSignalSpy spyWork(m_proxy, &Worker::workResult);
    m_proxy->doWorkEx();
    QCOMPARE(spyWork.count(), 0);
}

void TestDataRestoreProxy::testGetRsyncArgs_factoryAndAutoUuid_branches()
{
    // 通过友元访问 protected 成员，构造必要文件路径，避免 getRsyncArgs 组装时使用空路径
    m_proxy->m_userFile = QStringLiteral("/tmp/.user.txt");
    m_proxy->m_excludeUserFile = QStringLiteral("/tmp/.exclude.user.txt");
    m_proxy->m_extraUserFile = QStringLiteral("/tmp/.extra.user.txt");

    // FACTORY_BACKUP_UUID 分支
    m_proxy->m_curUuid = QStringLiteral(FACTORY_BACKUP_UUID);
    QStringList a1 = m_proxy->getRsyncArgs(DataRestoreProxy::RESTORE_SYSTEM_WITH_DATA);
    QVERIFY(!a1.isEmpty());

    // AUTO_BACKUP_UUID 分支（额外排除 remain）
    m_proxy->m_curUuid = QStringLiteral(AUTO_BACKUP_UUID);
    QStringList a2 = m_proxy->getRsyncArgs(DataRestoreProxy::RESTORE_SYSTEM_WITH_DATA);
    QVERIFY(!a2.isEmpty());

    // SYSTEM_RESTORE（fallthrough 会包含 SYSTEM_RESTORE 的排除项）
    QStringList a3 = m_proxy->getRsyncArgs(DataRestoreProxy::SYSTEM_RESTORE);
    QVERIFY(!a3.isEmpty());

    // RESTORE_EXTRA_DATA（使用 files-from）
    QStringList a4 = m_proxy->getRsyncArgs(DataRestoreProxy::RESTORE_EXTRA_DATA);
    QVERIFY(!a4.isEmpty());
}

void TestDataRestoreProxy::testGenerate16DigitRandomNumber_is16Digits()
{
    QString s = m_proxy->generate16DigitRandomNumber();
    QCOMPARE(s.size(), 16);
    QRegularExpression re(QStringLiteral("^[0-9]{16}$"));
    QVERIFY(re.match(s).hasMatch());
}

void TestDataRestoreProxy::testUpdateProgress_emitsPercent()
{
    m_proxy->m_totalItems = 10;
    QSignalSpy spy(m_proxy, &Worker::progress);
    m_proxy->updateProgress(5);
    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 50);
}

void TestDataRestoreProxy::testClearDirectory_clearsFiles()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString p = dir.path();
    QFile f(p + QStringLiteral("/a.txt"));
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();
    QVERIFY(QFile::exists(f.fileName()));

    m_proxy->clearDirectory(p);
    QDir d(p);
    QVERIFY(d.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty());
}

void TestDataRestoreProxy::testRsyncCopy_nonexistentSource_returnsFalse()
{
    QVERIFY(!m_proxy->rsyncCopy(QStringLiteral("/path/not/exist"), QStringLiteral("/tmp/"), true));
}

void TestDataRestoreProxy::testOstreeCheckout_invalidRepo_returnsFalse()
{
    // repoPath 为空会导致参数 repo 为空，通常会失败；验证返回 false
    m_proxy->repoPath = QStringLiteral("'/tmp/not-a-repo/repo_user'");
    QVERIFY(!m_proxy->ostreeCheckout(QStringLiteral("not-a-commit"), QStringLiteral("/tmp/checkout-target")));
}

void TestDataRestoreProxy::testMountImg_nonexistentImg_emitsRestoreDirPrepareFailed()
{
#ifdef UNIT_TEST
    QTemporaryDir root;
    QVERIFY(root.isValid());
    Utils::setBackupRootPathForTest(root.path() + "/");
#endif
    m_proxy->m_imgFileName = QStringLiteral("/path/not/exist/dst.img");
    QSignalSpy spy(m_proxy, &Worker::checkResult);
    QVERIFY(!m_proxy->mountImg());
    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy.at(0).at(0).toInt(), int(BackupResult::RESTOREDIR_PREPARE_FAILED));
}

void TestDataRestoreProxy::testRestoreExtraData_emptyOrMissing_returnsEarly()
{
#ifdef UNIT_TEST
    QTemporaryDir root;
    QVERIFY(root.isValid());
    Utils::setBackupRootPathForTest(root.path() + "/");
    // 额外目录不存在/为空，直接 return
    m_proxy->m_backupPoint.m_path.clear();
    m_proxy->m_curUuid = QStringLiteral("{uuid-extra}");
    m_proxy->restoreExtraData();
    QVERIFY(true);
#else
    QSKIP("UNIT_TEST not defined");
#endif
}

void TestDataRestoreProxy::testLinkFile_nonexistentSrc_returnsMinus1()
{
    int r = m_proxy->linkFile(QStringLiteral("/path/not/exist"), QStringLiteral("relative/desc"));
    QCOMPARE(r, -1);
}
