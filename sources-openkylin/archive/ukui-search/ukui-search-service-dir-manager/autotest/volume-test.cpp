#include "volume-test.h"

#include <QSignalSpy>
#include <QStorageInfo>
#include <QTest>

#define private public
#include "volume-manager.h"
#undef private

extern "C" {
void ukui_test_gio_reset(void);
void ukui_test_gio_clear_volumes(void);
void ukui_test_gio_set_monitor_available(int available);
void ukui_test_gio_add_volume(const char *device, int can_eject, int drive_can_stop);
GMount *ukui_test_gio_create_mount(const char *uri, int root_available);
}

namespace {

QString findMountedBlockDevice()
{
    for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
        if (!storage.isValid() || !storage.isReady()) {
            continue;
        }
        const QString device = storage.device();
        if (device.contains(QStringLiteral("/dev/sd")) ||
            device.contains(QStringLiteral("/dev/nvme"))) {
            return device;
        }
    }
    return QString();
}

} // namespace

void VolumeTest::testDefaultAndDeviceAccessors()
{
    Volume volume;
    QVERIFY(volume.device().isEmpty());

    volume.setDevice(QStringLiteral("/dev/sda1"));
    QCOMPARE(volume.device(), QStringLiteral("/dev/sda1"));
}

void VolumeTest::testDuplicateMountPointsOnlyReturnedWhenMoreThanOne()
{
    Volume single(QStringLiteral("/dev/sda1"));
    single.addMountPoint(QStringLiteral("/"));
    QVERIFY(single.duplicatesMountPoints().isEmpty());

    Volume duplicated(QStringLiteral("/dev/sdb1"));
    duplicated.addMountPoint(QStringLiteral("/mnt/a"));
    duplicated.addMountPoint(QStringLiteral("/mnt/b"));
    QCOMPARE(duplicated.duplicatesMountPoints(), QStringList({QStringLiteral("/mnt/a"), QStringLiteral("/mnt/b")}));
}

void VolumeTest::testSubVolumesAndMountPointsAccessors()
{
    Volume volume(QStringLiteral("/dev/sdc1"));
    volume.addMountPoint(QStringLiteral("/mnt/root"));
    volume.addMountPoint(QStringLiteral("/mnt/sub"), QStringLiteral("/subvol"));

    QCOMPARE(volume.mountPoints().size(), 2);
    QVERIFY(volume.mountPoints().contains(QStringLiteral("/mnt/root")));
    QVERIFY(volume.mountPoints().contains(QStringLiteral("/mnt/sub")));
    QCOMPARE(volume.subVolumes().value(QStringLiteral("/mnt/sub")), QStringLiteral("/subvol"));
    QVERIFY(!volume.subVolumes().contains(QStringLiteral("/mnt/root")));
}

void VolumeTest::testVolumeManagerFiltersDuplicatesAndSubVolumes()
{
    VolumeManager manager;
    manager.m_volumes.clear();
    manager.m_uDiskVolumes.clear();

    Volume duplicate(QStringLiteral("/dev/sdb1"));
    duplicate.addMountPoint(QStringLiteral("/mnt/a"));
    duplicate.addMountPoint(QStringLiteral("/mnt/b"));

    Volume subVolume(QStringLiteral("/dev/sdc1"));
    subVolume.addMountPoint(QStringLiteral("/mnt/root"), QStringLiteral("/subvol-root"));

    Volume single(QStringLiteral("/dev/sdd1"));
    single.addMountPoint(QStringLiteral("/mnt/only"));

    manager.m_volumes.insert(QStringLiteral("/dev/sdb1"), duplicate);
    manager.m_volumes.insert(QStringLiteral("/dev/sdc1"), subVolume);
    manager.m_volumes.insert(QStringLiteral("/dev/sdd1"), single);
    manager.m_uDiskVolumes.insert(QStringLiteral("/dev/sdc1"), subVolume);

    const QVector<QStringList> duplicates = manager.getDuplicates();
    QCOMPARE(duplicates.size(), 1);
    QCOMPARE(duplicates.first(), QStringList({QStringLiteral("/mnt/a"), QStringLiteral("/mnt/b")}));

    const QVector<Volume> subVolumes = manager.volumesHaveSubVolumes();
    QCOMPARE(subVolumes.size(), 1);
    Volume firstSubVolume = subVolumes.first();
    QCOMPARE(firstSubVolume.device(), QStringLiteral("/dev/sdc1"));
    QCOMPARE(firstSubVolume.subVolumes().value(QStringLiteral("/mnt/root")), QStringLiteral("/subvol-root"));

    const QVector<Volume> volumes = manager.volumes();
    QCOMPARE(volumes.size(), 3);

    const QVector<Volume> uDiskVolumes = manager.uDiskVolumes();
    QCOMPARE(uDiskVolumes.size(), 1);
    Volume firstUDiskVolume = uDiskVolumes.first();
    QCOMPARE(firstUDiskVolume.device(), QStringLiteral("/dev/sdc1"));
}

void VolumeTest::testSubVolumeMountPointsAreNotDuplicateMounts()
{
    Volume volume(QStringLiteral("/dev/sda1"));
    volume.addMountPoint(QStringLiteral("/mnt/root"), QStringLiteral("/@"));
    volume.addMountPoint(QStringLiteral("/mnt/home"), QStringLiteral("/@home"));

    QVERIFY(volume.duplicatesMountPoints().isEmpty());
    QCOMPARE(volume.subVolumes().size(), 2);
}

void VolumeTest::testMountPointOverwriteUpdatesSubVolumeState()
{
    Volume volume(QStringLiteral("/dev/sda1"));
    volume.addMountPoint(QStringLiteral("/mnt/data"));
    QVERIFY(volume.mountPoints().contains(QStringLiteral("/mnt/data")));
    QVERIFY(volume.duplicatesMountPoints().isEmpty());
    QVERIFY(volume.subVolumes().isEmpty());

    volume.addMountPoint(QStringLiteral("/mnt/data"), QStringLiteral("/@data"));

    QVERIFY(volume.duplicatesMountPoints().isEmpty());
    QCOMPARE(volume.subVolumes().value(QStringLiteral("/mnt/data")), QStringLiteral("/@data"));
}

void VolumeTest::testManagerSelfReturnsStableSingleton()
{
    VolumeManager *first = VolumeManager::self();
    VolumeManager *second = VolumeManager::self();

    QVERIFY(first != nullptr);
    QCOMPARE(first, second);
}

void VolumeTest::testEmptyManagerAccessorsReturnEmptyCollections()
{
    VolumeManager manager;
    manager.m_volumes.clear();
    manager.m_uDiskVolumes.clear();

    QVERIFY(manager.getDuplicates().isEmpty());
    QVERIFY(manager.volumesHaveSubVolumes().isEmpty());
    QVERIFY(manager.volumes().isEmpty());
    QVERIFY(manager.uDiskVolumes().isEmpty());
}

void VolumeTest::testRefreshEmitsUpdateAndRebuildsCachedVolumes()
{
    ukui_test_gio_reset();
    VolumeManager manager;

    Volume staleVolume(QStringLiteral("/dev/ukui-search-stale"));
    staleVolume.addMountPoint(QStringLiteral("/tmp/ukui-search-stale"));
    manager.m_volumes.insert(staleVolume.device(), staleVolume);
    manager.m_uDiskVolumes.insert(staleVolume.device(), staleVolume);

    QSignalSpy updateSpy(&manager, SIGNAL(VolumeDataUpdated()));
    QVERIFY(updateSpy.isValid());

    manager.refresh();

    QCOMPARE(updateSpy.count(), 1);
    QVERIFY(!manager.m_volumes.contains(staleVolume.device()));
    QVERIFY(manager.m_uDiskVolumes.isEmpty());
}

void VolumeTest::testGetMountPointHandlesNullMount()
{
    ukui_test_gio_reset();
    QVERIFY(VolumeManager::getMountPoint(nullptr).isEmpty());
}

void VolumeTest::testMountCallbacksEmitBaseSignalsForNullMount()
{
    ukui_test_gio_reset();
    VolumeManager manager;
    manager.m_volumes.clear();
    manager.m_uDiskVolumes.clear();

    QSignalSpy mountAddedSpy(&manager, SIGNAL(MountAdded(QString)));
    QSignalSpy uDiskMountedSpy(&manager, SIGNAL(UDiskMounted(QString)));
    QSignalSpy uDiskUnmountedSpy(&manager, SIGNAL(UDiskUnmounted(QString)));
    QVERIFY(mountAddedSpy.isValid());
    QVERIFY(uDiskMountedSpy.isValid());
    QVERIFY(uDiskUnmountedSpy.isValid());

    VolumeManager::mountAddCallback(nullptr, nullptr, &manager);
    QCOMPARE(mountAddedSpy.count(), 1);
    QVERIFY(mountAddedSpy.first().at(0).toString().isEmpty());

    Volume removable(QStringLiteral("/dev/sdz1"));
    removable.addMountPoint(QStringLiteral("/media/disk"));
    manager.m_uDiskVolumes.insert(removable.device(), removable);

    VolumeManager::mountRemoveCallback(nullptr, nullptr, &manager);
    QVERIFY(uDiskMountedSpy.isEmpty());
    QVERIFY(uDiskUnmountedSpy.count() <= 1);
}

void VolumeTest::testDuplicatesMountPointsWithExactlyOneReturnsEmpty()
{
    Volume volume(QStringLiteral("/dev/sdz1"));
    volume.addMountPoint(QStringLiteral("/mnt/solo"));
    // Exactly one mount point with empty sub-volume → cleared, returns empty
    QVERIFY(volume.duplicatesMountPoints().isEmpty());

    // Two mount points without sub-volumes → returned as duplicates
    volume.addMountPoint(QStringLiteral("/mnt/solo2"));
    QCOMPARE(volume.duplicatesMountPoints().size(), 2);
}

void VolumeTest::testSubVolumesReturnsOnlyEntriesWithNonEmptyValue()
{
    Volume volume(QStringLiteral("/dev/sda1"));
    volume.addMountPoint(QStringLiteral("/mnt/a"));               // empty sub-volume → excluded
    volume.addMountPoint(QStringLiteral("/mnt/b"), QStringLiteral("/@b"));  // has sub-volume
    volume.addMountPoint(QStringLiteral("/mnt/c"), QStringLiteral(""));     // empty → excluded

    const QMap<QString, QString> subs = volume.subVolumes();
    QCOMPARE(subs.size(), 1);
    QVERIFY(subs.contains(QStringLiteral("/mnt/b")));
    QCOMPARE(subs.value(QStringLiteral("/mnt/b")), QStringLiteral("/@b"));
    QVERIFY(!subs.contains(QStringLiteral("/mnt/a")));
    QVERIFY(!subs.contains(QStringLiteral("/mnt/c")));
}

void VolumeTest::testMountAddCallbackEmitsUDiskMountedWhenNewDiskAppears()
{
    const QString device = findMountedBlockDevice();
    if (device.isEmpty()) {
        QSKIP("No mounted block device matching the removable-device regex is available");
    }

    ukui_test_gio_reset();
    ukui_test_gio_set_monitor_available(1);
    ukui_test_gio_add_volume(device.toUtf8().constData(), 1, 0);

    VolumeManager manager;
    manager.m_uDiskVolumes.clear();

    QSignalSpy mountAddedSpy(&manager, SIGNAL(MountAdded(QString)));
    QSignalSpy uDiskMountedSpy(&manager, SIGNAL(UDiskMounted(QString)));
    QVERIFY(mountAddedSpy.isValid());
    QVERIFY(uDiskMountedSpy.isValid());

    GMount *mount = ukui_test_gio_create_mount("file:///media/new-disk", 1);

    VolumeManager::mountAddCallback(nullptr, mount, &manager);

    QCOMPARE(mountAddedSpy.count(), 1);
    QCOMPARE(mountAddedSpy.first().at(0).toString(), QStringLiteral("/media/new-disk"));
    QCOMPARE(uDiskMountedSpy.count(), 1);
    QCOMPARE(uDiskMountedSpy.first().at(0).toString(), QStringLiteral("/media/new-disk"));
}

void VolumeTest::testGetMountPointReturnsPathForValidMount()
{
    ukui_test_gio_reset();
    QCOMPARE(VolumeManager::getMountPoint(ukui_test_gio_create_mount("file:///tmp/volume-test", 1)),
             QStringLiteral("/tmp/volume-test"));
}

void VolumeTest::testConstructorHandlesMissingMonitor()
{
    ukui_test_gio_reset();
    ukui_test_gio_set_monitor_available(0);

    VolumeManager manager;

    QVERIFY(manager.m_volumeMonitor == nullptr);
}

void VolumeTest::testRefreshBuildsUDiskVolumesFromFakeGioData()
{
    const QString device = findMountedBlockDevice();
    if (device.isEmpty()) {
        QSKIP("No mounted block device matching the removable-device regex is available");
    }

    ukui_test_gio_reset();
    ukui_test_gio_set_monitor_available(1);
    ukui_test_gio_add_volume(device.toUtf8().constData(), 1, 0);

    VolumeManager manager;
    QSignalSpy updateSpy(&manager, SIGNAL(VolumeDataUpdated()));
    QVERIFY(updateSpy.isValid());

    manager.refresh();

    QCOMPARE(updateSpy.count(), 1);
    QVERIFY(manager.m_volumes.contains(device));
    QVERIFY(manager.m_uDiskVolumes.contains(device));
}

void VolumeTest::testRefreshUsesDriveCanStopWhenVolumeCannotEject()
{
    const QString device = findMountedBlockDevice();
    if (device.isEmpty()) {
        QSKIP("No mounted block device matching the removable-device regex is available");
    }

    ukui_test_gio_reset();
    ukui_test_gio_set_monitor_available(1);
    ukui_test_gio_add_volume(device.toUtf8().constData(), 0, 1);

    VolumeManager manager;
    manager.refresh();

    QVERIFY(manager.m_uDiskVolumes.contains(device));
}

void VolumeTest::testRefreshSkipsMatchingDeviceWithoutEjectCapability()
{
    const QString device = findMountedBlockDevice();
    if (device.isEmpty()) {
        QSKIP("No mounted block device matching the removable-device regex is available");
    }

    ukui_test_gio_reset();
    ukui_test_gio_set_monitor_available(1);
    ukui_test_gio_add_volume(device.toUtf8().constData(), 0, 0);

    VolumeManager manager;
    manager.refresh();

    QVERIFY(manager.m_volumes.contains(device));
    QVERIFY(!manager.m_uDiskVolumes.contains(device));
}

void VolumeTest::testRefreshIgnoresNonMatchingGioDevice()
{
    ukui_test_gio_reset();
    ukui_test_gio_set_monitor_available(1);
    ukui_test_gio_add_volume("/dev/mapper/ukui-search-test", 1, 0);

    VolumeManager manager;
    manager.refresh();

    QVERIFY(!manager.m_uDiskVolumes.contains(QStringLiteral("/dev/mapper/ukui-search-test")));
}

void VolumeTest::testGetMountPointConvertsFileUriToLocalPath()
{
    ukui_test_gio_reset();

    GMount *mount = ukui_test_gio_create_mount("file:///media/test-disk", 1);

    QCOMPARE(VolumeManager::getMountPoint(mount), QStringLiteral("/media/test-disk"));
}

void VolumeTest::testGetMountPointKeepsSmbUri()
{
    ukui_test_gio_reset();

    GMount *mount = ukui_test_gio_create_mount("smb://server/share", 1);

    QCOMPARE(VolumeManager::getMountPoint(mount), QStringLiteral("smb://server/share"));
}

void VolumeTest::testGetMountPointReturnsEmptyWhenRootMissing()
{
    ukui_test_gio_reset();

    GMount *mount = ukui_test_gio_create_mount("file:///media/no-root", 0);

    QVERIFY(VolumeManager::getMountPoint(mount).isEmpty());
}

void VolumeTest::testGetMountPointReturnsEmptyWhenUriCannotConvert()
{
    ukui_test_gio_reset();

    GMount *mount = ukui_test_gio_create_mount("mtp://phone/storage", 1);

    QVERIFY(VolumeManager::getMountPoint(mount).isEmpty());
}

void VolumeTest::testMountAddCallbackDoesNotEmitUDiskMountedWhenCountUnchanged()
{
    ukui_test_gio_reset();
    ukui_test_gio_set_monitor_available(1);

    VolumeManager manager;
    QSignalSpy mountAddedSpy(&manager, SIGNAL(MountAdded(QString)));
    QSignalSpy uDiskMountedSpy(&manager, SIGNAL(UDiskMounted(QString)));
    QVERIFY(mountAddedSpy.isValid());
    QVERIFY(uDiskMountedSpy.isValid());

    GMount *mount = ukui_test_gio_create_mount("file:///media/regular-disk", 1);

    VolumeManager::mountAddCallback(nullptr, mount, &manager);

    QCOMPARE(mountAddedSpy.count(), 1);
    QCOMPARE(mountAddedSpy.first().at(0).toString(), QStringLiteral("/media/regular-disk"));
    QVERIFY(uDiskMountedSpy.isEmpty());
}

void VolumeTest::testMountRemoveCallbackEmitsUDiskUnmountedWhenDiskDisappears()
{
    const QString device = findMountedBlockDevice();
    if (device.isEmpty()) {
        QSKIP("No mounted block device matching the removable-device regex is available");
    }

    ukui_test_gio_reset();
    ukui_test_gio_set_monitor_available(1);
    ukui_test_gio_add_volume(device.toUtf8().constData(), 1, 0);

    VolumeManager manager;
    QSignalSpy unmountedSpy(&manager, SIGNAL(UDiskUnmounted(QString)));
    QVERIFY(unmountedSpy.isValid());

    GMount *mount = ukui_test_gio_create_mount("file:///media/removed-disk", 1);
    ukui_test_gio_clear_volumes();
    ukui_test_gio_set_monitor_available(1);

    VolumeManager::mountRemoveCallback(nullptr, mount, &manager);

    QCOMPARE(unmountedSpy.count(), 1);
    QCOMPARE(unmountedSpy.first().at(0).toString(), QStringLiteral("/media/removed-disk"));
}

void VolumeTest::testMountRemoveCallbackDoesNotEmitUDiskUnmountedWhenCountUnchanged()
{
    ukui_test_gio_reset();
    ukui_test_gio_set_monitor_available(1);

    VolumeManager manager;
    QSignalSpy unmountedSpy(&manager, SIGNAL(UDiskUnmounted(QString)));
    QVERIFY(unmountedSpy.isValid());

    GMount *mount = ukui_test_gio_create_mount("file:///media/kept-disk", 1);

    VolumeManager::mountRemoveCallback(nullptr, mount, &manager);

    QVERIFY(unmountedSpy.isEmpty());
}

void VolumeTest::testHeapAllocatedManagerCanBeDeleted()
{
    ukui_test_gio_reset();

    VolumeManager *manager = new VolumeManager();

    QVERIFY(manager != nullptr);
    delete manager;
}

QTEST_MAIN(VolumeTest)
