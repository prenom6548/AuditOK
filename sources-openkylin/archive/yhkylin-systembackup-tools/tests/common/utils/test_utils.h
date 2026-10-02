#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../common/utils.h"

class TestUtils : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // ---- StringBySize / SizeByString ----
    void testStringBySize_data();
    void testStringBySize();

    void testSizeByString_data();
    void testSizeByString();

    // ---- swapKeyValue ----
    void testSwapKeyValue();

    // ---- 目录/文件创建与删除 ----
    void testMkpathAndMkdir();
    void testDeleteSingleFile();
    void testDeleteDirectory();
    void testDeletePath_fileDirNotExists();

    // ---- 目录判断 ----
    void testIsDirExistAndIsDirExists();
    void testIsDirEmpty();

    // ---- 基础文件读写 ----
    void testWriteFileByLines();
    void testFileExists();
    void testWriteFileAndSyncWriteFile();

    // ---- 其它纯逻辑函数 ----
    void testCreateUuid();
    void testHasChinese();
    void testIsSubPath();

    // ---- 系统路径与 init ----
    void testInitSysRootPathAndGetters();
    void testIsCasper();

    // ---- 日志与消息 ----
    void testCustomMessageHandler_allMsgTypes();
    void testWriteBackupLog();

    // ---- 锁 ----
    void testLockProgramUnLockProgramRmLockFile();

    // ---- fstab / 排除路径 ----
    void testGetPartUuidMap();
    void testGetFstabBindInfoAndExcludeFstabBindPath();
    void testGetFromExcludePathsFile();
    void testExtraBackupOrRestorePaths();
    void testGenerateExcludePathsFile();

    // ---- 子目录与 getSubDirs ----
    void testGetSubDirs();

    // ---- 文件写失败分支 ----
    void testWriteFileSyncWriteFileOpenFail();

    // ---- getDirOrFileSize / getDirectorySize / getLeftSize ----
    void testGetDirOrFileSize();
    void testGetDirectorySize();
    void testGetLeftSize();

    // ---- 回滚文件与 deleteFile ----
    void testCreateRollBackFileAndDeleteFile();

    // ---- 套接字与文件系统类型 ----
    void testIsSocketFileAndDeleteSocketFiles();
    void testIsVFatFilesystemAndIsUDFFilesystem();

    // ---- 等待与版本 ----
    void testWaitMsecAndGetBackupVersion();

    // ---- 备份设置与 sync ----
    void testUpdateBackupUniqueSettingsAndGetBackupUuidNameMap();
    void testUpdateSyncFile();
    void testSaveSizeToFileAndGetSizeFromFile();

    // ---- 其它可测接口 ----
    void testCheckBootInfoExistsAndIsTablet();
    void testGetAllUsers();
    void testInitSystemInfoAndGetOsArch();

    // ---- 异常条件 ----
    void testException_getPartUuidMap();
    void testException_unLockProgram_invalidFd();
    void testException_isSubPath();
    void testException_getDirectorySize();
    void testException_getSizeFromFile();
    void testException_deletePath_notFileNorDir();
    void testException_rmLockFile_whenNotExist();
    void testException_getSubDirs_notExist();
    void testException_checkBootInfoExists();
    void testException_writeFile_openFail();
    void testException_hasChinese_empty();
    void testException_mkpath_dirExists();

    // ---- 未覆盖接口 ----
    void testGetBackupPartitionUuid();
    void testExcludeSomeHomePath();
    void testExcludeCustomizePath();
    void testGetRemovableStorages();
    void testGetLocalDisks();
    void testGetPartitions();
    void testIsRunning();
    void testIsHuawei990();
    void testExecuteCMDAndExecuteCmdAndProcessCmd();
    void testGetBackupLogList();
    void testGetLeftSizeOfPartitions();
    void testGetAvailableSizeOfPartitions();
    void testWait();
    void testRemountEfiAndRemountBoot();
    void testStopNetwork();
    void testSetLogFile();
    void testClearBox();
};

#endif // TEST_UTILS_H