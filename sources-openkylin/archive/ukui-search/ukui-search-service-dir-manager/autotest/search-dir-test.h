#ifndef SEARCH_DIR_TEST_H
#define SEARCH_DIR_TEST_H

#include <QObject>

class SearchDirTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testNonExistingPath();
    void testExistingPathWithoutGeneratedBlacklist();
    void testBlackListAccessorsAndEquality();
    void testErrorStrings();
    void testErrorStringReturnsEmptyForUnknownError();
    void testGeneratedBlackListRejectsGlobalBlackListPath();
    void testGeneratedBlackListRejectsExistingSearchDir();
    void testGeneratedBlackListAllowsUDiskPathUnderMediaSymlinkTarget();
    void testGenerateBlackListClearsPreviousEntries();
    void testBlackListOfRootIncludesGlobalBlackList();
    void testBlackListOfDirIncludesNestedGlobalBlackList();
    void testGeneratedBlackListContainsChildGlobalBlackList();
    void testGeneratedRootBlackListContainsConfiguredSearchDirs();
    void testGeneratedRootBlackListHandlesDuplicateMountContainingRoot();
    void testGeneratedRootBlackListHandlesDuplicateMountSearchDirPolicies();
    void testBlackListOfDirHandlesNestedDuplicateMounts();
    void testBlackListOfDirHandlesForcedChildBeforeParentDuplicates();
    void testGeneratedBlackListHandlesRepeatMountErrorBranches();
    void testGeneratedBlackListHandlesForcedChildBeforeParentDuplicates();
    void testGeneratedBlackListHandlesSubVolumeMirrorBranches();
    void testGeneratedBlackListRejectsChildOfExistingSearchDir();
    void testGeneratedBlackListContainsExistingChildSearchDir();
    void testHandleSubVolumesForRootAndMirrorPaths();
    void testHandleSubVolumesAddsSpecBelowParentSearchDir();
    void testHandleSubVolumesReturnsEmptyForUnrelatedPath();
    void testNestedDuplicateMountsParentKeptChildBlacklisted();
    void testGeneratedBlackListBlacklistsRepeatedNestedMountUnderIndexedMount();
    void testGeneratedBlackListBlacklistsIndexedPathMappedFromRepeatedNestedMount();
    void testGeneratedBlackListRejectsNestedRepeatedMountWhenParentMountIndexed();
    void testGeneratedBlackListBlacklistsRepeatedMountUnderIndexedAncestor();
    void testGeneratedBlackListBlacklistsMappedSubVolumeChildUnderSpecPath();
    void testGeneratedBlackListRejectsMappedMountChildUnderSubVolumeMirror();
};

#endif // SEARCH_DIR_TEST_H
