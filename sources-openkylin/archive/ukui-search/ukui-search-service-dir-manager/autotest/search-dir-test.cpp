#include "search-dir-test.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

#define private public
#include "config.h"
#undef private

#define private public
#include "search-dir.h"
#undef private

#define private public
#include "volume-manager.h"
#undef private

void setForcedDuplicatesForTest(const QVector<QStringList> &duplicates);
void clearForcedDuplicatesForTest();

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

void resetGlobalConfig()
{
    Config::self()->m_settingsData = baseSettings();
    Config::self()->m_compatibilityMode = false;
    Config::self()->m_compatibleCache.clear();
}

void resetVolumeManager()
{
    VolumeManager::self()->m_volumes.clear();
    VolumeManager::self()->m_uDiskVolumes.clear();
    clearForcedDuplicatesForTest();
}

void appendGlobalBlackList(const QString &path)
{
    QJsonObject globalSettings = Config::self()->m_settingsData.value(QStringLiteral("GlobalSettings")).toObject();
    QJsonArray globalBlackList = globalSettings.value(QStringLiteral("GlobalBlackList")).toArray();
    globalBlackList.append(path);
    globalSettings.insert(QStringLiteral("GlobalBlackList"), globalBlackList);
    Config::self()->m_settingsData.insert(QStringLiteral("GlobalSettings"), globalSettings);
}

void setSearchDirs(const QStringList &paths)
{
    QJsonObject searchDirs;
    for (const QString &path : paths) {
        searchDirs.insert(path, QJsonArray{});
    }
    Config::self()->m_settingsData.insert(QStringLiteral("SearchDirs"), searchDirs);
}

} // namespace

void SearchDirTest::testNonExistingPath()
{
    SearchDir dir(QStringLiteral("/tmp/ukui-search-this-path-should-not-exist"), false);

    QCOMPARE(dir.error(), SearchDir::NotExists);
    QVERIFY(dir.getPath().isEmpty());
    QCOMPARE(dir.errorString(), QStringLiteral("Path is not exists"));
}

void SearchDirTest::testExistingPathWithoutGeneratedBlacklist()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SearchDir dir(tempDir.path() + QStringLiteral("/../") + QFileInfo(tempDir.path()).fileName(), false);

    QCOMPARE(dir.error(), SearchDir::Successful);
    QCOMPARE(dir.getPath(), QDir::cleanPath(tempDir.path() + QStringLiteral("/../") + QFileInfo(tempDir.path()).fileName()));
    QVERIFY(dir.getBlackList().isEmpty());
    QCOMPARE(dir.errorString(), QStringLiteral("Add search dir successful"));
}

void SearchDirTest::testBlackListAccessorsAndEquality()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SearchDir lhs(tempDir.path(), false);
    SearchDir rhs(tempDir.path(), false);
    SearchDir other(QDir::tempPath(), false);

    lhs.setBlackList({QStringLiteral("/tmp/a"), QStringLiteral("/tmp/b")});

    QVERIFY(lhs == rhs);
    QVERIFY(!(lhs == other));
    QCOMPARE(lhs.getBlackList(), QStringList({QStringLiteral("/tmp/a"), QStringLiteral("/tmp/b")}));
}

void SearchDirTest::testErrorStrings()
{
    SearchDir dir;

    dir.m_error = SearchDir::Successful;
    QCOMPARE(dir.errorString(), QStringLiteral("Add search dir successful"));
    dir.m_error = SearchDir::Duplicated;
    QCOMPARE(dir.errorString(), QStringLiteral("Path or its parent dir has been added"));
    dir.m_error = SearchDir::UnderBlackList;
    QCOMPARE(dir.errorString(), QStringLiteral("Path is or under blacklist"));
    dir.m_error = SearchDir::RepeatMount1;
    QCOMPARE(dir.errorString(), QStringLiteral("Path is in repeat mounted devices and another path which is in the same device has been indexed"));
    dir.m_error = SearchDir::RepeatMount2;
    QCOMPARE(dir.errorString(), QStringLiteral("Another path which is in the same device has been indexed"));
    dir.m_error = SearchDir::NotExists;
    QCOMPARE(dir.errorString(), QStringLiteral("Path is not exists"));
}

void SearchDirTest::testErrorStringReturnsEmptyForUnknownError()
{
    SearchDir dir;

    dir.m_error = static_cast<SearchDir::ErrorInfo>(-1);

    QVERIFY(dir.errorString().isEmpty());
}

void SearchDirTest::testGeneratedBlackListRejectsGlobalBlackListPath()
{
    resetGlobalConfig();

    SearchDir dir(QStringLiteral("/tmp"));

    QCOMPARE(dir.error(), SearchDir::UnderBlackList);
}

void SearchDirTest::testGeneratedBlackListRejectsExistingSearchDir()
{
    resetGlobalConfig();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    setSearchDirs({tempDir.path()});

    SearchDir dir(tempDir.path());

    QCOMPARE(dir.error(), SearchDir::Duplicated);
}

void SearchDirTest::testGeneratedBlackListAllowsUDiskPathUnderMediaSymlinkTarget()
{
    resetGlobalConfig();
    resetVolumeManager();

    QFileInfo uDiskPathInfo(QStringLiteral("/media"));
    if (!uDiskPathInfo.isSymLink()) {
        QSKIP("/media is not a symlink on this platform");
    }

    const QString uDiskPath = uDiskPathInfo.symLinkTarget();
    if (!QFile::exists(uDiskPath)) {
        QSKIP("The /media symlink target does not exist");
    }

    SearchDir dir(uDiskPath);

    QCOMPARE(dir.error(), SearchDir::Successful);
    QCOMPARE(dir.getPath(), uDiskPath);
}

void SearchDirTest::testGenerateBlackListClearsPreviousEntries()
{
    resetGlobalConfig();

    SearchDir dir(QStringLiteral("/tmp"), false);
    dir.setBlackList({QStringLiteral("/stale")});

    dir.generateBlackList();

    QCOMPARE(dir.error(), SearchDir::UnderBlackList);
    QVERIFY(dir.getBlackList().isEmpty());
}

void SearchDirTest::testBlackListOfRootIncludesGlobalBlackList()
{
    resetGlobalConfig();

    const QStringList blackList = SearchDir::blackListOfDir(QStringLiteral("/"));

    QVERIFY(blackList.contains(QStringLiteral("/proc")));
    QVERIFY(blackList.contains(QStringLiteral("/sysroot")));
}

void SearchDirTest::testBlackListOfDirIncludesNestedGlobalBlackList()
{
    resetGlobalConfig();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("parent/blocked")));

    const QString parent = tempDir.filePath(QStringLiteral("parent"));
    const QString blocked = tempDir.filePath(QStringLiteral("parent/blocked"));
    appendGlobalBlackList(blocked);

    const QStringList blackList = SearchDir::blackListOfDir(parent);

    QVERIFY(blackList.contains(blocked));
}

void SearchDirTest::testGeneratedBlackListContainsChildGlobalBlackList()
{
    resetGlobalConfig();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("parent/blocked")));

    const QString parent = tempDir.filePath(QStringLiteral("parent"));
    const QString blocked = tempDir.filePath(QStringLiteral("parent/blocked"));
    appendGlobalBlackList(blocked);

    SearchDir dir(parent);

    QCOMPARE(dir.error(), SearchDir::Successful);
    QVERIFY(dir.getBlackList().contains(blocked));
}

void SearchDirTest::testGeneratedRootBlackListContainsConfiguredSearchDirs()
{
    resetGlobalConfig();
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("indexed")));
    const QString indexed = tempDir.filePath(QStringLiteral("indexed"));
    setSearchDirs({indexed});

    SearchDir root(QStringLiteral("/"));

    QCOMPARE(root.error(), SearchDir::Successful);
    QVERIFY(root.getBlackList().contains(indexed));
    QVERIFY(root.getBlackList().contains(QStringLiteral("/proc")));
}

void SearchDirTest::testGeneratedRootBlackListHandlesDuplicateMountContainingRoot()
{
    resetGlobalConfig();
    resetVolumeManager();

    Volume volume(QStringLiteral("/dev/whitebox-root-duplicate"));
    volume.addMountPoint(QStringLiteral("/"));
    volume.addMountPoint(QStringLiteral("/mnt/root-copy"));
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);

    SearchDir root(QStringLiteral("/"));

    QCOMPARE(root.error(), SearchDir::Successful);
    QVERIFY(root.getBlackList().contains(QStringLiteral("/mnt/root-copy")));
    resetVolumeManager();
}

void SearchDirTest::testGeneratedRootBlackListHandlesDuplicateMountSearchDirPolicies()
{
    resetGlobalConfig();
    resetVolumeManager();

    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/root-repeat-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("mnt-a/docs")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("mnt-b/docs")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("indexed-parent")));

    const QString mountA = tempDir.filePath(QStringLiteral("mnt-a"));
    const QString mountB = tempDir.filePath(QStringLiteral("mnt-b"));
    Volume volume(QStringLiteral("/dev/whitebox-root-repeat"));
    volume.addMountPoint(mountA);
    volume.addMountPoint(mountB);
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);

    setSearchDirs({mountA + QStringLiteral("/docs")});
    SearchDir keepIndexedDuplicate(QStringLiteral("/"));
    QCOMPARE(keepIndexedDuplicate.error(), SearchDir::Successful);
    QVERIFY(!keepIndexedDuplicate.getBlackList().contains(mountA));
    QVERIFY(keepIndexedDuplicate.getBlackList().contains(mountB));

    setSearchDirs({tempDir.filePath(QStringLiteral("indexed-parent"))});
    SearchDir excludeDefaultDuplicate(QStringLiteral("/"));
    QCOMPARE(excludeDefaultDuplicate.error(), SearchDir::Successful);
    QVERIFY(excludeDefaultDuplicate.getBlackList().contains(mountB));

    setSearchDirs({tempDir.path()});
    SearchDir excludeAllUnderIndexedParent(QStringLiteral("/"));
    QCOMPARE(excludeAllUnderIndexedParent.error(), SearchDir::Successful);
    QVERIFY(excludeAllUnderIndexedParent.getBlackList().contains(mountA));
    QVERIFY(excludeAllUnderIndexedParent.getBlackList().contains(mountB));

    resetVolumeManager();
}

void SearchDirTest::testBlackListOfDirHandlesNestedDuplicateMounts()
{
    resetGlobalConfig();
    resetVolumeManager();

    Volume volume(QStringLiteral("/dev/whitebox-nested-duplicate"));
    volume.addMountPoint(QStringLiteral("/data/child"));
    volume.addMountPoint(QStringLiteral("/data/child/grandchild"));
    volume.addMountPoint(QStringLiteral("/data/sibling"));
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);

    const QStringList blackList = SearchDir::blackListOfDir(QStringLiteral("/data"));

    QVERIFY(blackList.contains(QStringLiteral("/data/child/grandchild")));
    QVERIFY(blackList.contains(QStringLiteral("/data/sibling")));
    resetVolumeManager();
}

void SearchDirTest::testBlackListOfDirHandlesForcedChildBeforeParentDuplicates()
{
    resetGlobalConfig();
    resetVolumeManager();

    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/forced-dup-blacklist-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/mount/nested")));

    const QString parentMount = tempDir.filePath(QStringLiteral("data/mount"));
    const QString childMount = tempDir.filePath(QStringLiteral("data/mount/nested"));
    setForcedDuplicatesForTest({QStringList{childMount, parentMount}});

    const QStringList blackList = SearchDir::blackListOfDir(tempDir.filePath(QStringLiteral("data")));

    QVERIFY(blackList.contains(childMount));
    resetVolumeManager();
}

void SearchDirTest::testGeneratedBlackListHandlesRepeatMountErrorBranches()
{
    resetGlobalConfig();
    resetVolumeManager();

    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/repeat-mount-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("mnt-a/docs")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("mnt-b/docs")));

    const QString mountA = tempDir.filePath(QStringLiteral("mnt-a"));
    const QString mountB = tempDir.filePath(QStringLiteral("mnt-b"));
    const QString addedUnderA = mountA + QStringLiteral("/docs");
    const QString sameUnderB = mountB + QStringLiteral("/docs");

    Volume volume(QStringLiteral("/dev/whitebox-repeat"));
    volume.addMountPoint(mountA);
    volume.addMountPoint(mountB);
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);
    setSearchDirs({addedUnderA});

    SearchDir repeatSamePath(sameUnderB);
    QCOMPARE(repeatSamePath.error(), SearchDir::RepeatMount2);

    SearchDir mountParent(mountB);
    QCOMPARE(mountParent.error(), SearchDir::Successful);
    QVERIFY(mountParent.getBlackList().contains(sameUnderB));

    resetVolumeManager();
}

void SearchDirTest::testGeneratedBlackListHandlesForcedChildBeforeParentDuplicates()
{
    resetGlobalConfig();
    resetVolumeManager();

    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/forced-dup-generate-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/mount/nested")));

    const QString dataDir = tempDir.filePath(QStringLiteral("data"));
    const QString parentMount = tempDir.filePath(QStringLiteral("data/mount"));
    const QString childMount = tempDir.filePath(QStringLiteral("data/mount/nested"));
    setForcedDuplicatesForTest({QStringList{childMount, parentMount}});

    SearchDir dir(dataDir);

    QCOMPARE(dir.error(), SearchDir::Successful);
    QVERIFY(dir.getBlackList().contains(childMount));
    resetVolumeManager();
}

void SearchDirTest::testGeneratedBlackListHandlesSubVolumeMirrorBranches()
{
    resetGlobalConfig();
    resetVolumeManager();

    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/subvol-repeat-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("home/user/docs")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/@home/user/docs")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/projects")));

    const QString homeMount = tempDir.filePath(QStringLiteral("home"));
    const QString dataMount = tempDir.filePath(QStringLiteral("data"));
    const QString dataHome = dataMount + QStringLiteral("/@home");
    const QString homeDocs = homeMount + QStringLiteral("/user/docs");
    const QString dataHomeDocs = dataHome + QStringLiteral("/user/docs");

    Volume volume(QStringLiteral("/dev/whitebox-subvol-generate"));
    volume.addMountPoint(homeMount, QStringLiteral("/@home"));
    volume.addMountPoint(dataMount);
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);

    setSearchDirs({homeMount});
    SearchDir parentOfSpec(dataMount);
    QCOMPARE(parentOfSpec.error(), SearchDir::Successful);
    QVERIFY(parentOfSpec.getBlackList().contains(dataHome));

    setSearchDirs({homeDocs});
    SearchDir parentOfSpecWithChild(dataMount);
    QCOMPARE(parentOfSpecWithChild.error(), SearchDir::Successful);
    QVERIFY(parentOfSpecWithChild.getBlackList().contains(dataHomeDocs));

    setSearchDirs({homeMount});
    SearchDir specPath(dataHome);
    QCOMPARE(specPath.error(), SearchDir::RepeatMount1);

    setSearchDirs({homeDocs});
    SearchDir sameSpecChild(dataHomeDocs);
    QCOMPARE(sameSpecChild.error(), SearchDir::RepeatMount2);

    setSearchDirs({dataHome});
    SearchDir mountPath(homeMount);
    QCOMPARE(mountPath.error(), SearchDir::RepeatMount1);

    setSearchDirs({dataHomeDocs});
    SearchDir homeParent(homeMount + QStringLiteral("/user"));
    QCOMPARE(homeParent.error(), SearchDir::Successful);
    QVERIFY(homeParent.getBlackList().contains(homeDocs));

    resetVolumeManager();
}

void SearchDirTest::testGeneratedBlackListRejectsChildOfExistingSearchDir()
{
    resetGlobalConfig();
    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/search-dir-parent-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("child")));

    Config::self()->addDir(SearchDir(tempDir.path(), false));

    SearchDir child(tempDir.path() + QStringLiteral("/child"));

    QCOMPARE(child.error(), SearchDir::Duplicated);
    Config::self()->removeDir(tempDir.path());
}

void SearchDirTest::testGeneratedBlackListContainsExistingChildSearchDir()
{
    resetGlobalConfig();
    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/search-dir-child-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("parent/child")));

    const QString parent = tempDir.path() + QStringLiteral("/parent");
    const QString child = parent + QStringLiteral("/child");
    Config::self()->addDir(SearchDir(child, false));

    SearchDir dir(parent);

    QCOMPARE(dir.error(), SearchDir::Successful);
    QVERIFY(dir.getBlackList().contains(child));
    Config::self()->removeDir(child);
}

void SearchDirTest::testHandleSubVolumesForRootAndMirrorPaths()
{
    resetGlobalConfig();
    resetVolumeManager();

    Volume volume(QStringLiteral("/dev/whitebox-subvol"));
    volume.addMountPoint(QStringLiteral("/home"), QStringLiteral("/@home"));
    volume.addMountPoint(QStringLiteral("/data"));
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);

    const QStringList rootBlackList = SearchDir::handleSubVolumes4SingleDir(QStringLiteral("/"));
    QVERIFY(rootBlackList.contains(QStringLiteral("/data/@home")));

    const QStringList subVolumePathBlackList = SearchDir::handleSubVolumes4SingleDir(QStringLiteral("/home/user/docs"));
    QVERIFY(subVolumePathBlackList.contains(QStringLiteral("/data/@home/user/docs")));

    const QStringList mirrorPathBlackList = SearchDir::handleSubVolumes4SingleDir(QStringLiteral("/data/@home/user/docs"));
    QVERIFY(mirrorPathBlackList.contains(QStringLiteral("/home/user/docs")));

    resetVolumeManager();
}

void SearchDirTest::testHandleSubVolumesAddsSpecBelowParentSearchDir()
{
    resetGlobalConfig();
    resetVolumeManager();

    Volume volume(QStringLiteral("/dev/whitebox-parent-subvol"));
    volume.addMountPoint(QStringLiteral("/data/home"), QStringLiteral("/@home"));
    volume.addMountPoint(QStringLiteral("/data"));
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);

    const QStringList blackList = SearchDir::handleSubVolumes4SingleDir(QStringLiteral("/data"));

    QVERIFY(blackList.contains(QStringLiteral("/data/@home")));
    resetVolumeManager();
}

void SearchDirTest::testHandleSubVolumesReturnsEmptyForUnrelatedPath()
{
    resetGlobalConfig();
    resetVolumeManager();

    Volume volume(QStringLiteral("/dev/whitebox-unrelated-subvol"));
    volume.addMountPoint(QStringLiteral("/home"), QStringLiteral("/@home"));
    volume.addMountPoint(QStringLiteral("/data"));
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);

    QVERIFY(SearchDir::handleSubVolumes4SingleDir(QStringLiteral("/opt/project")).isEmpty());
    resetVolumeManager();
}

void SearchDirTest::testNestedDuplicateMountsParentKeptChildBlacklisted()
{
    resetGlobalConfig();
    resetVolumeManager();

    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/nested-dup-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("mnt")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("mnt/sub")));

    const QString parentMount = tempDir.filePath(QStringLiteral("mnt"));
    const QString childMount = tempDir.filePath(QStringLiteral("mnt/sub"));

    // Both mount points on same device: parent is kept, child is blacklisted
    Volume volume(QStringLiteral("/dev/whitebox-nested-dup"));
    volume.addMountPoint(parentMount);
    volume.addMountPoint(childMount);
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);

    SearchDir dir(parentMount);
    QCOMPARE(dir.error(), SearchDir::Successful);
    // Child mount should be blacklisted since device is same
    QVERIFY(dir.getBlackList().contains(childMount));

    resetVolumeManager();
}

void SearchDirTest::testGeneratedBlackListBlacklistsRepeatedNestedMountUnderIndexedMount()
{
    resetGlobalConfig();
    resetVolumeManager();

    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/repeat-nested-parent-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/mount")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/mount/nested")));

    const QString parent = tempDir.filePath(QStringLiteral("data"));
    const QString mount = tempDir.filePath(QStringLiteral("data/mount"));
    const QString nestedMount = tempDir.filePath(QStringLiteral("data/mount/nested"));

    Volume volume(QStringLiteral("/dev/whitebox-repeat-nested-parent"));
    volume.addMountPoint(mount);
    volume.addMountPoint(nestedMount);
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);
    setSearchDirs({mount});

    SearchDir dir(parent);

    QCOMPARE(dir.error(), SearchDir::Successful);
    QVERIFY(dir.getBlackList().contains(nestedMount));
    resetVolumeManager();
}

void SearchDirTest::testGeneratedBlackListBlacklistsIndexedPathMappedFromRepeatedNestedMount()
{
    resetGlobalConfig();
    resetVolumeManager();

    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/repeat-nested-mapped-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/mount/docs")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/mount/nested/docs")));

    const QString parent = tempDir.filePath(QStringLiteral("data"));
    const QString mount = tempDir.filePath(QStringLiteral("data/mount"));
    const QString nestedMount = tempDir.filePath(QStringLiteral("data/mount/nested"));
    const QString indexedPath = mount + QStringLiteral("/docs");

    Volume volume(QStringLiteral("/dev/whitebox-repeat-nested-mapped"));
    volume.addMountPoint(mount);
    volume.addMountPoint(nestedMount);
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);
    setSearchDirs({indexedPath});

    SearchDir dir(parent);

    QCOMPARE(dir.error(), SearchDir::Successful);
    QVERIFY(dir.getBlackList().contains(indexedPath));
    resetVolumeManager();
}

void SearchDirTest::testGeneratedBlackListRejectsNestedRepeatedMountWhenParentMountIndexed()
{
    resetGlobalConfig();
    resetVolumeManager();

    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/repeat-nested-indexed-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("mnt-a")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("indexed-parent/mnt-b")));

    const QString mountA = tempDir.filePath(QStringLiteral("mnt-a"));
    const QString mountB = tempDir.filePath(QStringLiteral("indexed-parent/mnt-b"));
    const QString indexedParentOfSibling = tempDir.filePath(QStringLiteral("indexed-parent"));

    Volume volume(QStringLiteral("/dev/whitebox-repeat-nested-indexed"));
    volume.addMountPoint(mountA);
    volume.addMountPoint(mountB);
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);
    setSearchDirs({indexedParentOfSibling});

    SearchDir dir(mountA);

    QCOMPARE(dir.error(), SearchDir::RepeatMount1);
    resetVolumeManager();
}

void SearchDirTest::testGeneratedBlackListBlacklistsRepeatedMountUnderIndexedAncestor()
{
    resetGlobalConfig();
    resetVolumeManager();

    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/repeat-indexed-ancestor-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/indexed")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/indexed/mount")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/other")));

    const QString parent = tempDir.filePath(QStringLiteral("data"));
    const QString indexedAncestor = tempDir.filePath(QStringLiteral("data/indexed"));
    const QString repeatedMount = tempDir.filePath(QStringLiteral("data/indexed/mount"));
    const QString siblingMount = tempDir.filePath(QStringLiteral("data/other"));

    Volume volume(QStringLiteral("/dev/whitebox-repeat-indexed-ancestor"));
    volume.addMountPoint(repeatedMount);
    volume.addMountPoint(siblingMount);
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);
    setSearchDirs({indexedAncestor});

    SearchDir dir(parent);

    QCOMPARE(dir.error(), SearchDir::Successful);
    QVERIFY(dir.getBlackList().contains(repeatedMount));
    resetVolumeManager();
}

void SearchDirTest::testGeneratedBlackListBlacklistsMappedSubVolumeChildUnderSpecPath()
{
    resetGlobalConfig();
    resetVolumeManager();

    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/subvol-mapped-child-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("home/user/docs")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/@home/user/docs")));

    const QString homeMount = tempDir.filePath(QStringLiteral("home"));
    const QString dataMount = tempDir.filePath(QStringLiteral("data"));
    const QString specParent = dataMount + QStringLiteral("/@home/user");
    const QString mappedChild = specParent + QStringLiteral("/docs");

    Volume volume(QStringLiteral("/dev/whitebox-subvol-mapped-child"));
    volume.addMountPoint(homeMount, QStringLiteral("/@home"));
    volume.addMountPoint(dataMount);
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);
    setSearchDirs({homeMount + QStringLiteral("/user/docs")});

    SearchDir dir(specParent);

    QCOMPARE(dir.error(), SearchDir::Successful);
    QVERIFY(dir.getBlackList().contains(mappedChild));
    resetVolumeManager();
}

void SearchDirTest::testGeneratedBlackListRejectsMappedMountChildUnderSubVolumeMirror()
{
    resetGlobalConfig();
    resetVolumeManager();

    QTemporaryDir tempDir(QDir::currentPath() + QStringLiteral("/subvol-mapped-repeat-XXXXXX"));
    QVERIFY(tempDir.isValid());
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("home/user/docs")));
    QVERIFY(QDir(tempDir.path()).mkpath(QStringLiteral("data/@home/user/docs")));

    const QString homeMount = tempDir.filePath(QStringLiteral("home"));
    const QString dataMount = tempDir.filePath(QStringLiteral("data"));
    const QString homeDocs = homeMount + QStringLiteral("/user/docs");

    Volume volume(QStringLiteral("/dev/whitebox-subvol-mapped-repeat"));
    volume.addMountPoint(homeMount, QStringLiteral("/@home"));
    volume.addMountPoint(dataMount);
    VolumeManager::self()->m_volumes.insert(volume.device(), volume);
    setSearchDirs({dataMount + QStringLiteral("/@home/user/docs")});

    SearchDir dir(homeDocs);

    QCOMPARE(dir.error(), SearchDir::RepeatMount2);
    resetVolumeManager();
}

QTEST_MAIN(SearchDirTest)
