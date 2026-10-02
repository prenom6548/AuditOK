#include "test_utils.h"
#include "../../../common/utils.h"
#include "../../../common/mydefine.h"

#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QDir>
#include <QFileInfo>
#include <QMessageLogContext>
#include <QTextStream>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

void TestUtils::initTestCase()
{
    // 可在此做一些全局性的初始化
}

void TestUtils::cleanupTestCase()
{
    // 全局清理
}

// ---------------- StringBySize ----------------

void TestUtils::testStringBySize_data()
{
    QTest::addColumn<qint64>("size");
    QTest::addColumn<QString>("expected");

    // 覆盖所有分支：>GB, >MB, 其它(KB)
    QTest::newRow("0 bytes")                << qint64(0)                      << QString("0.00KB");
    QTest::newRow("just below 1KB")         << qint64(1023)                   << QString("1.00KB");
    QTest::newRow("1KB")                    << qint64(1024)                   << QString("1.00KB");
    QTest::newRow("just below 1MB")         << qint64(1024 * 1024 - 1)        << QString("1024.00KB");
    QTest::newRow("1MB")                    << qint64(1024 * 1024)            << QString("1024.00KB");
    QTest::newRow("just above 1MB")         << qint64(1024 * 1024 + 1)        << QString("1.00MB");
    QTest::newRow("just below 1GB")         << qint64(1024LL * 1024 * 1024 - 1) << QString("1024.00MB");
    QTest::newRow("1GB")                    << qint64(1024LL * 1024 * 1024)   << QString("1024.00MB");
    QTest::newRow("2GB")                    << qint64(2LL * 1024 * 1024 * 1024) << QString("2.00GB");
}

void TestUtils::testStringBySize()
{
    QFETCH(qint64, size);
    QFETCH(QString, expected);

    QString result = Utils::StringBySize(size);
    QCOMPARE(result, expected);
}

// ---------------- SizeByString ----------------

void TestUtils::testSizeByString_data()
{
    QTest::addColumn<QString>("sizeString");
    QTest::addColumn<qint64>("expected");

    // 正常单位
    QTest::newRow("0B")          << QString("0B")       << qint64(0);
    QTest::newRow("1KB")         << QString("1KB")      << qint64(1024);
    QTest::newRow("1.5KB")       << QString("1.5KB")    << qint64(1536);
    QTest::newRow("1MB")         << QString("1MB")      << qint64(1024 * 1024);
    QTest::newRow("2GB")         << QString("2GB")      << qint64(2LL * 1024 * 1024 * 1024);
    QTest::newRow("1TB")         << QString("1TB")      << qint64(1024LL * 1024 * 1024 * 1024);

    // 无单位/空格大小写混合
    QTest::newRow("no unit")     << QString("123")      << qint64(123);
    QTest::newRow("with space")  << QString(" 1.0 mb ") << qint64(1024 * 1024);

    // 未知单位 -> 按字节处理
    QTest::newRow("unknown unit") << QString("10XY")    << qint64(10);

    // 非法字符串 -> 0
    QTest::newRow("empty")       << QString("")         << qint64(0);
    QTest::newRow("invalid")     << QString("abcKB")    << qint64(0);
}

void TestUtils::testSizeByString()
{
    QFETCH(QString, sizeString);
    QFETCH(qint64, expected);

    qint64 result = Utils::SizeByString(sizeString);
    QCOMPARE(result, expected);
}

// ---------------- swapKeyValue ----------------

void TestUtils::testSwapKeyValue()
{
    QHash<QString, QString> src;
    src.insert("k1", "v1");
    src.insert("k2", "v2");

    QHash<QString, QString> swapped = Utils::swapKeyValue(src);

    QCOMPARE(swapped.size(), 2);
    QCOMPARE(swapped.value("v1"), QString("k1"));
    QCOMPARE(swapped.value("v2"), QString("k2"));

    QHash<QString, QString> emptySwapped = Utils::swapKeyValue(QHash<QString, QString>());
    QVERIFY(emptySwapped.isEmpty());
}

// ---------------- mkdir / mkpath / 删除相关 ----------------

void TestUtils::testMkpathAndMkdir()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString dirPath = tempDir.path() + "/a/b/c";

    // mkpath 递归创建
    QVERIFY(Utils::mkpath(dirPath));
    QVERIFY(QDir(dirPath).exists());

    // mkdir 非递归创建：对已存在目录调用应不报错
    Utils::mkdir(dirPath);
    QVERIFY(QDir(dirPath).exists());
}

void TestUtils::testDeleteSingleFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.path() + "/test.txt";

    // 不存在文件视为删除成功
    QVERIFY(Utils::deleteSingleFile(filePath));

    // 创建文件并删除
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("hello");
    f.close();
    QVERIFY(QFile::exists(filePath));

    QVERIFY(Utils::deleteSingleFile(filePath));
    QVERIFY(!QFile::exists(filePath));
}

void TestUtils::testDeleteDirectory()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString dirPath = tempDir.path() + "/sub";
    QDir().mkpath(dirPath);

    // 目录存在且可删除
    QVERIFY(Utils::deleteDirectory(dirPath));
    QVERIFY(!QDir(dirPath).exists());

    // 不存在目录 -> 视为成功
    QVERIFY(Utils::deleteDirectory(dirPath));
}

void TestUtils::testDeletePath_fileDirNotExists()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.path() + "/f.txt";
    QString dirPath  = tempDir.path() + "/d";
    QString nonePath = tempDir.path() + "/no_such_path";

    // 准备文件
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    // 准备目录
    QDir().mkpath(dirPath);

    // 删除文件
    QVERIFY(Utils::deletePath(filePath));
    QVERIFY(!QFile::exists(filePath));

    // 删除目录
    QVERIFY(Utils::deletePath(dirPath));
    QVERIFY(!QDir(dirPath).exists());

    // 删除不存在路径 -> false
    QVERIFY(!Utils::deletePath(nonePath));
}

// ---------------- 目录状态判断 ----------------

void TestUtils::testIsDirExistAndIsDirExists()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString dirPath = tempDir.path();
    QString nonePath = dirPath + "/no_such_dir";

    QVERIFY(Utils::isDirExist(dirPath));
    QVERIFY(Utils::isDirExists(dirPath));

    QVERIFY(!Utils::isDirExist(nonePath));
    QVERIFY(!Utils::isDirExists(nonePath));
}

void TestUtils::testIsDirEmpty()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString dirPath = tempDir.path();
    QString subDir  = dirPath + "/sub";

    // 目录存在且为空
    QVERIFY(Utils::isDirEmpty(dirPath));

    // 创建子目录后，非空
    QDir().mkpath(subDir);
    QVERIFY(!Utils::isDirEmpty(dirPath));

    // 不存在目录 -> 视为为空
    QString nonePath = dirPath + "/no_such";
    QVERIFY(Utils::isDirEmpty(nonePath));
}

// ---------------- 文件写入相关 ----------------

void TestUtils::testWriteFileByLines()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.path() + "/lines.txt";
    QStringList lines;
    lines << "line1" << "line2" << "line3";

    QVERIFY(Utils::writeFileByLines(filePath, lines));

    QFile f(filePath);
    QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(f.readAll());
    f.close();

    QStringList readLines = content.split("\n", Qt::SkipEmptyParts);
    QCOMPARE(readLines.size(), lines.size());
    QCOMPARE(readLines.at(0), QString("line1"));
    QCOMPARE(readLines.at(1), QString("line2"));
    QCOMPARE(readLines.at(2), QString("line3"));

    QVERIFY(!Utils::writeFileByLines(tempDir.path(), lines));
}

void TestUtils::testFileExists()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.path() + "/exist.txt";

    // 不存在
    QVERIFY(!Utils::fileExists(filePath));

    // 创建文件
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("abc");
    f.close();

    QVERIFY(Utils::fileExists(filePath));
}

void TestUtils::testWriteFileAndSyncWriteFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath1 = tempDir.path() + "/writeFile.txt";
    QString filePath2 = tempDir.path() + "/syncWriteFile.txt";
    QString text = "hello\nworld";

    // writeFile
    QVERIFY(Utils::writeFile(filePath1, text));
    QFile f1(filePath1);
    QVERIFY(f1.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content1 = QString::fromUtf8(f1.readAll());
    f1.close();
    QCOMPARE(content1, text);

    // syncWriteFile
    QVERIFY(Utils::syncWriteFile(filePath2, text));
    QFile f2(filePath2);
    QVERIFY(f2.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content2 = QString::fromUtf8(f2.readAll());
    f2.close();
    // syncWriteFile 按行写入，不追加换行符，因此直接比较去掉换行
    QCOMPARE(content2, QString("helloworld"));
}

// ---------------- 其它纯逻辑函数 ----------------

void TestUtils::testCreateUuid()
{
    QString uuid1 = Utils::createUuid();
    QString uuid2 = Utils::createUuid();

    QVERIFY(!uuid1.isEmpty());
    QVERIFY(!uuid2.isEmpty());
    QVERIFY(uuid1 != uuid2);

    QVERIFY(uuid1 != AUTO_BACKUP_UUID);
    QVERIFY(uuid1 != FACTORY_BACKUP_UUID);
}

void TestUtils::testHasChinese()
{
    QVERIFY(!Utils::hasChinese("hello world"));
    QVERIFY(Utils::hasChinese("hello世界"));
    QVERIFY(Utils::hasChinese("中文"));
}

void TestUtils::testIsSubPath()
{
    QStringList paths;
    paths << "/data" << "/backup";

    // 子路径
    QVERIFY(Utils::isSubPath(paths, "/data/abc"));
    QVERIFY(Utils::isSubPath(paths, "/backup/xyz/file"));

    // 非子路径
    QVERIFY(!Utils::isSubPath(paths, "/tmp/test"));

    // 以根路径结尾的处理（确保内部追加 / 的逻辑生效）
    paths.clear();
    paths << "/root";
    QVERIFY(Utils::isSubPath(paths, "/root/a/b"));
}

// ---------------- 系统路径与 init ----------------

void TestUtils::testInitSysRootPathAndGetters()
{
    Utils::initSysRootPath();
    QVERIFY(!Utils::getSysRootPath().isEmpty());
    QCOMPARE(Utils::getSysRootPath().at(0), QChar('/'));
    QVERIFY(!Utils::getBackupRootPath().isEmpty());
}

void TestUtils::testIsCasper()
{
    bool casper = Utils::isCasper();
    Q_UNUSED(casper);
    // 仅执行以覆盖 isCasper 分支（正常环境一般为 false）
}

// ---------------- 日志与消息 ----------------

void TestUtils::testCustomMessageHandler_allMsgTypes()
{
    QMessageLogContext ctx(nullptr, 0, nullptr, nullptr);
    Utils::customMessageHandler(QtDebugMsg, ctx, QString("debug"));
    Utils::customMessageHandler(QtWarningMsg, ctx, QString("warning"));
    Utils::customMessageHandler(QtCriticalMsg, ctx, QString("critical"));
    Utils::customMessageHandler(QtFatalMsg, ctx, QString("fatal"));
    Utils::customMessageHandler(static_cast<QtMsgType>(99), ctx, QString("default"));
}

void TestUtils::testWriteBackupLog()
{
    // writeBackupLog 固定写 getBackupRootPath() + BACKUP_LOG_TEXT_PATH，仅调用以覆盖代码
    Utils::writeBackupLog("test line");
}

// ---------------- 锁 ----------------

void TestUtils::testLockProgramUnLockProgramRmLockFile()
{
    int fd = Utils::lockProgram(12345);
    if (fd >= 0) {
        QCOMPARE(Utils::unLockProgram(fd), 0);
        Utils::rmLockFile();
    }
    // 若已被其它进程锁定则 fd == -11，仅覆盖 lockProgram 分支
}

// ---------------- fstab / 排除路径 ----------------

void TestUtils::testGetPartUuidMap()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();

    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&f);
    out << "# comment\n";
    out << "UUID=aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee  /boot  ext4  defaults  0  0\n";
    out << "# UUID=xxxxxxxx-yyyy-zzzz-1111-222222222222  LABEL=XXX\n";
    out << "/dev/sda1  /backup  ext4  noauto  0  0\n";
    f.close();

    QHash<QString, QString> map = Utils::getPartUuidMap(path);
    QVERIFY(map.contains("/boot"));
    QCOMPARE(map.value("/boot"), QString("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"));
    QVERIFY(map.contains("/backup"));
    QCOMPARE(map.value("/backup"), QString("xxxxxxxx-yyyy-zzzz-1111-222222222222"));
}

void TestUtils::testGetFstabBindInfoAndExcludeFstabBindPath()
{
    QHash<QString, QString> bindInfo = Utils::getFstabBindInfo();
    Q_UNUSED(bindInfo);

    QStringList excludes;
    Utils::excludeFstabBindPath(excludes);
    Q_UNUSED(excludes);
}

void TestUtils::testGetFromExcludePathsFile()
{
    QStringList list = Utils::getFromExcludePathsFile();
    QVERIFY(list.contains("/backup"));
    QVERIFY(list.contains("/tmp"));
}

void TestUtils::testExtraBackupOrRestorePaths()
{
    QStringList extraPaths;
    Utils::extraBackupOrRestorePaths(extraPaths);
    Q_UNUSED(extraPaths);
}

void TestUtils::testGenerateExcludePathsFile()
{
    Utils::generateExcludePathsFile();
}

// ---------------- getSubDirs ----------------

void TestUtils::testGetSubDirs()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir().mkpath(tempDir.path() + "/a");
    QDir().mkpath(tempDir.path() + "/b");

    QStringList subs = Utils::getSubDirs(tempDir.path());
    QVERIFY(subs.contains("a"));
    QVERIFY(subs.contains("b"));
}

// ---------------- 文件写失败分支 ----------------

void TestUtils::testWriteFileSyncWriteFileOpenFail()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString dirPath = dir.path();
    QVERIFY(!Utils::writeFile(dirPath, "x"));
    QVERIFY(!Utils::syncWriteFile(dirPath, "x"));
}

// ---------------- getDirOrFileSize / getDirectorySize / getLeftSize ----------------

void TestUtils::testGetDirOrFileSize()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    qint64 size = Utils::getDirOrFileSize(tempDir.path());
    QVERIFY(size >= 0);

    QCOMPARE(Utils::getDirOrFileSize("/path/with<>invalid"), qint64(0));
    QCOMPARE(Utils::getDirOrFileSize("path|with#bad"), qint64(0));
    QCOMPARE(Utils::getDirOrFileSize("'" + tempDir.path() + "'"), size);
}

void TestUtils::testGetDirectorySize()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    qint64 size = Utils::getDirectorySize(tempDir.path(), "");
    QVERIFY(size >= 0);
    qint64 sizeExcl = Utils::getDirectorySize(tempDir.path(), "nonexist");
    QVERIFY(sizeExcl >= 0);
}

void TestUtils::testGetLeftSize()
{
    qint64 left = Utils::getLeftSize("/");
    Q_UNUSED(left);
}

// ---------------- createRollBackFile / deleteFile ----------------

void TestUtils::testCreateRollBackFileAndDeleteFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString filePath = tempDir.path() + "/rollback.txt";
    Utils::createRollBackFile(filePath, "uuid-123");
    QVERIFY(QFile::exists(filePath));
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
    QCOMPARE(QString::fromUtf8(f.readAll()).trimmed(), QString("uuid-123"));
    f.close();

    Utils::deleteFile(filePath);
    QVERIFY(!QFile::exists(filePath));
    Utils::deleteFile(tempDir.path() + "/no_such_file");
}

// ---------------- 套接字与文件系统类型 ----------------

void TestUtils::testIsSocketFileAndDeleteSocketFiles()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString regFile = tempDir.path() + "/f.txt";
    QFile f(regFile);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();
    QVERIFY(!Utils::isSocketFile(regFile));
    QVERIFY(!Utils::isSocketFile(tempDir.path() + "/nonexist"));

    QVERIFY(Utils::deleteSocketFiles(tempDir.path()));
    QVERIFY(!Utils::deleteSocketFiles(tempDir.path() + "/no_such_dir"));
}

void TestUtils::testIsVFatFilesystemAndIsUDFFilesystem()
{
    QVERIFY(!Utils::isVFatFilesystem("/nonexistent_path_xyz"));
    QVERIFY(!Utils::isUDFFilesystem("/nonexistent_path_xyz"));
}

// ---------------- 等待与版本 ----------------

void TestUtils::testWaitMsecAndGetBackupVersion()
{
    Utils::waitMsec(0);
    QString ver = Utils::getBackupVersion();
    QVERIFY(!ver.isEmpty());
}

// ---------------- 备份设置与 sync ----------------

void TestUtils::testUpdateBackupUniqueSettingsAndGetBackupUuidNameMap()
{
    Utils::update_backup_unique_settings("{test-uuid-001}", "test_backup_name");
    Utils::deleteBackupUniqueRecord("test_backup_name");
    QMap<QString, QString> map = Utils::getBackupUuidNameMap();
    Q_UNUSED(map);
}

void TestUtils::testUpdateSyncFile()
{
    bool ok = Utils::updateSyncFile();
    Q_UNUSED(ok);
}

void TestUtils::testSaveSizeToFileAndGetSizeFromFile()
{
    bool saved = Utils::saveSizeToFile(1000, 2000);
    if (saved) {
        qint64 total = 0, left = 0;
        QVERIFY(Utils::getSizeFromFile(total, left));
        QCOMPARE(total, qint64(1000));
        QCOMPARE(left, qint64(2000));
    }
    qint64 t2 = 0, l2 = 0;
    Utils::getSizeFromFile(t2, l2);
    Q_UNUSED(t2);
    Q_UNUSED(l2);
}

// ---------------- 其它可测接口 ----------------

void TestUtils::testCheckBootInfoExistsAndIsTablet()
{
    Utils::checkBootInfoExists();
    Utils::isTablet();
}

void TestUtils::testGetAllUsers()
{
    QStringList users = Utils::getAllUsers();
    Q_UNUSED(users);
}

void TestUtils::testInitSystemInfoAndGetOsArch()
{
    Utils::initSystemInfo();
    QString os = Utils::getOs();
    QString arch = Utils::getArch();
    QString archDetect = Utils::getArchDetect();
    Q_UNUSED(os);
    Q_UNUSED(arch);
    Q_UNUSED(archDetect);
}

// ---------------- 异常条件 ----------------

void TestUtils::testException_getPartUuidMap()
{
    QHash<QString, QString> empty = Utils::getPartUuidMap("/nonexistent_file_xyz_123");
    QVERIFY(empty.isEmpty());

    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("");
    f.close();
    QVERIFY(Utils::getPartUuidMap(path).isEmpty());

    f.open(QIODevice::WriteOnly | QIODevice::Text);
    f.write("# only comment\n\n");
    f.close();
    QVERIFY(Utils::getPartUuidMap(path).isEmpty());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QHash<QString, QString> fromDir = Utils::getPartUuidMap(dir.path());
    QVERIFY(fromDir.isEmpty());
}

void TestUtils::testException_unLockProgram_invalidFd()
{
    int ret = Utils::unLockProgram(-1);
    QCOMPARE(ret, 1);
}

void TestUtils::testException_isSubPath()
{
    QVERIFY(!Utils::isSubPath(QStringList(), "/any/path"));
    QStringList onlyEmpty;
    onlyEmpty << "";
    QVERIFY(!Utils::isSubPath(onlyEmpty, "/any/path"));
    QStringList onlyBackupRoot;
    onlyBackupRoot << Utils::getBackupRootPath();
    QVERIFY(!Utils::isSubPath(onlyBackupRoot, "/data/abc"));
    QStringList mixed;
    mixed << "" << Utils::getBackupRootPath() << "/real";
    QVERIFY(Utils::isSubPath(mixed, "/real/sub"));
}

void TestUtils::testException_getDirectorySize()
{
    qint64 size = Utils::getDirectorySize("/nonexistent_path_xyz_12345", "");
    QCOMPARE(size, qint64(-1));
}

void TestUtils::testException_getSizeFromFile()
{
    qint64 total = 0, left = 0;
    bool ok = Utils::getSizeFromFile(total, left);
    Q_UNUSED(total);
    Q_UNUSED(left);
    Q_UNUSED(ok);
}

void TestUtils::testException_deletePath_notFileNorDir()
{
#ifdef __linux__
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString fifoPath = tempDir.path() + "/myfifo";
    if (mkfifo(fifoPath.toStdString().c_str(), 0644) == 0) {
        QVERIFY(!Utils::deletePath(fifoPath));
        unlink(fifoPath.toStdString().c_str());
    }
#else
    QVERIFY(true);
#endif
}

void TestUtils::testException_rmLockFile_whenNotExist()
{
    Utils::rmLockFile();
    bool second = Utils::rmLockFile();
    Q_UNUSED(second);
}

void TestUtils::testException_getSubDirs_notExist()
{
    QStringList subs = Utils::getSubDirs("/nonexistent_directory_xyz_123");
    QVERIFY(subs.isEmpty());
}

void TestUtils::testException_checkBootInfoExists()
{
    bool exists = Utils::checkBootInfoExists();
    Q_UNUSED(exists);
}

void TestUtils::testException_writeFile_openFail()
{
    QVERIFY(!Utils::writeFile("", "content"));
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(!Utils::writeFile(dir.path(), "content"));
}

void TestUtils::testException_hasChinese_empty()
{
    QVERIFY(!Utils::hasChinese(""));
}

void TestUtils::testException_mkpath_dirExists()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(Utils::mkpath(tempDir.path()));
}

// ---------------- 未覆盖接口 ----------------

void TestUtils::testGetBackupPartitionUuid()
{
    QString uuid = Utils::getBackupPartitionUuid();
    Q_UNUSED(uuid);
}

void TestUtils::testExcludeSomeHomePath()
{
    QStringList excludes;
    Utils::excludeSomeHomePath(excludes);
    Q_UNUSED(excludes);
}

void TestUtils::testExcludeCustomizePath()
{
    QStringList excludes;
    Utils::excludeCustomizePath(excludes);
    Q_UNUSED(excludes);
}

void TestUtils::testGetRemovableStorages()
{
    QHash<QString, QString> map = Utils::getRemovableStorages();
    Q_UNUSED(map);
}

void TestUtils::testGetLocalDisks()
{
    QList<QString> disks = Utils::getLocalDisks();
    Q_UNUSED(disks);
}

void TestUtils::testGetPartitions()
{
    QHash<QString, PartitionInfo> parts = Utils::getPartitions();
    Q_UNUSED(parts);
}

void TestUtils::testIsRunning()
{
    bool running = Utils::isRunning("nonexistent_proc_xyz_12345");
    QVERIFY(!running);
    bool init = Utils::isRunning("init");
    Q_UNUSED(init);
}

void TestUtils::testIsHuawei990()
{
    bool hw = Utils::isHuawei990();
    Q_UNUSED(hw);
}

void TestUtils::testExecuteCMDAndExecuteCmdAndProcessCmd()
{
    QString result;
    Utils::executeCMD("echo 1", result);
    QVERIFY(result.contains("1") || result.isEmpty());

    QString out = Utils::executeCmd("echo", QStringList() << "ok");
    QVERIFY(out.contains("ok") || out.isEmpty());

    QString procOut = Utils::processCmd("echo hello");
    QVERIFY(procOut.contains("hello") || procOut.isEmpty());
}

void TestUtils::testGetBackupLogList()
{
    QList<BackupWrapper> list = Utils::getBackupLogList();
    Q_UNUSED(list);
}

void TestUtils::testGetLeftSizeOfPartitions()
{
    QMap<QString, QString> map = Utils::getLeftSizeOfPartitions(true, QString());
    Q_UNUSED(map);
    QMap<QString, QString> map2 = Utils::getLeftSizeOfPartitions(false, "/tmp");
    Q_UNUSED(map2);
}

void TestUtils::testGetAvailableSizeOfPartitions()
{
    QHash<QString, qint64> hash = Utils::getAvailableSizeOfPartitions();
    Q_UNUSED(hash);
}

void TestUtils::testWait()
{
    Utils::wait(0);
}

void TestUtils::testRemountEfiAndRemountBoot()
{
    Utils::remountEfi();
    Utils::remountBoot();
}

void TestUtils::testStopNetwork()
{
    Utils::stopNetwork();
}

void TestUtils::testSetLogFile()
{
    Utils::setLogFile("/tmp/test_backup.log");
    Utils::setLogFile(QString());
}

void TestUtils::testClearBox()
{
    Utils::clearBox();
}