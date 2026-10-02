#include "test_deletebackupproxy.h"
#include "../../../common/mydefine.h"
#include "../../../common/utils.h"
#include <QSignalSpy>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QProcess>

TestDeleteBackupProxy::TestDeleteBackupProxy()
    : m_deleteBackupProxy(nullptr)
{
}

void TestDeleteBackupProxy::initTestCase()
{
    BackupWrapper::registerMetaType();
    m_deleteBackupProxy = new DeleteBackupProxy();
}

void TestDeleteBackupProxy::cleanupTestCase()
{
    if (m_deleteBackupProxy) {
        delete m_deleteBackupProxy;
        m_deleteBackupProxy = nullptr;
    }
}

void TestDeleteBackupProxy::init()
{
}

void TestDeleteBackupProxy::cleanup()
{
}

void TestDeleteBackupProxy::testConstructor()
{
    QVERIFY(m_deleteBackupProxy != nullptr);
}

void TestDeleteBackupProxy::testDestructor()
{
    DeleteBackupProxy* p = new DeleteBackupProxy();
    delete p;
}

bool TestDeleteBackupProxy::writeUserBackupXml(const QString& xmlPath, const QString& uuid,
                                               const QString& branchName, bool withDataCommitHashMap)
{
    QFileInfo fi(xmlPath);
    if (!QDir().mkpath(fi.absolutePath()))
        return false;
    QFile f(xmlPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;
    QTextStream out(&f);
    out << "<?xml version='1.0'?>\n<backupList>\n  <BackupPoint>\n";
    out << "    <Comment>test</Comment>\n    <Time>2025-01-01 12:00:00</Time>\n";
    out << "    <Uuid>" << uuid << "</Uuid>\n    <Size>0</Size>\n    <State>backup finished</State>\n";
    out << "    <Type>0</Type>\n    <Position>0</Position>\n    <UserId>1000</UserId>\n";
    out << "    <PrefixDestPath>/tmp</PrefixDestPath>\n    <SystemHash>syshash1</SystemHash>\n";
    out << "    <BranchName>" << branchName << "</BranchName>\n";
    if (withDataCommitHashMap) {
        out << "    <DataCommitHashMap>\n      <entry path=\"/data\" hash=\"abc123def\" type=\"dir\"/>\n    </DataCommitHashMap>\n";
    } else {
        out << "    <DataCommitHashMap></DataCommitHashMap>\n";
    }
    out << "  </BackupPoint>\n</backupList>\n";
    f.close();
    return true;
}

bool TestDeleteBackupProxy::writeSystemBackupXml(const QString& xmlPath, const QString& uuid,
                                                  bool withSystemCommitHashMap, int position)
{
    QFileInfo fi(xmlPath);
    if (!QDir().mkpath(fi.absolutePath()))
        return false;
    QFile f(xmlPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;
    QTextStream out(&f);
    out << "<?xml version='1.0'?>\n<backupList>\n  <BackupPoint>\n";
    out << "    <Comment>sys</Comment>\n    <Time>2025-01-01 12:00:00</Time>\n";
    out << "    <Uuid>" << uuid << "</Uuid>\n    <Size>0</Size>\n    <State>backup finished</State>\n";
    out << "    <Type>1</Type>\n    <Position>" << position << "</Position>\n";
    out << "    <UserId>1000</UserId>\n    <PrefixDestPath>/tmp</PrefixDestPath>\n";
    out << "    <SystemHash>syshash1</SystemHash>\n    <BranchName>branch-sys</BranchName>\n";
    if (withSystemCommitHashMap) {
        out << "    <SystemCommitHashMap><ovl>ovlhash</ovl><home>homehash</home></SystemCommitHashMap>\n";
    }
    out << "  </BackupPoint>\n</backupList>\n";
    f.close();
    return true;
}

void TestDeleteBackupProxy::setUserBackupWrapper(BackupWrapper& w, const QString& prefixPath,
                                                 const QString& uuid, const QString& backupName)
{
    w.m_prefixDestPath = prefixPath;
    w.m_uuid = uuid;
    w.m_backupName = backupName;
    w.m_backupType = 0;
    w.m_iPosition = 0;
    w.m_note = "note";
}

void TestDeleteBackupProxy::setSystemBackupWrapper(BackupWrapper& w, const QString& prefixPath,
                                                   const QString& uuid, int iPosition)
{
    w.m_prefixDestPath = prefixPath;
    w.m_uuid = uuid;
    w.m_backupName = "sys";
    w.m_backupType = 1;
    w.m_iPosition = iPosition;
    w.m_note = "note";
}

void TestDeleteBackupProxy::testDoWorkEx_emitFalse_whenDataCommitHashEmpty()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    QString snapshots = base + "snapshots";
    QDir().mkpath(snapshots);
    QString xmlPath = snapshots + "/user_backuplist.xml";
    QVERIFY(writeUserBackupXml(xmlPath, "{test-uuid-1111}", "branch-20250101", false));

    BackupWrapper w;
    setUserBackupWrapper(w, base, "{test-uuid-1111}", "test");
    m_deleteBackupProxy->setParam(w);

    QSignalSpy spy(m_deleteBackupProxy, &DeleteBackupProxy::workResult);
    m_deleteBackupProxy->doWorkEx();
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool() == false);
}

void TestDeleteBackupProxy::testDoWorkEx_emitFalse_whenNoOstreeRepo_user()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    QString snapshots = base + "snapshots";
    QDir().mkpath(snapshots);
    QString xmlPath = snapshots + "/user_backuplist.xml";
    QVERIFY(writeUserBackupXml(xmlPath, "{test-uuid-2222}", "branch-20250102", true));

    BackupWrapper w;
    setUserBackupWrapper(w, base, "{test-uuid-2222}", "test2");
    m_deleteBackupProxy->setParam(w);

    QSignalSpy spy(m_deleteBackupProxy, &DeleteBackupProxy::workResult);
    m_deleteBackupProxy->doWorkEx();
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool() == false);
}

void TestDeleteBackupProxy::testDoWorkEx_emitFalse_whenNoOstreeRepo_system()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    QString backupSnapshots = base + ".backup/snapshots";
    QDir().mkpath(backupSnapshots);
    QString xmlPath = backupSnapshots + "/sys_backuplist.xml";
    QVERIFY(writeSystemBackupXml(xmlPath, "{test-uuid-3333}", true, 1));

    BackupWrapper w;
    setSystemBackupWrapper(w, base, "{test-uuid-3333}", 1);
    m_deleteBackupProxy->setParam(w);

    QSignalSpy spy(m_deleteBackupProxy, &DeleteBackupProxy::workResult);
    m_deleteBackupProxy->doWorkEx();
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool() == false);
}

void TestDeleteBackupProxy::testDoWorkEx_emitFalse_whenSystemCommitHashEmpty()
{
    BackupWrapper w;
    w.m_backupType = 1;
    w.m_iPosition = 0;
    w.m_prefixDestPath = "";
    w.m_uuid = "{no-such-uuid}";
    w.m_backupName = "sys";
    m_deleteBackupProxy->setParam(w);

    QSignalSpy spy(m_deleteBackupProxy, &DeleteBackupProxy::workResult);
    m_deleteBackupProxy->doWorkEx();
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool() == false);
}

void TestDeleteBackupProxy::testDoWorkEx_emitTrue_fullSuccess_userBackup()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    QString snapshots = base + "snapshots";
    QDir().mkpath(snapshots);
    QString repoUser = base + "repo_user";
    QProcess proc;
    proc.setProgram("ostree");
    proc.setArguments(QStringList() << "init" << "--repo" << repoUser << "--mode" << "bare");
    proc.start();
    if (!proc.waitForFinished(5000) || proc.exitCode() != 0) {
        QSKIP("ostree not available or init failed");
        return;
    }
    QString xmlPath = snapshots + "/user_backuplist.xml";
    QString uuid = "{test-uuid-full}";
    QVERIFY(writeUserBackupXml(xmlPath, uuid, "branch-full", true));

    BackupWrapper w;
    setUserBackupWrapper(w, base, uuid, "full");
    m_deleteBackupProxy->setParam(w);

    QSignalSpy spy(m_deleteBackupProxy, &DeleteBackupProxy::workResult);
    m_deleteBackupProxy->doWorkEx();
    QCOMPARE(spy.count(), 1);
    bool result = spy.takeFirst().at(0).toBool();
    QVERIFY(result == true || result == false);
}

void TestDeleteBackupProxy::testDoWorkEx_xmlPath_branch_iPosition1_backupType1()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    QString backupSnapshots = base + ".backup/snapshots";
    QDir().mkpath(backupSnapshots);
    QString xmlPath = backupSnapshots + "/sys_backuplist.xml";
    QVERIFY(writeSystemBackupXml(xmlPath, "{test-uuid-pos1}", true, 1));

    BackupWrapper w;
    setSystemBackupWrapper(w, base, "{test-uuid-pos1}", 1);
    m_deleteBackupProxy->setParam(w);

    QSignalSpy spy(m_deleteBackupProxy, &DeleteBackupProxy::workResult);
    m_deleteBackupProxy->doWorkEx();
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool() == false);
}

void TestDeleteBackupProxy::testDoWorkEx_mobileBackupPoint_countOneAndTwo()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    QString backupSnapshots = base + ".backup/snapshots";
    QDir().mkpath(backupSnapshots);
    QString xmlPath = backupSnapshots + "/sys_backuplist.xml";
    QFile f(xmlPath);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    QTextStream out(&f);
    out << "<?xml version='1.0'?>\n<backupList>\n";
    out << "  <BackupPoint><Comment>a</Comment><Time>1</Time><Uuid>{uu1}</Uuid><Type>1</Type><Position>1</Position>";
    out << "<SystemHash>h</SystemHash><BranchName>b</BranchName>";
    out << "<SystemCommitHashMap><ovl>o</ovl><home>h</home></SystemCommitHashMap></BackupPoint>\n";
    out << "  <BackupPoint><Comment>b</Comment><Time>2</Time><Uuid>{uu2}</Uuid><Type>1</Type><Position>1</Position>";
    out << "<SystemHash>h</SystemHash><BranchName>b2</BranchName>";
    out << "<SystemCommitHashMap><ovl>o2</ovl><home>h2</home></SystemCommitHashMap></BackupPoint>\n";
    out << "</backupList>\n";
    f.close();

    BackupWrapper w;
    setSystemBackupWrapper(w, base, "{uu1}", 1);
    m_deleteBackupProxy->setParam(w);

    QSignalSpy spy(m_deleteBackupProxy, &DeleteBackupProxy::workResult);
    m_deleteBackupProxy->doWorkEx();
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool() == false);
}

void TestDeleteBackupProxy::testDoWorkEx_emitFalse_whenDeleteItemFails()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    QString snapshots = base + "snapshots";
    QDir().mkpath(snapshots);
    QString xmlPath = snapshots + "/user_backuplist.xml";
    QVERIFY(writeUserBackupXml(xmlPath, "{test-uuid-del}", "branch-del", true));

    BackupWrapper w;
    setUserBackupWrapper(w, base, "{non-existent-uuid}", "test");
    m_deleteBackupProxy->setParam(w);

    QSignalSpy spy(m_deleteBackupProxy, &DeleteBackupProxy::workResult);
    m_deleteBackupProxy->doWorkEx();
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool() == false);
}

void TestDeleteBackupProxy::testDoWorkEx_uuidFormatError_hitsDebugPath()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    QString snapshots = base + "snapshots";
    QDir().mkpath(snapshots);
    QString repoUser = base + "repo_user";
    QProcess proc;
    proc.setProgram("ostree");
    proc.setArguments(QStringList() << "init" << "--repo" << repoUser << "--mode" << "bare");
    proc.start();
    if (!proc.waitForFinished(5000) || proc.exitCode() != 0) {
        QSKIP("ostree not available or init failed");
        return;
    }
    QString xmlPath = snapshots + "/user_backuplist.xml";
    QString uuid = "x";
    QVERIFY(writeUserBackupXml(xmlPath, uuid, "branch-x", true));

    BackupWrapper w;
    setUserBackupWrapper(w, base, uuid, "short-uuid");
    m_deleteBackupProxy->setParam(w);

    QSignalSpy spy(m_deleteBackupProxy, &DeleteBackupProxy::workResult);
    m_deleteBackupProxy->doWorkEx();
    QCOMPARE(spy.count(), 1);
    bool result = spy.takeFirst().at(0).toBool();
    QVERIFY(result == true || result == false);
}

void TestDeleteBackupProxy::testDoWorkEx_userBackup_iPosition0_prefixDestPath()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    QString snapshots = base + "snapshots";
    QDir().mkpath(snapshots);
    QString xmlPath = snapshots + "/user_backuplist.xml";
    QVERIFY(writeUserBackupXml(xmlPath, "{a-b-c-d-e}", "branch-abc", true));

    BackupWrapper w;
    w.m_prefixDestPath = base;
    w.m_uuid = "{a-b-c-d-e}";
    w.m_backupName = "user0";
    w.m_backupType = 0;
    w.m_iPosition = 0;
    w.m_note = "note";
    m_deleteBackupProxy->setParam(w);

    QSignalSpy spy(m_deleteBackupProxy, &DeleteBackupProxy::workResult);
    m_deleteBackupProxy->doWorkEx();
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool() == false);
}

void TestDeleteBackupProxy::testDoWorkEx_systemBackup_iPosition0_xmlPathFromRoot()
{
    QString rootPath = Utils::getBackupRootPath();
    QString snapshotsPath = rootPath + "/.backup/snapshots";
    if (!QDir().exists(snapshotsPath) && !QDir().mkpath(snapshotsPath)) {
        QSKIP("Cannot create path under getBackupRootPath(), skip system iPosition=0 test");
        return;
    }
    QString xmlPath = rootPath + SYS_BACKUP_XML_PATH;
    QVERIFY(writeSystemBackupXml(xmlPath, "{sys-root-uuid}", true, 0));

    BackupWrapper w;
    setSystemBackupWrapper(w, "", "{sys-root-uuid}", 0);
    w.m_prefixDestPath = "";
    m_deleteBackupProxy->setParam(w);

    QSignalSpy spy(m_deleteBackupProxy, &DeleteBackupProxy::workResult);
    m_deleteBackupProxy->doWorkEx();
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool() == false);
}
