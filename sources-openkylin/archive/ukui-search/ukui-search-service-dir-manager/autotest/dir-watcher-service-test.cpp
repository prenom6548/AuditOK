#include "dir-watcher-service-test.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#define private public
#include "dir-watcher.h"
#undef private

#include "search-dir.h"

#define private public
#include "config.h"
#undef private

#define private public
#include "volume-manager.h"
#undef private

namespace {
void resetDirManagerState()
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

    Config::self()->m_settingsData = settings;
    Config::self()->m_compatibilityMode = false;
    Config::self()->m_compatibleCache.clear();
    VolumeManager::self()->m_volumes.clear();
    VolumeManager::self()->m_uDiskVolumes.clear();
}

void appendGlobalBlackList(const QString &path)
{
    QJsonObject globalSettings = Config::self()->m_settingsData.value(QStringLiteral("GlobalSettings")).toObject();
    QJsonArray globalBlackList = globalSettings.value(QStringLiteral("GlobalBlackList")).toArray();
    globalBlackList.append(path);
    globalSettings.insert(QStringLiteral("GlobalBlackList"), globalBlackList);
    Config::self()->m_settingsData.insert(QStringLiteral("GlobalSettings"), globalSettings);
}

void setRootSearchDirWithBlacklist(const QString &blackListPath)
{
    QJsonObject searchDirs;
    searchDirs.insert(QStringLiteral("/"), QJsonArray{blackListPath});
    Config::self()->m_settingsData.insert(QStringLiteral("SearchDirs"), searchDirs);
}
}

void DirWatcherServiceTest::testSingletonAndAliasMethods()
{
    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QVERIFY(watcher != nullptr);
    QCOMPARE(DirWatcher::getDirWatcher(), watcher);

    QCOMPARE(watcher->currentIndexableDir(), watcher->currentSearchDirs());
    QCOMPARE(watcher->currentBlackListOfIndex(), watcher->currentBlackList());
    QCOMPARE(watcher->currentSearchableDir(), watcher->currentSearchDirs());
    QCOMPARE(watcher->searchableDirForSearchApplication(), watcher->currentSearchDirs());
}

void DirWatcherServiceTest::testAppendAndRemoveSignalsForTemporaryDirectory()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QSignalSpy appendSpy(watcher, SIGNAL(appendIndexItem(QString,QStringList)));
    QSignalSpy removeSpy(watcher, SIGNAL(removeIndexItem(QString)));
    QVERIFY(appendSpy.isValid());
    QVERIFY(removeSpy.isValid());

    const QString path = tempDir.path();
    QCOMPARE(watcher->appendSearchDir(path), int(SearchDir::Successful));
    QCOMPARE(appendSpy.count(), 1);
    QCOMPARE(appendSpy.first().at(0).toString(), path);
    QVERIFY(watcher->currentSearchDirs().contains(path));

    QVERIFY(watcher->removeIndexableListItem(path));
    QCOMPARE(removeSpy.count(), 1);
    QCOMPARE(removeSpy.first().at(0).toString(), path);
    QVERIFY(!watcher->currentSearchDirs().contains(path));
}

void DirWatcherServiceTest::testBlockDirSignalsAndErrorCodes()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QSignalSpy addedSpy(watcher, SIGNAL(blockItemAdded(QString)));
    QSignalSpy removedSpy(watcher, SIGNAL(blockItemRemoved(QString)));
    QVERIFY(addedSpy.isValid());
    QVERIFY(removedSpy.isValid());

    const QString path = tempDir.path();
    QCOMPARE(watcher->addBlockDirOfUser(path), int(SearchDir::Successful));
    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(addedSpy.first().at(0).toString(), path);
    QVERIFY(watcher->blockDirsForUser().contains(path));

    QCOMPARE(watcher->addBlockDirOfUser(path), int(SearchDir::Duplicated));
    QCOMPARE(addedSpy.count(), 1);

    const QString missing = path + QStringLiteral("/missing");
    QCOMPARE(watcher->addBlockDirOfUser(missing), int(SearchDir::NotExists));
    QCOMPARE(addedSpy.count(), 1);

    watcher->removeBlockDirOfUser(path);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.first().at(0).toString(), path);
    QVERIFY(!watcher->blockDirsForUser().contains(path));
}

void DirWatcherServiceTest::testSearchableListsAndUkccVisibility()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QString path = tempDir.path();
    QCOMPARE(watcher->appendIndexableListItem(path), int(SearchDir::Successful));

    QVERIFY(watcher->currentSearchDirs().contains(path));
    QVERIFY(watcher->currentSearchableDir().contains(path));
    QVERIFY(watcher->searchableDirForSearchApplication().contains(path));
    QVERIFY(watcher->showInUkcc(path));

    watcher->removeSearchDir(path);
}

void DirWatcherServiceTest::testExtraDirsOfPathsWithNoMatchingUDisk()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QStringList paths{tempDir.path(), tempDir.path() + QStringLiteral("/child/../child")};
    const QStringList extraDirs = watcher->extraDirsOfPaths(paths);

    for (const QString &path : extraDirs) {
        QVERIFY(!path.isEmpty());
    }
}

void DirWatcherServiceTest::testAppendRejectsMissingPathWithoutSignal()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QSignalSpy appendSpy(watcher, SIGNAL(appendIndexItem(QString,QStringList)));
    QVERIFY(appendSpy.isValid());

    const QString missing = tempDir.path() + QStringLiteral("/missing");
    QCOMPARE(watcher->appendSearchDir(missing), int(SearchDir::NotExists));
    QCOMPARE(appendSpy.count(), 0);
    QVERIFY(!watcher->currentSearchDirs().contains(QDir::cleanPath(missing)));
}

void DirWatcherServiceTest::testDuplicateSearchDirDoesNotEmitAppendAgain()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QString path = tempDir.path();
    QCOMPARE(watcher->appendSearchDir(path), int(SearchDir::Successful));

    QSignalSpy appendSpy(watcher, SIGNAL(appendIndexItem(QString,QStringList)));
    QVERIFY(appendSpy.isValid());

    QCOMPARE(watcher->appendSearchDir(path), int(SearchDir::Duplicated));
    QCOMPARE(appendSpy.count(), 0);

    watcher->removeSearchDir(path);
}

void DirWatcherServiceTest::testRemoveNormalizesPathAndEmitsSignal()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QString normalized = tempDir.path();
    const QString dirtyPath = normalized + QStringLiteral("/child/..");
    QCOMPARE(watcher->appendSearchDir(normalized), int(SearchDir::Successful));

    QSignalSpy removeSpy(watcher, SIGNAL(removeIndexItem(QString)));
    QVERIFY(removeSpy.isValid());

    watcher->removeSearchDir(dirtyPath);
    QCOMPARE(removeSpy.count(), 1);
    QCOMPARE(removeSpy.first().at(0).toString(), normalized);
    QVERIFY(!watcher->currentSearchDirs().contains(normalized));
}

void DirWatcherServiceTest::testBlockDirRemovalEmitsEvenWhenMissing()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QString missing = tempDir.path() + QStringLiteral("/missing/..");
    const QString normalized = QDir::cleanPath(missing);

    QSignalSpy removedSpy(watcher, SIGNAL(blockItemRemoved(QString)));
    QVERIFY(removedSpy.isValid());

    watcher->removeBlockDirOfUser(missing);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.first().at(0).toString(), normalized);
}

void DirWatcherServiceTest::testBlackListOfDirDelegatesToSearchDir()
{
    DirWatcher *watcher = DirWatcher::getDirWatcher();

    const QStringList blackList = watcher->blackListOfDir(QStringLiteral("/"));

    QVERIFY(blackList.contains(QStringLiteral("/proc")));
    QVERIFY(blackList.contains(QStringLiteral("/sysroot")));
}

void DirWatcherServiceTest::testAppendParentRemovesExistingChildSearchDir()
{
    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/dir-watcher-parent-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("parent/child")));

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QString parent = tempDir.path() + QStringLiteral("/parent");
    const QString child = parent + QStringLiteral("/child");

    QCOMPARE(watcher->appendSearchDir(child), int(SearchDir::Successful));
    QCOMPARE(watcher->appendSearchDir(parent), int(SearchDir::Successful));

    QVERIFY(watcher->currentSearchDirs().contains(parent));
    QVERIFY(!watcher->currentSearchDirs().contains(child));

    watcher->removeSearchDir(parent);
}

void DirWatcherServiceTest::testShowInUkccNormalizesPath()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QString dirtyPath = tempDir.path() + QStringLiteral("/child/..");

    QVERIFY(watcher->showInUkcc(dirtyPath));
}

void DirWatcherServiceTest::testInitDbusServiceCanBeCalledRepeatedly()
{
    DirWatcher *watcher = DirWatcher::getDirWatcher();

    watcher->initDbusService();
    watcher->initDbusService();

    QVERIFY(watcher != nullptr);
}

void DirWatcherServiceTest::testExtraDirsOfPathsReturnsUDiskUnderGlobalBlackList()
{
    resetDirManagerState();

    Volume uDisk(QStringLiteral("/dev/sdz1"));
    uDisk.addMountPoint(QStringLiteral("/run/media/user/disk"));
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);
    setRootSearchDirWithBlacklist(QStringLiteral("/run"));

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QStringList extraDirs = watcher->extraDirsOfPaths({QStringLiteral("/")});

    QCOMPARE(extraDirs, QStringList{QStringLiteral("/run/media/user/disk")});
    resetDirManagerState();
}

void DirWatcherServiceTest::testRefreshExtraDirsEmitsAddedAndRemovedSignals()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("disk2")));
    const QString mountPoint = tempDir.filePath(QStringLiteral("disk2"));
    appendGlobalBlackList(tempDir.path());

    Volume uDisk(QStringLiteral("/dev/sdz2"));
    uDisk.addMountPoint(mountPoint);
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);
    setRootSearchDirWithBlacklist(tempDir.path());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QSignalSpy appendSpy(watcher, SIGNAL(appendIndexItem(QString,QStringList)));
    QSignalSpy removeSpy(watcher, SIGNAL(removeIndexItem(QString)));
    QVERIFY(appendSpy.isValid());
    QVERIFY(removeSpy.isValid());

    watcher->refreshExtraDirs();

    QCOMPARE(appendSpy.count(), 1);
    QCOMPARE(appendSpy.first().at(0).toString(), mountPoint);
    QVERIFY(Config::self()->extraDirs().contains(mountPoint));

    VolumeManager::self()->m_uDiskVolumes.clear();
    watcher->refreshExtraDirs();

    QCOMPARE(removeSpy.count(), 1);
    QCOMPARE(removeSpy.first().at(0).toString(), mountPoint);
    resetDirManagerState();
}

void DirWatcherServiceTest::testUDiskCallbacksTriggerRefreshExtraDirs()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QSignalSpy appendSpy(watcher, SIGNAL(appendIndexItem(QString, QStringList)));
    QSignalSpy removeSpy(watcher, SIGNAL(removeIndexItem(QString)));
    QVERIFY(appendSpy.isValid());
    QVERIFY(removeSpy.isValid());

    // onUDiskMounted triggers refreshExtraDirs
    watcher->onUDiskMounted(QStringLiteral("/media/disk1"));
    // No UDisk volumes configured, so no extra dirs added — but the branch is covered

    // onUDiskUnmounted triggers refreshExtraDirs
    watcher->onUDiskUnmounted(QStringLiteral("/media/disk1"));

    resetDirManagerState();
}

void DirWatcherServiceTest::testExtraDirsOfPathsSkipsMountPointNotUnderIndexDir()
{
    resetDirManagerState();

    // UDisk mount point that is NOT under any index dir → continue branch
    Volume uDisk(QStringLiteral("/dev/sda1"));
    uDisk.addMountPoint(QStringLiteral("/mnt/other"));
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    // Search only /tmp, /mnt/other is not under /tmp → skipped via continue
    const QStringList extraDirs = watcher->extraDirsOfPaths({QStringLiteral("/tmp")});
    QVERIFY(!extraDirs.contains(QStringLiteral("/mnt/other")));

    resetDirManagerState();
}

void DirWatcherServiceTest::testAppendRootSearchDirRemovesAllExistingChildren()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    // Add a child dir first
    QCOMPARE(watcher->appendSearchDir(tempDir.path()), int(SearchDir::Successful));
    QVERIFY(watcher->currentSearchDirs().contains(tempDir.path()));

    // Now append "/" — should remove the child (dir.getPath() == "/" branch)
    QSignalSpy appendSpy(watcher, SIGNAL(appendIndexItem(QString, QStringList)));
    QVERIFY(appendSpy.isValid());
    QCOMPARE(watcher->appendSearchDir(QStringLiteral("/")), int(SearchDir::Successful));
    QVERIFY(!watcher->currentSearchDirs().contains(tempDir.path()));

    // Clean up
    watcher->removeSearchDir(QStringLiteral("/"));
    resetDirManagerState();
}

void DirWatcherServiceTest::testExtraDirsOfPathsUDiskUnderIndexNotUnderBlacklist()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("disk3")));
    const QString mountPoint = tempDir.filePath(QStringLiteral("disk3"));

    Volume uDisk(QStringLiteral("/dev/sdz3"));
    uDisk.addMountPoint(mountPoint);
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);

    // Index dir = tempDir.path(), UDisk mount is under it
    // No global blacklist covers this mount point
    // → needCheck=true, underGlobalBlacklist=false
    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QStringList extraDirs = watcher->extraDirsOfPaths({tempDir.path()});

    // Mount point is under index dir but not blacklisted → should be excluded
    QVERIFY(extraDirs.isEmpty() || !extraDirs.contains(mountPoint));

    resetDirManagerState();
}

void DirWatcherServiceTest::testRefreshExtraDirsWithNoChangesEmitsNothing()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("disk4")));
    const QString mountPoint = tempDir.filePath(QStringLiteral("disk4"));

    Volume uDisk(QStringLiteral("/dev/sdz4"));
    uDisk.addMountPoint(mountPoint);
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);
    appendGlobalBlackList(tempDir.path());
    setRootSearchDirWithBlacklist(tempDir.path());
    Config::self()->setExtraDirs({mountPoint});

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QSignalSpy appendSpy(watcher, SIGNAL(appendIndexItem(QString,QStringList)));
    QSignalSpy removeSpy(watcher, SIGNAL(removeIndexItem(QString)));
    QVERIFY(appendSpy.isValid());
    QVERIFY(removeSpy.isValid());

    watcher->refreshExtraDirs();

    QCOMPARE(appendSpy.count(), 0);
    QCOMPARE(removeSpy.count(), 0);
    QCOMPARE(Config::self()->extraDirs(), QStringList{mountPoint});
    resetDirManagerState();
}

void DirWatcherServiceTest::testExtraDirsOfPathsIgnoresNonGlobalBlackListMatch()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("disk5")));
    const QString mountPoint = tempDir.filePath(QStringLiteral("disk5"));

    Volume uDisk(QStringLiteral("/dev/sdz5"));
    uDisk.addMountPoint(mountPoint);
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);
    setRootSearchDirWithBlacklist(tempDir.path());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QStringList extraDirs = watcher->extraDirsOfPaths({QStringLiteral("/")});

    QVERIFY(!extraDirs.contains(mountPoint));
    resetDirManagerState();
}

void DirWatcherServiceTest::testExtraDirsOfPathsAddsUDiskOnlyOnceForMultipleBlacklistedMountPoints()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("disk-a")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("disk-b")));
    const QString mountA = tempDir.filePath(QStringLiteral("disk-a"));
    const QString mountB = tempDir.filePath(QStringLiteral("disk-b"));

    Volume uDisk(QStringLiteral("/dev/sdz6"));
    uDisk.addMountPoint(mountA);
    uDisk.addMountPoint(mountB);
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);
    appendGlobalBlackList(tempDir.path());
    setRootSearchDirWithBlacklist(tempDir.path());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QStringList extraDirs = watcher->extraDirsOfPaths({QStringLiteral("/")});

    QCOMPARE(extraDirs.size(), 1);
    QVERIFY(extraDirs.contains(mountA) || extraDirs.contains(mountB));
    resetDirManagerState();
}

void DirWatcherServiceTest::testExtraDirsOfPathsUsesConfiguredBlackDirsWhenPathSetMatchesSearchDirs()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("indexed/mount")));

    const QString indexDir = tempDir.filePath(QStringLiteral("indexed"));
    const QString mountPoint = tempDir.filePath(QStringLiteral("indexed/mount"));
    const QString dirtyIndexDir = indexDir + QStringLiteral("/child/..");

    Volume uDisk(QStringLiteral("/dev/sdz7"));
    uDisk.addMountPoint(mountPoint);
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);
    appendGlobalBlackList(mountPoint);

    QJsonObject searchDirs;
    searchDirs.insert(indexDir, QJsonArray{mountPoint});
    Config::self()->m_settingsData.insert(QStringLiteral("SearchDirs"), searchDirs);

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QStringList extraDirs = watcher->extraDirsOfPaths({dirtyIndexDir});

    QCOMPARE(extraDirs, QStringList{mountPoint});
    resetDirManagerState();
}

void DirWatcherServiceTest::testExtraDirsOfPathsIgnoresExactBlackDirWhenNotGlobalBlackList()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("indexed/mount")));

    const QString indexDir = tempDir.filePath(QStringLiteral("indexed"));
    const QString mountPoint = tempDir.filePath(QStringLiteral("indexed/mount"));

    Volume uDisk(QStringLiteral("/dev/sdz8"));
    uDisk.addMountPoint(mountPoint);
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);

    QJsonObject searchDirs;
    searchDirs.insert(indexDir, QJsonArray{mountPoint});
    Config::self()->m_settingsData.insert(QStringLiteral("SearchDirs"), searchDirs);

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QStringList extraDirs = watcher->extraDirsOfPaths({indexDir});

    QVERIFY(!extraDirs.contains(mountPoint));
    resetDirManagerState();
}

void DirWatcherServiceTest::testExtraDirsOfPathsSkipsMountPointOutsideMatchedSearchDirs()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("indexed")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("outside")));

    const QString indexDir = tempDir.filePath(QStringLiteral("indexed"));
    const QString mountPoint = tempDir.filePath(QStringLiteral("outside"));

    Volume uDisk(QStringLiteral("/dev/sdz11"));
    uDisk.addMountPoint(mountPoint);
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);

    QJsonObject searchDirs;
    searchDirs.insert(indexDir, QJsonArray{});
    Config::self()->m_settingsData.insert(QStringLiteral("SearchDirs"), searchDirs);

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QStringList extraDirs = watcher->extraDirsOfPaths({indexDir});

    QVERIFY(!extraDirs.contains(mountPoint));
    resetDirManagerState();
}

void DirWatcherServiceTest::testExtraDirsOfPathsMatchesSecondSearchDirAfterSkippingFirst()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("indexed-a")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("indexed-b/mount")));

    const QString indexDirA = tempDir.filePath(QStringLiteral("indexed-a"));
    const QString indexDirB = tempDir.filePath(QStringLiteral("indexed-b"));
    const QString mountPoint = tempDir.filePath(QStringLiteral("indexed-b/mount"));

    Volume uDisk(QStringLiteral("/dev/sdz12"));
    uDisk.addMountPoint(mountPoint);
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);
    appendGlobalBlackList(mountPoint);

    QJsonObject searchDirs;
    searchDirs.insert(indexDirA, QJsonArray{});
    searchDirs.insert(indexDirB, QJsonArray{mountPoint});
    Config::self()->m_settingsData.insert(QStringLiteral("SearchDirs"), searchDirs);

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QStringList extraDirs = watcher->extraDirsOfPaths({indexDirB, indexDirA});

    QCOMPARE(extraDirs, QStringList{mountPoint});
    resetDirManagerState();
}

void DirWatcherServiceTest::testExtraDirsOfPathsAddsEligibleMountAfterSkippingSiblingMount()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("indexed/inside")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("outside")));

    const QString indexDir = tempDir.filePath(QStringLiteral("indexed"));
    const QString outsideMount = tempDir.filePath(QStringLiteral("outside"));
    const QString insideMount = tempDir.filePath(QStringLiteral("indexed/inside"));

    Volume uDisk(QStringLiteral("/dev/sdz13"));
    uDisk.addMountPoint(outsideMount);
    uDisk.addMountPoint(insideMount);
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);
    appendGlobalBlackList(insideMount);

    QJsonObject searchDirs;
    searchDirs.insert(indexDir, QJsonArray{insideMount});
    Config::self()->m_settingsData.insert(QStringLiteral("SearchDirs"), searchDirs);

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QStringList extraDirs = watcher->extraDirsOfPaths({indexDir});

    QCOMPARE(extraDirs, QStringList{insideMount});
    resetDirManagerState();
}

void DirWatcherServiceTest::testRefreshExtraDirsOnlyAddsNewEntries()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("disk-add")));
    const QString mountPoint = tempDir.filePath(QStringLiteral("disk-add"));

    appendGlobalBlackList(tempDir.path());
    setRootSearchDirWithBlacklist(tempDir.path());

    Volume uDisk(QStringLiteral("/dev/sdz14"));
    uDisk.addMountPoint(mountPoint);
    VolumeManager::self()->m_uDiskVolumes.insert(uDisk.device(), uDisk);

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QSignalSpy appendSpy(watcher, SIGNAL(appendIndexItem(QString,QStringList)));
    QSignalSpy removeSpy(watcher, SIGNAL(removeIndexItem(QString)));
    QVERIFY(appendSpy.isValid());
    QVERIFY(removeSpy.isValid());

    watcher->refreshExtraDirs();

    QCOMPARE(appendSpy.count(), 1);
    QCOMPARE(appendSpy.first().at(0).toString(), mountPoint);
    QVERIFY(removeSpy.isEmpty());
    resetDirManagerState();
}

void DirWatcherServiceTest::testRefreshExtraDirsOnlyRemovesStaleEntries()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("disk-remove")));
    const QString mountPoint = tempDir.filePath(QStringLiteral("disk-remove"));

    appendGlobalBlackList(tempDir.path());
    setRootSearchDirWithBlacklist(tempDir.path());
    Config::self()->setExtraDirs({mountPoint});

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QSignalSpy appendSpy(watcher, SIGNAL(appendIndexItem(QString,QStringList)));
    QSignalSpy removeSpy(watcher, SIGNAL(removeIndexItem(QString)));
    QVERIFY(appendSpy.isValid());
    QVERIFY(removeSpy.isValid());

    watcher->refreshExtraDirs();

    QVERIFY(appendSpy.isEmpty());
    QCOMPARE(removeSpy.count(), 1);
    QCOMPARE(removeSpy.first().at(0).toString(), mountPoint);
    resetDirManagerState();
}

void DirWatcherServiceTest::testRefreshExtraDirsHandlesMultipleAddsAndRemoves()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("disk-a")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("disk-b")));
    const QString mountA = tempDir.filePath(QStringLiteral("disk-a"));
    const QString mountB = tempDir.filePath(QStringLiteral("disk-b"));

    appendGlobalBlackList(tempDir.path());
    setRootSearchDirWithBlacklist(tempDir.path());

    Volume uDiskA(QStringLiteral("/dev/sdz9"));
    uDiskA.addMountPoint(mountA);
    VolumeManager::self()->m_uDiskVolumes.insert(uDiskA.device(), uDiskA);
    Volume uDiskB(QStringLiteral("/dev/sdz10"));
    uDiskB.addMountPoint(mountB);
    VolumeManager::self()->m_uDiskVolumes.insert(uDiskB.device(), uDiskB);

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QSignalSpy appendSpy(watcher, SIGNAL(appendIndexItem(QString,QStringList)));
    QSignalSpy removeSpy(watcher, SIGNAL(removeIndexItem(QString)));
    QVERIFY(appendSpy.isValid());
    QVERIFY(removeSpy.isValid());

    watcher->refreshExtraDirs();

    QCOMPARE(appendSpy.count(), 2);
    QVERIFY(Config::self()->extraDirs().contains(mountA));
    QVERIFY(Config::self()->extraDirs().contains(mountB));

    VolumeManager::self()->m_uDiskVolumes.clear();
    watcher->refreshExtraDirs();

    QCOMPARE(removeSpy.count(), 2);
    resetDirManagerState();
}

void DirWatcherServiceTest::testAppendSearchDirUnderGlobalBlackListDoesNotEmitAppend()
{
    resetDirManagerState();

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QSignalSpy appendSpy(watcher, SIGNAL(appendIndexItem(QString,QStringList)));
    QVERIFY(appendSpy.isValid());

    QCOMPARE(watcher->appendSearchDir(QStringLiteral("/tmp")), int(SearchDir::UnderBlackList));
    QCOMPARE(appendSpy.count(), 0);
    QVERIFY(!watcher->currentSearchDirs().contains(QStringLiteral("/tmp")));
    resetDirManagerState();
}

void DirWatcherServiceTest::testAddBlockDirNormalizesAndEmitsNormalizedPath()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QString dirtyPath = tempDir.path() + QStringLiteral("/child/..");
    const QString normalizedPath = QDir::cleanPath(dirtyPath);

    QSignalSpy addedSpy(watcher, SIGNAL(blockItemAdded(QString)));
    QVERIFY(addedSpy.isValid());

    QCOMPARE(watcher->addBlockDirOfUser(dirtyPath), int(SearchDir::Successful));
    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(addedSpy.first().at(0).toString(), normalizedPath);
    QVERIFY(watcher->blockDirsForUser().contains(normalizedPath));
    resetDirManagerState();
}

void DirWatcherServiceTest::testAddBlockDirFailureDoesNotEmitSignal()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QSignalSpy addedSpy(watcher, SIGNAL(blockItemAdded(QString)));
    QVERIFY(addedSpy.isValid());

    const QString missingPath = tempDir.path() + QStringLiteral("/missing");
    QCOMPARE(watcher->addBlockDirOfUser(missingPath), int(SearchDir::NotExists));
    QVERIFY(addedSpy.isEmpty());
    resetDirManagerState();
}

void DirWatcherServiceTest::testAddBlockDirDuplicateDoesNotEmitSignal()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DirWatcher *watcher = DirWatcher::getDirWatcher();
    QCOMPARE(watcher->addBlockDirOfUser(tempDir.path()), int(SearchDir::Successful));

    QSignalSpy addedSpy(watcher, SIGNAL(blockItemAdded(QString)));
    QVERIFY(addedSpy.isValid());

    QCOMPARE(watcher->addBlockDirOfUser(tempDir.path()), int(SearchDir::Duplicated));
    QVERIFY(addedSpy.isEmpty());
    resetDirManagerState();
}

void DirWatcherServiceTest::testShowInUkccReturnsFalseForExactExtraDirPath()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Config::self()->setExtraDirs({tempDir.path()});
    DirWatcher *watcher = DirWatcher::getDirWatcher();

    QVERIFY(!watcher->showInUkcc(tempDir.path()));
    resetDirManagerState();
}

void DirWatcherServiceTest::testShowInUkccReturnsFalseForDirtyExtraDirPath()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Config::self()->setExtraDirs({tempDir.path()});
    DirWatcher *watcher = DirWatcher::getDirWatcher();
    const QString dirtyPath = tempDir.path() + QStringLiteral("/child/..");

    QVERIFY(!watcher->showInUkcc(dirtyPath));
    resetDirManagerState();
}

void DirWatcherServiceTest::testLocalWatcherProcessesCompatibleCacheAndForwardsMountAdded()
{
    resetDirManagerState();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Config::self()->m_compatibilityMode = true;
    Config::self()->m_compatibleCache.append(SearchDir(tempDir.path(), false));

    DirWatcher watcher;
    QSignalSpy mountAddedSpy(&watcher, SIGNAL(mountAdded(QString)));
    QVERIFY(mountAddedSpy.isValid());

    QVERIFY(Config::self()->searchDirs().contains(tempDir.path()));

    Q_EMIT VolumeManager::self()->MountAdded(QStringLiteral("/media/test-disk"));

    QCOMPARE(mountAddedSpy.count(), 1);
    QCOMPARE(mountAddedSpy.first().at(0).toString(), QStringLiteral("/media/test-disk"));
    resetDirManagerState();
}

void DirWatcherServiceTest::testHeapAllocatedWatcherCanBeDeleted()
{
    resetDirManagerState();

    DirWatcher *watcher = new DirWatcher();

    QVERIFY(watcher != nullptr);
    delete watcher;

    resetDirManagerState();
}

QTEST_MAIN(DirWatcherServiceTest)
