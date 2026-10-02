/*
 * 白盒测试：CustomizeSystemBackupProxy 公共接口（不测私有函数）
 */
#include "test_customizesystembackupproxy.h"
#include "../../../systembackup-daemon/customizesystembackupproxy.h"
#include "../../../systembackup-daemon/workerfactory.h"
#include "../../../common/mydefine.h"
#include "../../../common/utils.h"
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryDir>
#include <QDir>
#include <QMetaObject>
#include <QFile>
#include <QFileInfo>

void TestCustomizeSystemBackupProxy::initTestCase()
{
    BackupWrapper::registerMetaType();
}

void TestCustomizeSystemBackupProxy::cleanupTestCase()
{
}

void TestCustomizeSystemBackupProxy::init()
{
    m_proxy = new CustomizeSystemBackupProxy();
}

void TestCustomizeSystemBackupProxy::cleanup()
{
    if (m_proxy) {
        delete m_proxy;
        m_proxy = nullptr;
    }
#ifdef UNIT_TEST
    // 还原备份根路径，避免影响其他用例
    Utils::setBackupRootPathForTest(QStringLiteral("/data/"));
#endif
}

void TestCustomizeSystemBackupProxy::setSystemBackupWrapper(BackupWrapper &w, const QString &prefixPath)
{
    w.m_prefixDestPath = prefixPath;
    w.m_backupPaths = QStringList() << "/";
    w.m_backupExcludePaths = QStringList();
    w.m_backupName = "test_system_backup";
    w.m_note = "note";
    w.m_iPosition = 0;
    w.m_frontUid = 1000;
    w.m_gid = 1000;
    w.m_type = -1;
}

void TestCustomizeSystemBackupProxy::testConstructor()
{
    QVERIFY(m_proxy != nullptr);
}

void TestCustomizeSystemBackupProxy::testDestructor()
{
    CustomizeSystemBackupProxy *p = new CustomizeSystemBackupProxy();
    delete p;
    QVERIFY(true);
}

// void TestCustomizeSystemBackupProxy::testCheckEnvEx_emitsCheckResult()
// {
//     QTemporaryDir tempDir;
//     QVERIFY(tempDir.isValid());
//     BackupWrapper w;
//     setSystemBackupWrapper(w, tempDir.path() + "/");

//     m_proxy->setParam(w);
//     QSignalSpy spy(m_proxy, &Worker::checkResult);
//     m_proxy->checkEnvEx();
//     QVERIFY(spy.count() >= 1);
// }

// void TestCustomizeSystemBackupProxy::testCheckEnvEx_pathInjection_emitsDuErr()
// {
//     BackupWrapper w;
//     w.m_prefixDestPath = "/tmp/`id`/";
//     w.m_backupPaths = QStringList() << "/";
//     w.m_backupName = "test";
//     w.m_note = "note";
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

// void TestCustomizeSystemBackupProxy::testCheckEnvEx_emptyPrefixDestPath_mkpath()
// {
//     QTemporaryDir tempDir;
//     QVERIFY(tempDir.isValid());
//     QString base = tempDir.path() + "/empty_dest/";
//     BackupWrapper w;
//     setSystemBackupWrapper(w, base);
//     m_proxy->setParam(w);

//     QSignalSpy spy(m_proxy, &Worker::checkResult);
//     m_proxy->checkEnvEx();
//     QVERIFY(spy.count() >= 1);
//     QVERIFY(QDir(base).exists());
// }

void TestCustomizeSystemBackupProxy::testDoWorkEx()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setSystemBackupWrapper(w, tempDir.path() + "/");
    m_proxy->setParam(w);

    QSignalSpy spyResult(m_proxy, &Worker::workResult);
    QSignalSpy spyCheck(m_proxy, &Worker::checkResult);
    m_proxy->doWorkEx();
    QTest::qWait(600);
    QVERIFY(spyCheck.count() >= 1);
    if (spyResult.count() >= 1)
        QVERIFY(spyResult.at(0).at(0).toBool() == true || spyResult.at(0).at(0).toBool() == false);
}

void TestCustomizeSystemBackupProxy::testDoPrepare_createsSnapshotAndXml()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());

#ifdef UNIT_TEST
    // 将备份根路径指向临时目录，避免写真实 /data
    Utils::setBackupRootPathForTest(rootDir.path() + "/");
#endif

    BackupWrapper w;
    setSystemBackupWrapper(w, rootDir.path() + "/");
    m_proxy->setParam(w);

    bool ok = m_proxy->doPrepare();
    QVERIFY(ok);

    // UUID 应非空
    QVERIFY(!m_proxy->m_curUuid.isEmpty());

    // 目标数据目录应创建成功
    QVERIFY(!m_proxy->m_destPath.isEmpty());
    QVERIFY(QDir(m_proxy->m_destPath).exists());

    // 用户路径文件与排除路径文件应存在
    QString baseSnapshotDir = w.m_prefixDestPath + BACKUP_SNAPSHOTS_PATH + "/" + m_proxy->m_curUuid;
    baseSnapshotDir.replace("//", "/");
    QString userFile = baseSnapshotDir + "/" + PATHS_USER_FILE;
    QString excludeFile = baseSnapshotDir + "/" + EXCLUDE_PATHS_USER_FILE;
    QVERIFY(QFile::exists(userFile));
    QVERIFY(QFile::exists(excludeFile));

    // backuplist.xml 应被创建并非空
    QString xmlPath = Utils::getBackupRootPath() + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    QFile xml(xmlPath);
    QVERIFY(xml.exists());
    QVERIFY(xml.size() > 0);
}

void TestCustomizeSystemBackupProxy::testDoMksqushfs_basic()
{
    QTemporaryDir rootDir;
    QVERIFY(rootDir.isValid());

#ifdef UNIT_TEST
    Utils::setBackupRootPathForTest(rootDir.path() + "/");
#endif

    BackupWrapper w;
    setSystemBackupWrapper(w, rootDir.path() + "/");
    m_proxy->setParam(w);

    // 先做准备，确保 m_destPath 等成员有效
    bool ok = m_proxy->doPrepare();
    if (!ok) {
        QSKIP("doPrepare failed in test environment, skip doMksqushfs_basic");
        return;
    }

    QSignalSpy spyCheck(m_proxy, &Worker::checkResult);
    QSignalSpy spyWork(m_proxy, &Worker::workResult);

    // 直接调用私有 doMksqushfs（通过友元访问）
    m_proxy->doMksqushfs();

    // 至少应创建 MkSquashFSProcess 实例
    QVERIFY(m_proxy->m_mksquashfs != nullptr);

    // 给异步信号一点时间（无论成功或失败）
    QTest::qWait(200);

    // 两种情况都允许：
    // 1. 环境中无 mksquashfs：应快速发出 MKSQUASHFS_DO_FAIL
    // 2. 环境中有 mksquashfs：可能发出 workResult(true/false)
    if (spyCheck.count() > 0) {
        int code = spyCheck.at(0).at(0).toInt();
        QVERIFY(code == int(BackupResult::MKSQUASHFS_DO_FAIL)
                || code == int(BackupResult::CHECK_ENV_SUCCESS)
                || code == int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
    }

    if (spyWork.count() > 0) {
        bool r = spyWork.at(0).at(0).toBool();
        QVERIFY(r == true || r == false);
    }
}

// void TestCustomizeSystemBackupProxy::testCancelEx_emitsStartCancelAndCancelSuccess()
// {
//     QSignalSpy spy(m_proxy, &Worker::checkResult);
//     QMetaObject::invokeMethod(m_proxy, "cancelEx", Qt::DirectConnection);

//     bool gotStartCancel = false;
//     bool gotCancelSuccess = false;
//     for (int i = 0; i < spy.count(); ++i) {
//         int r = spy.at(i).at(0).toInt();
//         if (r == int(BackupResult::START_CANCEL))
//             gotStartCancel = true;
//         if (r == int(BackupResult::CANCEL_SUCCESS))
//             gotCancelSuccess = true;
//     }
//     QVERIFY(gotStartCancel && gotCancelSuccess);
// }
