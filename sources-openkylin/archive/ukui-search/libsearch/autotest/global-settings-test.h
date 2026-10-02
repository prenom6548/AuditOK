#ifndef GLOBAL_SETTINGS_TEST_H
#define GLOBAL_SETTINGS_TEST_H

#include <QObject>

class GlobalSettingsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaultValuesAndCacheAccess();
    void testHandleRadiusSetBranches();
    void testSingletonReturnsConfiguredDefaults();
    void testSchemaDrivenUpdatesEmitSignalsAndRefreshCache();
};

#endif // GLOBAL_SETTINGS_TEST_H
