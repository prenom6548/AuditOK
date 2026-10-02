#include "config-test.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

#define private public
#include "config.h"
#undef private

namespace {

QJsonObject baseSettings()
{
    QJsonObject globalSettings;
    globalSettings.insert(QStringLiteral("GlobalBlackList"),
                          QJsonArray{QStringLiteral("/proc"),
                                     QStringLiteral("/sys"),
                                     QStringLiteral("/dev"),
                                     QStringLiteral("/tmp"),
                                     QStringLiteral("/run"),
                                     QStringLiteral("/sysroot")});

    QJsonObject settings;
    settings.insert(QStringLiteral("ConfigVersion"), QStringLiteral("1.1"));
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);
    settings.insert(QStringLiteral("SearchDirs"), QJsonObject{});
    settings.insert(QStringLiteral("ExtraDirs"), QJsonArray{});
    return settings;
}

void initConfigWithSettings(Config &config, const QJsonObject &settings)
{
    config.m_settingsData = settings;
    config.m_compatibilityMode = false;
    config.m_compatibleCache.clear();
}

void removePathIfExists(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists()) {
        return;
    }
    if (info.isDir()) {
        QDir(path).removeRecursively();
        return;
    }
    QFile::remove(path);
}

void resetOnDiskConfigState()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    removePathIfExists(configDir + QStringLiteral("/ukui-search-dirs.json"));
    removePathIfExists(configDir + QStringLiteral("/ukui-search-block-dirs.conf"));
    removePathIfExists(configDir + QStringLiteral("/ukui-search-current-indexable-dir.conf"));
}

} // namespace

void ConfigTest::testSearchDirsCleansMissingAndNormalizesPaths()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("parent/child")));

    const QString dirtyPath = tempDir.path() + QStringLiteral("/parent/../parent/child");
    const QString cleanPath = QDir::cleanPath(dirtyPath);
    const QString missingPath = tempDir.path() + QStringLiteral("/missing");

    QJsonObject searchDirs;
    searchDirs.insert(dirtyPath, QJsonArray{QStringLiteral("/tmp/a")});
    searchDirs.insert(missingPath, QJsonArray{QStringLiteral("/tmp/b")});

    QJsonObject settings = baseSettings();
    settings.insert(QStringLiteral("SearchDirs"), searchDirs);
    Config config;
    initConfigWithSettings(config, settings);

    QCOMPARE(config.searchDirs(), QStringList{cleanPath});
    QJsonObject savedSearchDirs = config.m_settingsData.value(QStringLiteral("SearchDirs")).toObject();
    QVERIFY(savedSearchDirs.contains(cleanPath));
    QVERIFY(!savedSearchDirs.contains(dirtyPath));
    QVERIFY(!savedSearchDirs.contains(missingPath));
}

void ConfigTest::testBlackDirsCleansMissingSearchDirs()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString missingPath = tempDir.path() + QStringLiteral("/missing");
    QJsonObject searchDirs;
    searchDirs.insert(tempDir.path(), QJsonArray{QStringLiteral("/tmp/black-a")});
    searchDirs.insert(missingPath, QJsonArray{QStringLiteral("/tmp/black-b")});

    QJsonObject settings = baseSettings();
    settings.insert(QStringLiteral("SearchDirs"), searchDirs);
    Config config;
    initConfigWithSettings(config, settings);

    QCOMPARE(config.blackDirs(), QStringList{QStringLiteral("/tmp/black-a")});
    QVERIFY(!config.m_settingsData.value(QStringLiteral("SearchDirs")).toObject().contains(missingPath));
}

void ConfigTest::testGlobalBlackListRemovesDuplicates()
{
    QJsonObject settings = baseSettings();
    QJsonObject globalSettings = settings.value(QStringLiteral("GlobalSettings")).toObject();
    globalSettings.insert(QStringLiteral("GlobalBlackList"),
                          QJsonArray{QStringLiteral("/tmp"), QStringLiteral("/tmp"), QStringLiteral("/proc")});
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);
    Config config;
    initConfigWithSettings(config, settings);

    QCOMPARE(config.globalBlackList(), QStringList({QStringLiteral("/tmp"), QStringLiteral("/proc")}));
    QCOMPARE(config.m_settingsData.value(QStringLiteral("GlobalSettings"))
                 .toObject()
                 .value(QStringLiteral("GlobalBlackList"))
                 .toArray()
                 .size(),
             2);
}

void ConfigTest::testBlockDirsForUserCleansAndNormalizes()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("block")));

    const QString dirtyPath = tempDir.path() + QStringLiteral("/block/../block");
    const QString cleanPath = QDir::cleanPath(dirtyPath);
    const QString missingPath = tempDir.path() + QStringLiteral("/missing");

    QJsonObject settings = baseSettings();
    QJsonObject globalSettings = settings.value(QStringLiteral("GlobalSettings")).toObject();
    globalSettings.insert(QStringLiteral("BlockDirsForUser"), QJsonArray{dirtyPath, missingPath});
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);
    Config config;
    initConfigWithSettings(config, settings);

    QCOMPARE(config.blockDirsForUser(), QStringList{cleanPath});
    QCOMPARE(config.m_settingsData.value(QStringLiteral("GlobalSettings"))
                 .toObject()
                 .value(QStringLiteral("BlockDirsForUser"))
                 .toArray()
                 .size(),
             1);
}

void ConfigTest::testExtraDirsCleanMissingAndUkccVisibility()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString missingPath = tempDir.path() + QStringLiteral("/missing");
    QJsonObject settings = baseSettings();
    settings.insert(QStringLiteral("ExtraDirs"), QJsonArray{tempDir.path(), missingPath});
    Config config;
    initConfigWithSettings(config, settings);

    QCOMPARE(config.extraDirs(), QStringList{tempDir.path()});
    QVERIFY(!config.showInUkcc(tempDir.path()));
    QVERIFY(config.showInUkcc(missingPath));
}

void ConfigTest::testSetExtraDirsAndSearchDirsAppendExistingExtraDirs()
{
    QTemporaryDir searchRoot;
    QTemporaryDir extraRoot;
    QVERIFY(searchRoot.isValid());
    QVERIFY(extraRoot.isValid());

    Config config;
    initConfigWithSettings(config, baseSettings());

    config.setExtraDirs({extraRoot.path()});
    SearchDir dir(searchRoot.path(), false);
    dir.setBlackList({QStringLiteral("/black/a")});
    config.addDir(dir);

    const QStringList dirs = config.searchDirs();
    QCOMPARE(dirs.size(), 2);
    QVERIFY(dirs.contains(searchRoot.path()));
    QVERIFY(dirs.contains(extraRoot.path()));
    QVERIFY(!config.showInUkcc(extraRoot.path()));
}

void ConfigTest::testFilterSubDirsKeepsParentsOnly()
{
    Config config;
    initConfigWithSettings(config, baseSettings());
    QStringList paths{QStringLiteral("/home/user/docs"),
                      QStringLiteral("/home"),
                      QStringLiteral("/var/tmp"),
                      QStringLiteral("/var")};

    const QStringList removed = config.filterSubDirs(paths);

    QCOMPARE(paths, QStringList({QStringLiteral("/home"), QStringLiteral("/var")}));
    QCOMPARE(removed, QStringList({QStringLiteral("/home/user/docs"), QStringLiteral("/var/tmp")}));
}

void ConfigTest::testFilterSubDirsRootRemovesEverythingBelowIt()
{
    Config config;
    initConfigWithSettings(config, baseSettings());
    QStringList paths{QStringLiteral("/opt/app"), QStringLiteral("/"), QStringLiteral("/home/user")};

    const QStringList removed = config.filterSubDirs(paths);

    QCOMPARE(paths, QStringList{QStringLiteral("/")});
    QCOMPARE(removed, QStringList({QStringLiteral("/home/user"), QStringLiteral("/opt/app")}));
}

void ConfigTest::testFilterSubDirsUsesCaseInsensitiveSortOrder()
{
    Config config;
    initConfigWithSettings(config, baseSettings());
    QStringList paths{QStringLiteral("/Zeta/child"), QStringLiteral("/alpha"), QStringLiteral("/Zeta")};

    const QStringList removed = config.filterSubDirs(paths);

    QCOMPARE(paths, QStringList({QStringLiteral("/alpha"), QStringLiteral("/Zeta")}));
    QCOMPARE(removed, QStringList{QStringLiteral("/Zeta/child")});
}

void ConfigTest::testProcessCompatibleCacheMovesValidEntriesToSettings()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Config config;
    initConfigWithSettings(config, baseSettings());
    config.m_compatibilityMode = true;
    config.m_compatibleCache.append(SearchDir(tempDir.path(), false));

    config.processCompatibleCache();

    QVERIFY(!config.m_compatibilityMode);
    QVERIFY(config.m_compatibleCache.isEmpty());
    QVERIFY(config.m_settingsData.value(QStringLiteral("SearchDirs")).toObject().contains(tempDir.path()));
}

void ConfigTest::testProcessCompatibleCacheIgnoresInvalidEntriesAndNoopsWhenDisabled()
{
    Config config;
    initConfigWithSettings(config, baseSettings());
    const QJsonObject original = config.m_settingsData;

    config.processCompatibleCache();
    QCOMPARE(config.m_settingsData, original);

    config.m_compatibilityMode = true;
    config.m_compatibleCache.append(SearchDir(QStringLiteral("/tmp/ukui-search-config-missing-path"), false));

    config.processCompatibleCache();

    QVERIFY(!config.m_compatibilityMode);
    QVERIFY(config.m_compatibleCache.isEmpty());
    QVERIFY(config.m_settingsData.value(QStringLiteral("SearchDirs")).toObject().isEmpty());
}

void ConfigTest::testProcessCompatibleCacheSkipsInvalidButKeepsValidEntries()
{
    QTemporaryDir validDir;
    QVERIFY(validDir.isValid());

    Config config;
    initConfigWithSettings(config, baseSettings());
    config.m_compatibilityMode = true;
    config.m_compatibleCache.append(SearchDir(QStringLiteral("/tmp/ukui-search-config-missing-path"), false));
    config.m_compatibleCache.append(SearchDir(validDir.path(), false));

    config.processCompatibleCache();

    const QJsonObject searchDirs = config.m_settingsData.value(QStringLiteral("SearchDirs")).toObject();
    QVERIFY(searchDirs.contains(validDir.path()));
    QVERIFY(!searchDirs.contains(QStringLiteral("/tmp/ukui-search-config-missing-path")));
    QVERIFY(!config.m_compatibilityMode);
    QVERIFY(config.m_compatibleCache.isEmpty());
}

void ConfigTest::testProcessCompatibleCachePersistsGeneratedBlacklistEntries()
{
    Config config;
    initConfigWithSettings(config, baseSettings());
    config.m_compatibilityMode = true;
    config.m_compatibleCache.append(SearchDir(QStringLiteral("/"), false));

    config.processCompatibleCache();

    const QJsonObject searchDirs = config.m_settingsData.value(QStringLiteral("SearchDirs")).toObject();
    QVERIFY(searchDirs.contains(QStringLiteral("/")));
    const QStringList savedBlackList = searchDirs.value(QStringLiteral("/")).toVariant().toStringList();
    QVERIFY(!savedBlackList.isEmpty());
    QVERIFY(savedBlackList.contains(QStringLiteral("/proc")));
    QVERIFY(!config.m_compatibilityMode);
    QVERIFY(config.m_compatibleCache.isEmpty());
}

void ConfigTest::testAddAndRemoveDirRoundTrip()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Config config;
    initConfigWithSettings(config, baseSettings());
    SearchDir dir(tempDir.path(), false);
    dir.setBlackList({QStringLiteral("/black/a"), QStringLiteral("/black/b")});

    config.addDir(dir);
    QCOMPARE(config.searchDirs(), QStringList{tempDir.path()});

    const QStringList removed = config.removeDir(tempDir.path());
    QCOMPARE(removed, QStringList({QStringLiteral("/black/a"), QStringLiteral("/black/b")}));
    QVERIFY(config.searchDirs().isEmpty());
}

void ConfigTest::testAddBlockDirRemovesExistingChildren()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("parent/child")));

    const QString parent = tempDir.path() + QStringLiteral("/parent");
    const QString child = parent + QStringLiteral("/child");
    QJsonObject settings = baseSettings();
    QJsonObject globalSettings = settings.value(QStringLiteral("GlobalSettings")).toObject();
    globalSettings.insert(QStringLiteral("BlockDirsForUser"), QJsonArray{child});
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);

    Config config;
    initConfigWithSettings(config, settings);

    QCOMPARE(config.addBlockDirOfUser(parent), int(SearchDir::Successful));
    QCOMPARE(config.blockDirsForUser(), QStringList{parent});
}

void ConfigTest::testAddBlockDirRejectsMissingAndDuplicateParents()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("parent/child")));

    const QString parent = tempDir.path() + QStringLiteral("/parent");
    const QString child = parent + QStringLiteral("/child");
    QJsonObject settings = baseSettings();
    QJsonObject globalSettings = settings.value(QStringLiteral("GlobalSettings")).toObject();
    globalSettings.insert(QStringLiteral("BlockDirsForUser"), QJsonArray{parent});
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);

    Config config;
    initConfigWithSettings(config, settings);

    QCOMPARE(config.addBlockDirOfUser(tempDir.path() + QStringLiteral("/missing")), int(SearchDir::NotExists));
    QCOMPARE(config.addBlockDirOfUser(parent), int(SearchDir::Duplicated));
    QCOMPARE(config.addBlockDirOfUser(child), int(SearchDir::Duplicated));
    QCOMPARE(config.blockDirsForUser(), QStringList{parent});
}

void ConfigTest::testAddRootBlockDirRemovesExistingChildren()
{
    QJsonObject settings = baseSettings();
    QJsonObject globalSettings = settings.value(QStringLiteral("GlobalSettings")).toObject();
    globalSettings.insert(QStringLiteral("BlockDirsForUser"),
                          QJsonArray{QStringLiteral("/tmp"), QStringLiteral("/var/tmp")});
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);

    Config config;
    initConfigWithSettings(config, settings);

    QCOMPARE(config.addBlockDirOfUser(QStringLiteral("/")), int(SearchDir::Successful));
    QCOMPARE(config.blockDirsForUser(), QStringList{QStringLiteral("/")});
    QCOMPARE(config.addBlockDirOfUser(QStringLiteral("/tmp")), int(SearchDir::Duplicated));
}

void ConfigTest::testRemoveExistingBlockDirRewritesSettings()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QJsonObject settings = baseSettings();
    QJsonObject globalSettings = settings.value(QStringLiteral("GlobalSettings")).toObject();
    globalSettings.insert(QStringLiteral("BlockDirsForUser"), QJsonArray{tempDir.path()});
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);

    Config config;
    initConfigWithSettings(config, settings);

    config.removeBlockDirOfUser(tempDir.path());

    QVERIFY(config.blockDirsForUser().isEmpty());
    QVERIFY(config.m_settingsData.value(QStringLiteral("GlobalSettings"))
                .toObject()
                .value(QStringLiteral("BlockDirsForUser"))
                .toArray()
                .isEmpty());
}

void ConfigTest::testRemoveMissingBlockDirDoesNotRewriteSettings()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QJsonObject settings = baseSettings();
    QJsonObject globalSettings = settings.value(QStringLiteral("GlobalSettings")).toObject();
    globalSettings.insert(QStringLiteral("BlockDirsForUser"), QJsonArray{tempDir.path()});
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);

    Config config;
    initConfigWithSettings(config, settings);
    const QJsonObject before = config.m_settingsData;

    config.removeBlockDirOfUser(tempDir.path() + QStringLiteral("/missing"));

    QCOMPARE(config.m_settingsData, before);
}

void ConfigTest::testGlobalBlackListWithoutDuplicatesDoesNotRewrite()
{
    QJsonObject settings = baseSettings();
    Config config;
    initConfigWithSettings(config, settings);

    const QJsonObject before = config.m_settingsData;
    QCOMPARE(config.globalBlackList(),
             QStringList({QStringLiteral("/proc"),
                          QStringLiteral("/sys"),
                          QStringLiteral("/dev"),
                          QStringLiteral("/tmp"),
                          QStringLiteral("/run"),
                          QStringLiteral("/sysroot")}));
    QCOMPARE(config.m_settingsData, before);
}

void ConfigTest::testInitSettingsDataCreatesCurrentDefaults()
{
    Config config;
    initConfigWithSettings(config, QJsonObject{});

    QJsonObject data;
    config.initSettingsData(data);

    QCOMPARE(data.value(QStringLiteral("ConfigVersion")).toString(), QStringLiteral("1.1"));
    const QStringList globalBlackList = data.value(QStringLiteral("GlobalSettings"))
                                            .toObject()
                                            .value(QStringLiteral("GlobalBlackList"))
                                            .toVariant()
                                            .toStringList();
    QVERIFY(globalBlackList.contains(QStringLiteral("/proc")));
    QVERIFY(globalBlackList.contains(QStringLiteral("/sysroot")));
}

void ConfigTest::testInitDefaultCompatibleConfigOnlyAddsOnce()
{
    Config config;
    initConfigWithSettings(config, baseSettings());

    config.initDefaultCompatibleConfig();
    config.initDefaultCompatibleConfig();

    QVERIFY(config.m_compatibilityMode);
    QCOMPARE(config.m_compatibleCache.size(), 1);
    QCOMPARE(config.m_compatibleCache.first().getPath(), QDir::homePath());
}

void ConfigTest::testCheckSearchDirNormalizesAndDropsNestedChildren()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("parent/child")));

    const QString parent = tempDir.path() + QStringLiteral("/parent");
    const QString dirtyChild = parent + QStringLiteral("/child/../child");
    const QString cleanChild = QDir::cleanPath(dirtyChild);

    QJsonObject searchDirs;
    searchDirs.insert(parent, QJsonArray{});
    searchDirs.insert(dirtyChild, QJsonArray{});

    QJsonObject settings = baseSettings();
    settings.insert(QStringLiteral("SearchDirs"), searchDirs);
    Config config;
    initConfigWithSettings(config, settings);

    config.checkSearchDir();

    const QJsonObject savedSearchDirs = config.m_settingsData.value(QStringLiteral("SearchDirs")).toObject();
    QVERIFY(savedSearchDirs.contains(parent));
    QVERIFY(!savedSearchDirs.contains(dirtyChild));
    QVERIFY(!savedSearchDirs.contains(cleanChild));
}

void ConfigTest::testCheckSearchDirRemovesMissingWithoutRewriteWhenNoNormalization()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString missingPath = tempDir.path() + QStringLiteral("/missing");
    QJsonObject searchDirs;
    searchDirs.insert(missingPath, QJsonArray{QStringLiteral("/black")});

    QJsonObject settings = baseSettings();
    settings.insert(QStringLiteral("SearchDirs"), searchDirs);
    Config config;
    initConfigWithSettings(config, settings);

    config.checkSearchDir();

    QVERIFY(config.m_settingsData.value(QStringLiteral("SearchDirs")).toObject().contains(missingPath));
}

void ConfigTest::testCheckSearchDirNoRewriteWhenAlreadyClean()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QJsonObject searchDirs;
    searchDirs.insert(tempDir.path(), QJsonArray{QStringLiteral("/black")});

    QJsonObject settings = baseSettings();
    settings.insert(QStringLiteral("SearchDirs"), searchDirs);
    Config config;
    initConfigWithSettings(config, settings);

    const QJsonObject before = config.m_settingsData;
    config.checkSearchDir();

    QCOMPARE(config.m_settingsData, before);
}

void ConfigTest::testAddBlockDirPreservesSiblingDirs()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString sibling = tempDir.path() + QStringLiteral("/sibling");
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("sibling")));

    QJsonObject settings = baseSettings();
    QJsonObject globalSettings = settings.value(QStringLiteral("GlobalSettings")).toObject();
    globalSettings.insert(QStringLiteral("BlockDirsForUser"), QJsonArray{sibling});
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);

    Config config;
    initConfigWithSettings(config, settings);

    // Add a new dir that is NOT a parent of sibling → sibling stays (L103 it++)
    const QString other = tempDir.path() + QStringLiteral("/other");
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("other")));
    QCOMPARE(config.addBlockDirOfUser(other), int(SearchDir::Successful));
    QCOMPARE(config.blockDirsForUser().size(), 2);
    QVERIFY(config.blockDirsForUser().contains(sibling));
    QVERIFY(config.blockDirsForUser().contains(other));
}

void ConfigTest::testGlobalBlackListRewriteTrimsJsonArray()
{
    // globalBlackList with duplicates → rewrite with smaller array
    QJsonObject settings = baseSettings();
    QJsonObject globalSettings = settings.value(QStringLiteral("GlobalSettings")).toObject();
    globalSettings.insert(QStringLiteral("GlobalBlackList"),
                          QJsonArray{QStringLiteral("/tmp"), QStringLiteral("/tmp"), QStringLiteral("/tmp")});
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);
    Config config;
    initConfigWithSettings(config, settings);

    QStringList result = config.globalBlackList();
    QCOMPARE(result, QStringList{QStringLiteral("/tmp")});
    // Verify JSON array was trimmed
    QCOMPARE(config.m_settingsData.value(QStringLiteral("GlobalSettings"))
                 .toObject()
                 .value(QStringLiteral("GlobalBlackList"))
                 .toArray()
                 .size(),
             1);
}

void ConfigTest::testSearchDirsWithExistingExtraDirsIncludesThem()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QJsonObject settings = baseSettings();
    settings.insert(QStringLiteral("ExtraDirs"), QJsonArray{tempDir.path()});

    Config config;
    initConfigWithSettings(config, settings);

    QStringList dirs = config.searchDirs();
    QVERIFY(dirs.contains(tempDir.path()));
}

void ConfigTest::testCheckSearchDirNormalizesDirtyPathsAndSaves()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("parent/child")));

    const QString parent = tempDir.path() + QStringLiteral("/parent");
    const QString dirtyPath = parent + QStringLiteral("/child/../child/../child");

    QJsonObject searchDirs;
    searchDirs.insert(dirtyPath, QJsonArray{QStringLiteral("/black/dirty")});

    QJsonObject settings = baseSettings();
    settings.insert(QStringLiteral("SearchDirs"), searchDirs);
    Config config;
    initConfigWithSettings(config, settings);

    // This should normalize and trigger needCheck → filterSubDirs
    config.checkSearchDir();

    const QJsonObject savedSearchDirs = config.m_settingsData.value(QStringLiteral("SearchDirs")).toObject();
    QVERIFY(savedSearchDirs.contains(QDir::cleanPath(dirtyPath)));
    QVERIFY(!savedSearchDirs.contains(dirtyPath));
}

void ConfigTest::testConstructorMigratesOldBlockSettingsWhenNoNewConfigExists()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");
    const QString oldBlockFile = configDir + QStringLiteral("/ukui-search-block-dirs.conf");

    // Ensure new settings file does not exist so constructor takes migration path
    resetOnDiskConfigState();

    QDir().mkpath(configDir);
    QSettings oldBlock(oldBlockFile, QSettings::IniFormat);
    oldBlock.setValue(QStringLiteral("tmp"), QString());
    oldBlock.sync();

    Config config;
    // Old block dirs should be migrated: "/tmp" becomes a block dir
    QVERIFY(config.blockDirsForUser().contains(QStringLiteral("/tmp")));

    QFile::remove(oldBlockFile);
}

void ConfigTest::testSaveHandlesConfigDirCreationFailure()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    const QString configDirParent = QDir::homePath() + QStringLiteral("/.config/org.ukui");
    const QString dotConfigDir = QDir::homePath() + QStringLiteral("/.config");

    resetOnDiskConfigState();
    removePathIfExists(configDir);
    removePathIfExists(configDirParent);
    QDir().mkpath(dotConfigDir);

    QFile fileHandle(configDirParent);
    QVERIFY(fileHandle.open(QIODevice::WriteOnly));
    fileHandle.close();

    Config config;
    initConfigWithSettings(config, baseSettings());
    config.save(config.m_settingsData);

    QVERIFY(QFileInfo(configDirParent).isFile());
    QVERIFY(!QFileInfo::exists(configDir));

    QFile::remove(configDirParent);
}

void ConfigTest::testSaveHandlesFileOpenFailure()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");

    // Remove any existing file, create a directory with same name → open() fails
    QFile::remove(settingsFile);
    QDir().mkpath(settingsFile);  // directory where file should be

    Config config;
    initConfigWithSettings(config, baseSettings());
    config.save(config.m_settingsData);  // L152-153: open fails with "Fail to open file"

    // Clean up: remove directory
    QDir().rmdir(settingsFile);
}

void ConfigTest::testSaveHandlesWriteFailure()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");

    // Ensure file exists and is read-only → write() returns -1
    QDir().mkpath(configDir);
    QFile::remove(settingsFile);
    QFile file(settingsFile);
    file.open(QIODevice::WriteOnly);
    file.close();
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::ReadGroup);

    Config config;
    initConfigWithSettings(config, baseSettings());
    config.save(config.m_settingsData);  // L157: write fails with "Error saving"

    // Clean up
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorUpgradesVersion1_0To1_1()
{
    // Write settings file with version "1.0" (without /sysroot in blacklist)
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");

    QJsonObject globalSettings;
    globalSettings.insert(QStringLiteral("GlobalBlackList"),
                          QJsonArray{QStringLiteral("/proc"), QStringLiteral("/sys"),
                                     QStringLiteral("/dev"), QStringLiteral("/tmp"),
                                     QStringLiteral("/run")});  // no /sysroot

    QJsonObject settings;
    settings.insert(QStringLiteral("ConfigVersion"), QStringLiteral("1.0"));
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);
    settings.insert(QStringLiteral("SearchDirs"), QJsonObject{});

    QFile file(settingsFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(settings).toJson());
    file.close();

    // Constructor: sees version 1.0 → adds /sysroot, upgrades to 1.1
    Config config;
    QStringList bl = config.globalBlackList();
    QVERIFY(bl.contains(QStringLiteral("/sysroot")));

    // Verify config was upgraded to 1.1
    QCOMPARE(config.m_settingsData.value(QStringLiteral("ConfigVersion")).toString(),
             QStringLiteral("1.1"));

    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorVersion1_0KeepsExistingSysrootEntry()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");

    QJsonObject globalSettings;
    globalSettings.insert(QStringLiteral("GlobalBlackList"),
                          QJsonArray{QStringLiteral("/proc"), QStringLiteral("/sys"),
                                     QStringLiteral("/dev"), QStringLiteral("/tmp"),
                                     QStringLiteral("/run"), QStringLiteral("/sysroot")});

    QJsonObject settings;
    settings.insert(QStringLiteral("ConfigVersion"), QStringLiteral("1.0"));
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);
    settings.insert(QStringLiteral("SearchDirs"), QJsonObject{});

    QFile file(settingsFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(settings).toJson());
    file.close();

    Config config;

    QCOMPARE(config.globalBlackList().count(QStringLiteral("/sysroot")), 1);
    QCOMPARE(config.m_settingsData.value(QStringLiteral("ConfigVersion")).toString(),
             QStringLiteral("1.1"));

    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorLeavesUnsupportedVersionUntouched()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");

    QJsonObject settings = baseSettings();
    settings.insert(QStringLiteral("ConfigVersion"), QStringLiteral("2.0"));

    QFile file(settingsFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(settings).toJson());
    file.close();

    Config config;

    QCOMPARE(config.m_settingsData.value(QStringLiteral("ConfigVersion")).toString(),
             QStringLiteral("2.0"));
    QVERIFY(config.globalBlackList().contains(QStringLiteral("/sysroot")));

    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorHandlesJsonParseError()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");

    // Write invalid JSON
    QFile file(settingsFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QByteArray("not valid json {{{"));
    file.close();

    // Constructor: JSON parse error → inits defaults, saves
    Config config;
    QVERIFY(!config.m_settingsData.isEmpty());

    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorReturnsWhenSettingsPathCannotBeOpened()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");

    resetOnDiskConfigState();
    QDir().mkpath(settingsFile);

    Config config;

    QVERIFY(config.m_settingsData.isEmpty());

    QDir().rmdir(settingsFile);
}

void ConfigTest::testConstructorMissingSearchDirsGroupInitsCompatibleConfig()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");

    // Write valid JSON without SEARCH_DIRS_GROUP
    QJsonObject globalSettings;
    globalSettings.insert(QStringLiteral("GlobalBlackList"),
                          QJsonArray{QStringLiteral("/proc"), QStringLiteral("/sys"),
                                     QStringLiteral("/dev"), QStringLiteral("/tmp"),
                                     QStringLiteral("/run"), QStringLiteral("/sysroot")});

    QJsonObject settings;
    settings.insert(QStringLiteral("ConfigVersion"), QStringLiteral("1.1"));
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);
    // No SEARCH_DIRS_GROUP key

    QFile file(settingsFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(settings).toJson());
    file.close();

    // Constructor: missing SEARCH_DIRS_GROUP → initDefaultCompatibleConfig
    Config config;
    QVERIFY(config.isCompatibilityMode() || !config.m_compatibleCache.isEmpty());

    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorLoadsExistingOldIndexableDirsIntoCompatibleCache()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");
    const QString oldIndexableFile = configDir + QStringLiteral("/ukui-search-current-indexable-dir.conf");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);

    QSettings oldSettings(oldIndexableFile, QSettings::IniFormat);
    oldSettings.setValue(QStringLiteral("IndexableDir/IndexableDir"),
                         QStringList{tempDir.path(),
                                     tempDir.path() + QStringLiteral("/missing")});
    oldSettings.sync();

    Config config;

    QVERIFY(!config.isCompatibilityMode());
    QCOMPARE(config.m_compatibleCache.size(), 1);
    QCOMPARE(config.m_compatibleCache.first().getPath(), tempDir.path());

    QFile::remove(oldIndexableFile);
    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorSkipsCompatibilityInitWhenOldIndexableEntriesAreAllMissing()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");
    const QString oldIndexableFile = configDir + QStringLiteral("/ukui-search-current-indexable-dir.conf");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);

    QSettings oldSettings(oldIndexableFile, QSettings::IniFormat);
    oldSettings.setValue(QStringLiteral("IndexableDir/IndexableDir"),
                         QStringList{QStringLiteral("/tmp/ukui-search-config-missing-a"),
                                     QStringLiteral("/tmp/ukui-search-config-missing-b")});
    oldSettings.sync();

    Config config;

    QVERIFY(!config.isCompatibilityMode());
    QVERIFY(config.m_compatibleCache.isEmpty());
    QVERIFY(config.m_settingsData.contains(QStringLiteral("GlobalSettings")));
    QVERIFY(config.m_settingsData.value(QStringLiteral("SearchDirs")).toObject().isEmpty());

    QFile::remove(oldIndexableFile);
    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorReinitializesWhenGlobalSettingsMissing()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");

    QJsonObject settings;
    settings.insert(QStringLiteral("ConfigVersion"), QStringLiteral("1.1"));
    settings.insert(QStringLiteral("SearchDirs"), QJsonObject{});
    settings.insert(QStringLiteral("ExtraDirs"), QJsonArray{});

    QFile file(settingsFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(settings).toJson());
    file.close();

    Config config;

    QVERIFY(config.m_settingsData.contains(QStringLiteral("GlobalSettings")));
    QVERIFY(config.globalBlackList().contains(QStringLiteral("/sysroot")));

    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorReinitializesWhenVersionMissing()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");

    QJsonObject globalSettings;
    globalSettings.insert(QStringLiteral("GlobalBlackList"),
                          QJsonArray{QStringLiteral("/proc"),
                                     QStringLiteral("/sys"),
                                     QStringLiteral("/dev"),
                                     QStringLiteral("/tmp"),
                                     QStringLiteral("/run"),
                                     QStringLiteral("/sysroot")});

    QJsonObject searchDirs;
    searchDirs.insert(tempDir.path(), QJsonArray{});

    QJsonObject settings;
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);
    settings.insert(QStringLiteral("SearchDirs"), searchDirs);
    settings.insert(QStringLiteral("ExtraDirs"), QJsonArray{});

    QFile file(settingsFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(settings).toJson());
    file.close();

    Config config;

    QCOMPARE(config.m_settingsData.value(QStringLiteral("ConfigVersion")).toString(), QStringLiteral("1.1"));
    QVERIFY(config.m_settingsData.value(QStringLiteral("GlobalSettings")).isObject());
    QVERIFY(!config.isCompatibilityMode());
    QVERIFY(config.m_compatibleCache.isEmpty());
    QVERIFY(config.m_settingsData.value(QStringLiteral("SearchDirs")).toObject().contains(tempDir.path()));

    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorMissingVersionAndSearchDirsEnablesCompatibilityMode()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");

    QJsonObject globalSettings;
    globalSettings.insert(QStringLiteral("GlobalBlackList"),
                          QJsonArray{QStringLiteral("/proc"),
                                     QStringLiteral("/sys"),
                                     QStringLiteral("/dev"),
                                     QStringLiteral("/tmp"),
                                     QStringLiteral("/run"),
                                     QStringLiteral("/sysroot")});

    QJsonObject settings;
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);
    settings.insert(QStringLiteral("ExtraDirs"), QJsonArray{});

    QFile file(settingsFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(settings).toJson());
    file.close();

    Config config;

    QCOMPARE(config.m_settingsData.value(QStringLiteral("ConfigVersion")).toString(), QStringLiteral("1.1"));
    QVERIFY(config.isCompatibilityMode());
    QCOMPARE(config.m_compatibleCache.size(), 1);

    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorNormalizesExistingBlockDirsForUser()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("block")));

    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");
    const QString dirtyPath = tempDir.path() + QStringLiteral("/block/../block");
    const QString missingPath = tempDir.path() + QStringLiteral("/missing");

    QJsonObject settings = baseSettings();
    QJsonObject globalSettings = settings.value(QStringLiteral("GlobalSettings")).toObject();
    globalSettings.insert(QStringLiteral("BlockDirsForUser"), QJsonArray{dirtyPath, missingPath});
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);

    QFile file(settingsFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(settings).toJson());
    file.close();

    Config config;

    QCOMPARE(config.blockDirsForUser(), QStringList{QDir::cleanPath(dirtyPath)});

    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorDropsMissingBlockDirsWithoutFiltering()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");
    const QString missingPath = tempDir.path() + QStringLiteral("/missing");

    QJsonObject settings = baseSettings();
    QJsonObject globalSettings = settings.value(QStringLiteral("GlobalSettings")).toObject();
    globalSettings.insert(QStringLiteral("BlockDirsForUser"), QJsonArray{missingPath});
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);

    QFile file(settingsFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(settings).toJson());
    file.close();

    Config config;

    QVERIFY(config.blockDirsForUser().isEmpty());
    QVERIFY(config.m_settingsData.value(QStringLiteral("GlobalSettings"))
                .toObject()
                .value(QStringLiteral("BlockDirsForUser"))
                .toArray()
                .isEmpty());

    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorFiltersNestedBlockDirsForUser()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("parent/child")));

    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");

    const QString parent = tempDir.filePath(QStringLiteral("parent"));
    const QString dirtyChild = parent + QStringLiteral("/child/../child");
    const QString cleanChild = QDir::cleanPath(dirtyChild);

    QJsonObject settings = baseSettings();
    QJsonObject globalSettings = settings.value(QStringLiteral("GlobalSettings")).toObject();
    globalSettings.insert(QStringLiteral("BlockDirsForUser"), QJsonArray{parent, dirtyChild});
    settings.insert(QStringLiteral("GlobalSettings"), globalSettings);

    QFile file(settingsFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(settings).toJson());
    file.close();

    Config config;

    QCOMPARE(config.blockDirsForUser(), QStringList{parent});
    const QJsonArray blockDirs = config.m_settingsData.value(QStringLiteral("GlobalSettings"))
                                     .toObject()
                                     .value(QStringLiteral("BlockDirsForUser"))
                                     .toArray();
    QCOMPARE(blockDirs.size(), 1);
    QCOMPARE(blockDirs.first().toString(), parent);
    QVERIFY(!blockDirs.contains(cleanChild));

    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorMigratesEmptyOldBlockSettingsIntoCurrentConfig()
{
    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");
    const QString oldBlockFile = configDir + QStringLiteral("/ukui-search-block-dirs.conf");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);

    QJsonObject settingsData = baseSettings();
    QFile settingsHandle(settingsFile);
    QVERIFY(settingsHandle.open(QIODevice::WriteOnly));
    settingsHandle.write(QJsonDocument(settingsData).toJson());
    settingsHandle.close();

    QSettings oldBlock(oldBlockFile, QSettings::IniFormat);
    oldBlock.setValue(QStringLiteral("path/that/does/not/exist"), QString());
    oldBlock.sync();

    Config config;

    QVERIFY(config.m_settingsData.value(QStringLiteral("GlobalSettings"))
                .toObject()
                .contains(QStringLiteral("BlockDirsForUser")));
    QVERIFY(config.blockDirsForUser().isEmpty());

    QFile::remove(oldBlockFile);
    QFile::remove(settingsFile);
}

void ConfigTest::testConstructorMigratesOldBlockSettingsIntoExistingConfig()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("blocked")));

    const QString configDir = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search");
    const QString settingsFile = configDir + QStringLiteral("/ukui-search-dirs.json");
    const QString oldBlockFile = configDir + QStringLiteral("/ukui-search-block-dirs.conf");
    resetOnDiskConfigState();
    QDir().mkpath(configDir);

    QJsonObject settingsData = baseSettings();
    QFile settingsHandle(settingsFile);
    QVERIFY(settingsHandle.open(QIODevice::WriteOnly));
    settingsHandle.write(QJsonDocument(settingsData).toJson());
    settingsHandle.close();

    const QString oldKey = tempDir.path().mid(1) + QStringLiteral("/blocked");
    QSettings oldBlock(oldBlockFile, QSettings::IniFormat);
    oldBlock.setValue(oldKey, QString());
    oldBlock.sync();

    Config config;

    QCOMPARE(config.blockDirsForUser(), QStringList{tempDir.filePath(QStringLiteral("blocked"))});

    QFile::remove(oldBlockFile);
    QFile::remove(settingsFile);
}

QTEST_MAIN(ConfigTest)
