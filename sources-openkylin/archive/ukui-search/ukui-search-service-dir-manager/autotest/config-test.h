#ifndef CONFIG_TEST_H
#define CONFIG_TEST_H

#include <QObject>

class ConfigTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSearchDirsCleansMissingAndNormalizesPaths();
    void testBlackDirsCleansMissingSearchDirs();
    void testGlobalBlackListRemovesDuplicates();
    void testBlockDirsForUserCleansAndNormalizes();
    void testExtraDirsCleanMissingAndUkccVisibility();
    void testSetExtraDirsAndSearchDirsAppendExistingExtraDirs();
    void testFilterSubDirsKeepsParentsOnly();
    void testFilterSubDirsRootRemovesEverythingBelowIt();
    void testFilterSubDirsUsesCaseInsensitiveSortOrder();
    void testProcessCompatibleCacheMovesValidEntriesToSettings();
    void testProcessCompatibleCacheIgnoresInvalidEntriesAndNoopsWhenDisabled();
    void testProcessCompatibleCacheSkipsInvalidButKeepsValidEntries();
    void testProcessCompatibleCachePersistsGeneratedBlacklistEntries();
    void testAddAndRemoveDirRoundTrip();
    void testAddBlockDirRemovesExistingChildren();
    void testAddBlockDirRejectsMissingAndDuplicateParents();
    void testAddRootBlockDirRemovesExistingChildren();
    void testRemoveExistingBlockDirRewritesSettings();
    void testRemoveMissingBlockDirDoesNotRewriteSettings();
    void testGlobalBlackListWithoutDuplicatesDoesNotRewrite();
    void testInitSettingsDataCreatesCurrentDefaults();
    void testInitDefaultCompatibleConfigOnlyAddsOnce();
    void testCheckSearchDirNormalizesAndDropsNestedChildren();
    void testCheckSearchDirRemovesMissingWithoutRewriteWhenNoNormalization();
    void testCheckSearchDirNoRewriteWhenAlreadyClean();
    void testAddBlockDirPreservesSiblingDirs();
    void testGlobalBlackListRewriteTrimsJsonArray();
    void testSearchDirsWithExistingExtraDirsIncludesThem();
    void testCheckSearchDirNormalizesDirtyPathsAndSaves();
    void testConstructorMigratesOldBlockSettingsWhenNoNewConfigExists();
    void testSaveHandlesConfigDirCreationFailure();
    void testSaveHandlesFileOpenFailure();
    void testSaveHandlesWriteFailure();
    void testConstructorUpgradesVersion1_0To1_1();
    void testConstructorVersion1_0KeepsExistingSysrootEntry();
    void testConstructorLeavesUnsupportedVersionUntouched();
    void testConstructorHandlesJsonParseError();
    void testConstructorReturnsWhenSettingsPathCannotBeOpened();
    void testConstructorMissingSearchDirsGroupInitsCompatibleConfig();
    void testConstructorLoadsExistingOldIndexableDirsIntoCompatibleCache();
    void testConstructorSkipsCompatibilityInitWhenOldIndexableEntriesAreAllMissing();
    void testConstructorReinitializesWhenGlobalSettingsMissing();
    void testConstructorReinitializesWhenVersionMissing();
    void testConstructorMissingVersionAndSearchDirsEnablesCompatibilityMode();
    void testConstructorNormalizesExistingBlockDirsForUser();
    void testConstructorDropsMissingBlockDirsWithoutFiltering();
    void testConstructorFiltersNestedBlockDirsForUser();
    void testConstructorMigratesEmptyOldBlockSettingsIntoCurrentConfig();
    void testConstructorMigratesOldBlockSettingsIntoExistingConfig();
};

#endif // CONFIG_TEST_H
