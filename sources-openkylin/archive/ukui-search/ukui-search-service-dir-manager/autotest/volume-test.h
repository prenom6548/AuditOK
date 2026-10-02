#ifndef VOLUME_TEST_H
#define VOLUME_TEST_H

#include <QObject>

class VolumeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaultAndDeviceAccessors();
    void testDuplicateMountPointsOnlyReturnedWhenMoreThanOne();
    void testSubVolumesAndMountPointsAccessors();
    void testVolumeManagerFiltersDuplicatesAndSubVolumes();
    void testSubVolumeMountPointsAreNotDuplicateMounts();
    void testMountPointOverwriteUpdatesSubVolumeState();
    void testManagerSelfReturnsStableSingleton();
    void testEmptyManagerAccessorsReturnEmptyCollections();
    void testRefreshEmitsUpdateAndRebuildsCachedVolumes();
    void testGetMountPointHandlesNullMount();
    void testMountCallbacksEmitBaseSignalsForNullMount();
    void testDuplicatesMountPointsWithExactlyOneReturnsEmpty();
    void testSubVolumesReturnsOnlyEntriesWithNonEmptyValue();
    void testMountAddCallbackEmitsUDiskMountedWhenNewDiskAppears();
    void testGetMountPointReturnsPathForValidMount();
    void testConstructorHandlesMissingMonitor();
    void testRefreshBuildsUDiskVolumesFromFakeGioData();
    void testRefreshUsesDriveCanStopWhenVolumeCannotEject();
    void testRefreshSkipsMatchingDeviceWithoutEjectCapability();
    void testRefreshIgnoresNonMatchingGioDevice();
    void testGetMountPointConvertsFileUriToLocalPath();
    void testGetMountPointKeepsSmbUri();
    void testGetMountPointReturnsEmptyWhenRootMissing();
    void testGetMountPointReturnsEmptyWhenUriCannotConvert();
    void testMountAddCallbackDoesNotEmitUDiskMountedWhenCountUnchanged();
    void testMountRemoveCallbackEmitsUDiskUnmountedWhenDiskDisappears();
    void testMountRemoveCallbackDoesNotEmitUDiskUnmountedWhenCountUnchanged();
    void testHeapAllocatedManagerCanBeDeleted();
};

#endif // VOLUME_TEST_H
