#include "test_ostreeinterface.h"
#include "OstreeInterface.h"
#include <QTemporaryDir>
#include <QDir>
#include <QFile>

void TestOstreeInterface::initTestCase()
{
}

void TestOstreeInterface::cleanupTestCase()
{
}

void TestOstreeInterface::init()
{
}

void TestOstreeInterface::cleanup()
{
}

void TestOstreeInterface::testGetRepoPath_emptyBackupPath()
{
    OstreeInterface iface(nullptr);
    QString path = iface.getRepoPath("", OstreeInterface::BackupType::SYSTEM);
    QVERIFY(path.contains("repo_system"));
    QVERIFY(path.startsWith(DEFAULT_BACKUP_PATH) || path.contains(DEFAULT_BACKUP_PATH));
    path = iface.getRepoPath("", OstreeInterface::BackupType::SYSTEM_UPDATE);
    QVERIFY(path.contains("repo_system_update"));
    path = iface.getRepoPath("", OstreeInterface::BackupType::APPLICATION);
    QVERIFY(path.contains("repo_application"));
    path = iface.getRepoPath("", OstreeInterface::BackupType::USER);
    QVERIFY(path.contains("/repo_user"));
    path = iface.getRepoPath("", OstreeInterface::BackupType::FACTORY);
    QVERIFY(path.contains("repo_factory"));
}

void TestOstreeInterface::testGetRepoPath_allBackupTypes()
{
    OstreeInterface iface(nullptr);
    QString base = "/tmp/backup";
    QCOMPARE(iface.getRepoPath(base, OstreeInterface::BackupType::SYSTEM_UPDATE),
             "'/tmp/backuprepo_system_update'");
    QCOMPARE(iface.getRepoPath(base, OstreeInterface::BackupType::SYSTEM),
             "'/tmp/backuprepo_system'");
    QCOMPARE(iface.getRepoPath(base, OstreeInterface::BackupType::APPLICATION),
             "'/tmp/backuprepo_application'");
    QCOMPARE(iface.getRepoPath(base, OstreeInterface::BackupType::USER),
             "'/tmp/backup/repo_user'");
    QCOMPARE(iface.getRepoPath(base, OstreeInterface::BackupType::FACTORY),
             "'/tmp/backuprepo_factory'");
}

void TestOstreeInterface::testGetRepoPath_withQuotes()
{
    OstreeInterface iface(nullptr);
    QString path = iface.getRepoPath("", OstreeInterface::BackupType::SYSTEM);
    QVERIFY(!path.startsWith("'"));
}

void TestOstreeInterface::testCheckOstreeRepo_existAndNotExist()
{
    OstreeInterface iface(nullptr);
    QVERIFY(!iface.checkOstreeRepo("", OstreeInterface::BackupType::SYSTEM));

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    QString repoPath = base + "repo_system";
    QDir().mkpath(repoPath);
    bool exist = iface.checkOstreeRepo(base, OstreeInterface::BackupType::SYSTEM);
    QVERIFY(exist);
}

void TestOstreeInterface::testCheckOstreeRepo_withQuotes()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    QDir().mkpath(base + "repo_system");
    QString quoted = "'" + base + "'";
    OstreeInterface iface(nullptr);
    bool exist = iface.checkOstreeRepo(quoted, OstreeInterface::BackupType::SYSTEM);
    QVERIFY(exist);
}

void TestOstreeInterface::testInitOstreeRepo()
{
    OstreeInterface iface(nullptr);
    QVERIFY(iface.initOstreeRepo("", OstreeInterface::BackupType::SYSTEM));
}

void TestOstreeInterface::testGetSystemHashArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getSystemHashArgs();
    QVERIFY(args.contains("admin"));
    QVERIFY(args.contains("status"));
    QVERIFY(args.contains("--skip-signatures"));
}

void TestOstreeInterface::testGetPendingHashArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getPendingHashArgs();
    QVERIFY(args.contains("admin"));
}

void TestOstreeInterface::testGetDeployArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getDeployArgs("abc123");
    QVERIFY(args.contains("admin"));
    QVERIFY(args.contains("deploy"));
    QVERIFY(args.contains("abc123"));
}

void TestOstreeInterface::testGetUndeployArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getUndeployArgs("0");
    QVERIFY(args.contains("admin"));
    QVERIFY(args.contains("undeploy"));
    QVERIFY(args.contains("0"));
}

void TestOstreeInterface::testGetNeedUndeployHashArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getNeedUndeployHashArgs();
    QVERIFY(args.contains("admin"));
    QVERIFY(args.contains("status"));
}

void TestOstreeInterface::testGetHashNumberArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getHashNumberArgs("hash1");
    QVERIFY(args.contains("admin"));
    QVERIFY(args.contains("hash1"));
}

void TestOstreeInterface::testGetSystemRollBackArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getSystemRollBackArgs("1");
    QVERIFY(args.contains("admin"));
    QVERIFY(args.contains("set-default"));
    QVERIFY(args.contains("1"));
}

void TestOstreeInterface::testGetCommitArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getCommitArgs("/repo", "branch1", "/path", "desc");
    QVERIFY(args.contains("--repo=/repo"));
    QVERIFY(args.contains("commit"));
    QVERIFY(args.contains("-b"));
    QVERIFY(args.contains("branch1"));
    QVERIFY(args.contains("--skip-spec"));
    QVERIFY(args.contains("--tree=dir=/path"));
    QVERIFY(args.contains("-s"));
    QVERIFY(args.contains("desc"));
}

void TestOstreeInterface::testGetCheckoutArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getCheckoutArgs("/repo", "subdir", "hash1");
    QVERIFY(args.contains("--repo=/repo"));
    QVERIFY(args.contains("checkout"));
    QVERIFY(args.contains("hash1"));
    QVERIFY(args.last().startsWith(QString(TEMP_RESTORE_PATH)));
}

void TestOstreeInterface::testGetCheckoutFullSyncArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getCheckoutFullSyncArgs("/repo", "/dest", "hash1");
    QVERIFY(args.contains("--repo=/repo"));
    QVERIFY(args.contains("checkout"));
    QVERIFY(args.contains("--process-passthrough-whiteouts"));
    QVERIFY(args.contains("--force-copy"));
    QVERIFY(args.contains("hash1"));
    QVERIFY(args.contains("/dest"));
    QVERIFY(args.contains("--full-sync"));
}

void TestOstreeInterface::testGetRefsArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getRefsArgs("/repo", "branch1");
    QVERIFY(args.contains("--repo=/repo"));
    QVERIFY(args.contains("refs"));
    QVERIFY(args.contains("--delete"));
    QVERIFY(args.contains("branch1"));
}

void TestOstreeInterface::testGetPruneArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getPruneArgs("/repo", "commit-id-1");
    QVERIFY(args.contains("--repo=/repo"));
    QVERIFY(args.contains("prune"));
    QVERIFY(args.contains("--delete-commit=commit-id-1"));
}

void TestOstreeInterface::testGetRsyncRepoArgs_allScenes()
{
    OstreeInterface iface(nullptr);
    iface.setRsyncScense(OstreeInterface::RESTORE_SYSTEM);
    QStringList args1 = iface.getRsyncRepoArgs("/src", "/dst");
    QVERIFY(!args1.isEmpty());
    QVERIFY(args1.contains("-avAHXr"));
    QVERIFY(args1.last() == "/dst/");

    iface.setRsyncScense(OstreeInterface::RESTORE_HOMEDATA);
    QStringList args2 = iface.getRsyncRepoArgs("/a", "/b");
    QVERIFY(args2.contains("--include=*/.*"));

    iface.setRsyncScense(OstreeInterface::RESTORE_HOMEHIDDENDATA);
    QStringList args3 = iface.getRsyncRepoArgs("/a", "/b");
    QVERIFY(args3.contains("--exclude=.box/"));

    iface.setRsyncScense(OstreeInterface::RESTORE_SYSTEM_SAMESYSTEM);
    QStringList args4 = iface.getRsyncRepoArgs("/a", "/b");
    QVERIFY(args4.contains("--exclude=passwd"));
}

void TestOstreeInterface::testGetCpArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getCpArgs("/src", "/dst");
    QVERIFY(args.contains("-aL"));
    QVERIFY(args.contains("-P"));
    QVERIFY(args.contains("-rf"));
    QVERIFY(args.contains("/src"));
    QVERIFY(args.contains("/dst"));
}

void TestOstreeInterface::testGetRmArgs()
{
    OstreeInterface iface(nullptr);
    QStringList args = iface.getRmArgs("/path/to/rm");
    QVERIFY(args.contains("-rf"));
    QVERIFY(args.contains("/path/to/rm"));
}

void TestOstreeInterface::testGetFindArgs_allTypes()
{
    OstreeInterface iface(nullptr);
    QStringList args1 = iface.getFindArgs("/path", 1);
    QVERIFY(args1.contains("-type"));
    QVERIFY(args1.contains("s"));
    QStringList args2 = iface.getFindArgs("/path", 2);
    QVERIFY(args2.contains("c"));
    QStringList args3 = iface.getFindArgs("/path", 3);
    QVERIFY(args3.contains("b"));
    QStringList args0 = iface.getFindArgs("/path", 99);
    QVERIFY(args0.contains("s"));
}

void TestOstreeInterface::testSetGetOperationPath()
{
    OstreeInterface iface(nullptr);
    iface.setOperationPath("/my/path");
}

void TestOstreeInterface::testSetGetBackUpStep()
{
    OstreeInterface iface(nullptr);
    iface.setBackUpStep("ovl");
    QCOMPARE(iface.getBackUpStep(), QString("ovl"));
    iface.setBackUpStep("home");
    QCOMPARE(iface.getBackUpStep(), QString("home"));
}

void TestOstreeInterface::testSetGetOperation()
{
    OstreeInterface iface(nullptr);
    iface.setOperation(OstreeInterface::INIT);
    QCOMPARE(iface.getOperation(), OstreeInterface::Operation::INIT);
    iface.setOperation(OstreeInterface::COMMIT);
    QCOMPARE(iface.getOperation(), OstreeInterface::Operation::COMMIT);
}

void TestOstreeInterface::testSetGetTaskType()
{
    OstreeInterface iface(nullptr);
    iface.setTaskType(OstreeInterface::BACKUP);
    QCOMPARE(iface.getTaskType(), OstreeInterface::TaskType::BACKUP);
    iface.setTaskType(OstreeInterface::RESTORE);
    QCOMPARE(iface.getTaskType(), OstreeInterface::TaskType::RESTORE);
}

void TestOstreeInterface::testSetGetRsyncScense()
{
    OstreeInterface iface(nullptr);
    iface.setRsyncScense(OstreeInterface::RESTORE_SYSTEM);
    QCOMPARE(iface.getRsyncScense(), OstreeInterface::RsyncScense::RESTORE_SYSTEM);
}

void TestOstreeInterface::testStop()
{
    OstreeInterface iface(nullptr);
    iface.stop();
}
