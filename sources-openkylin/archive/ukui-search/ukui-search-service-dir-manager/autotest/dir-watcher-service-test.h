#ifndef DIR_WATCHER_SERVICE_TEST_H
#define DIR_WATCHER_SERVICE_TEST_H

#include <QObject>

class DirWatcherServiceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSingletonAndAliasMethods();
    void testAppendAndRemoveSignalsForTemporaryDirectory();
    void testBlockDirSignalsAndErrorCodes();
    void testSearchableListsAndUkccVisibility();
    void testExtraDirsOfPathsWithNoMatchingUDisk();
    void testAppendRejectsMissingPathWithoutSignal();
    void testDuplicateSearchDirDoesNotEmitAppendAgain();
    void testRemoveNormalizesPathAndEmitsSignal();
    void testBlockDirRemovalEmitsEvenWhenMissing();
    void testBlackListOfDirDelegatesToSearchDir();
    void testAppendParentRemovesExistingChildSearchDir();
    void testShowInUkccNormalizesPath();
    void testInitDbusServiceCanBeCalledRepeatedly();
    void testExtraDirsOfPathsReturnsUDiskUnderGlobalBlackList();
    void testRefreshExtraDirsEmitsAddedAndRemovedSignals();
    void testUDiskCallbacksTriggerRefreshExtraDirs();
    void testExtraDirsOfPathsSkipsMountPointNotUnderIndexDir();
    void testAppendRootSearchDirRemovesAllExistingChildren();
    void testExtraDirsOfPathsUDiskUnderIndexNotUnderBlacklist();
    void testRefreshExtraDirsWithNoChangesEmitsNothing();
    void testExtraDirsOfPathsIgnoresNonGlobalBlackListMatch();
    void testExtraDirsOfPathsAddsUDiskOnlyOnceForMultipleBlacklistedMountPoints();
    void testExtraDirsOfPathsUsesConfiguredBlackDirsWhenPathSetMatchesSearchDirs();
    void testExtraDirsOfPathsIgnoresExactBlackDirWhenNotGlobalBlackList();
    void testExtraDirsOfPathsSkipsMountPointOutsideMatchedSearchDirs();
    void testExtraDirsOfPathsMatchesSecondSearchDirAfterSkippingFirst();
    void testExtraDirsOfPathsAddsEligibleMountAfterSkippingSiblingMount();
    void testRefreshExtraDirsOnlyAddsNewEntries();
    void testRefreshExtraDirsOnlyRemovesStaleEntries();
    void testRefreshExtraDirsHandlesMultipleAddsAndRemoves();
    void testAppendSearchDirUnderGlobalBlackListDoesNotEmitAppend();
    void testAddBlockDirNormalizesAndEmitsNormalizedPath();
    void testAddBlockDirFailureDoesNotEmitSignal();
    void testAddBlockDirDuplicateDoesNotEmitSignal();
    void testShowInUkccReturnsFalseForExactExtraDirPath();
    void testShowInUkccReturnsFalseForDirtyExtraDirPath();
    void testLocalWatcherProcessesCompatibleCacheAndForwardsMountAdded();
    void testHeapAllocatedWatcherCanBeDeleted();
};

#endif // DIR_WATCHER_SERVICE_TEST_H
