#ifndef PENDING_APP_INFO_TEST_H
#define PENDING_APP_INFO_TEST_H

#include <QObject>

class PendingAppInfoTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaultsAndSetters();
    void testMergeCombinesFlagsAndState();
    void testEqualityUsesDesktopPath();
    void testMergePrefersIncomingScalarStateAndMergesValueMap();
    void testSetHandleTypeFromOtherInstance();
    void testMergeRetainsExistingScalarsWhenIncomingUnset();
    void testRefreshDatabaseAndValueMapAccessors();
    void testMergeAddsNewDesktopValueMaps();
    void testMergeLaunchIncrementOnlyWhenRequested();
    void testHandleTypeOrderingAndFlagCombinations();
};

#endif // PENDING_APP_INFO_TEST_H
