#include "pending-app-info-test.h"

#include <QTest>

#include "pending-app-info.h"

using namespace UkuiSearch;

void PendingAppInfoTest::testDefaultsAndSetters()
{
    PendingAppInfo info;

    QVERIFY(info.path().isEmpty());
    QCOMPARE(info.handleType(), PendingAppInfo::HandleTypes());
    QCOMPARE(info.favoritesState(), -1);
    QCOMPARE(info.topState(), -1);
    QCOMPARE(info.launchTimes(), 0);
    QVERIFY(!info.willAddLunchTimes());
    QVERIFY(!info.dbVersionNeedUpdate());
    QVERIFY(info.pathsNeedRefreshData().isEmpty());
    QVERIFY(info.value2BSet().isEmpty());

    ApplicationPropertyMap propertyMap;
    propertyMap.insert(ApplicationProperty::LocalName, QStringLiteral("Browser"));
    ApplicationInfoMap appInfo;
    appInfo.insert(QStringLiteral("/tmp/app.desktop"), propertyMap);

    info.setDesktopFp(QStringLiteral("/tmp/app.desktop"));
    info.setHandleType(PendingAppInfo::Insert | PendingAppInfo::UpdateAll);
    info.setFavorites(3);
    info.setTop(2);
    info.setLaunchWillAdd(true);
    info.setLaunchTimes(7);
    info.setPathsNeedRefreshData({QStringLiteral("/tmp/a"), QStringLiteral("/tmp/b")});
    info.setDBUpdate(true);
    info.setValue(appInfo);

    QCOMPARE(info.path(), QStringLiteral("/tmp/app.desktop"));
    QVERIFY(info.handleType().testFlag(PendingAppInfo::Insert));
    QVERIFY(info.handleType().testFlag(PendingAppInfo::UpdateAll));
    QCOMPARE(info.favoritesState(), 3);
    QCOMPARE(info.topState(), 2);
    QCOMPARE(info.launchTimes(), 7);
    QVERIFY(info.willAddLunchTimes());
    QVERIFY(info.dbVersionNeedUpdate());
    QCOMPARE(info.pathsNeedRefreshData(), QStringList({QStringLiteral("/tmp/a"), QStringLiteral("/tmp/b")}));
    QCOMPARE(info.value2BSet().value(QStringLiteral("/tmp/app.desktop")).value(ApplicationProperty::LocalName).toString(),
             QStringLiteral("Browser"));
}

void PendingAppInfoTest::testMergeCombinesFlagsAndState()
{
    PendingAppInfo base(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::Insert, 1, -1, false, 4);

    ApplicationPropertyMap baseProperties;
    baseProperties.insert(ApplicationProperty::LocalName, QStringLiteral("Old Name"));
    ApplicationInfoMap baseValue;
    baseValue.insert(QStringLiteral("/tmp/app.desktop"), baseProperties);
    base.setValue(baseValue);

    PendingAppInfo incoming(QStringLiteral("/tmp/app.desktop"),
                            PendingAppInfo::UpdateTop | PendingAppInfo::SetValue,
                            -1,
                            9,
                            true);
    ApplicationPropertyMap incomingProperties;
    incomingProperties.insert(ApplicationProperty::Exec, QStringLiteral("/usr/bin/app"));
    ApplicationInfoMap incomingValue;
    incomingValue.insert(QStringLiteral("/tmp/app.desktop"), incomingProperties);
    incoming.setValue(incomingValue);

    base.merge(incoming);

    QVERIFY(base.handleType().testFlag(PendingAppInfo::Insert));
    QVERIFY(base.handleType().testFlag(PendingAppInfo::UpdateTop));
    QVERIFY(base.handleType().testFlag(PendingAppInfo::SetValue));
    QCOMPARE(base.favoritesState(), 1);
    QCOMPARE(base.topState(), 9);
    QCOMPARE(base.launchTimes(), 5);
    QCOMPARE(base.value2BSet().value(QStringLiteral("/tmp/app.desktop")).value(ApplicationProperty::Exec).toString(),
             QStringLiteral("/usr/bin/app"));
}

void PendingAppInfoTest::testEqualityUsesDesktopPath()
{
    PendingAppInfo lhs(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::Insert);
    PendingAppInfo samePath(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::Delete);
    PendingAppInfo other(QStringLiteral("/tmp/other.desktop"), PendingAppInfo::Insert);

    QVERIFY(lhs == samePath);
    QVERIFY(!(lhs == other));
}

void PendingAppInfoTest::testMergePrefersIncomingScalarStateAndMergesValueMap()
{
    PendingAppInfo base(QStringLiteral("/tmp/app.desktop"),
                        PendingAppInfo::UpdateFavorites | PendingAppInfo::UpdateLaunchTimes,
                        1,
                        2,
                        false,
                        8);
    ApplicationPropertyMap baseProperties;
    baseProperties.insert(ApplicationProperty::LocalName, QStringLiteral("Old Name"));
    ApplicationInfoMap baseValue;
    baseValue.insert(QStringLiteral("/tmp/app.desktop"), baseProperties);
    base.setValue(baseValue);

    PendingAppInfo incoming(QStringLiteral("/tmp/app.desktop"),
                            PendingAppInfo::UpdateTop | PendingAppInfo::SetValue,
                            5,
                            9,
                            false,
                            1);
    ApplicationPropertyMap incomingProperties;
    incomingProperties.insert(ApplicationProperty::Exec, QStringLiteral("/usr/bin/new"));
    ApplicationInfoMap incomingValue;
    incomingValue.insert(QStringLiteral("/tmp/app.desktop"), incomingProperties);
    incoming.setValue(incomingValue);

    base.merge(incoming);

    QVERIFY(base.handleType().testFlag(PendingAppInfo::UpdateFavorites));
    QVERIFY(base.handleType().testFlag(PendingAppInfo::UpdateTop));
    QVERIFY(base.handleType().testFlag(PendingAppInfo::SetValue));
    QCOMPARE(base.favoritesState(), 5);
    QCOMPARE(base.topState(), 9);
    QCOMPARE(base.launchTimes(), 8);
    QCOMPARE(base.value2BSet().size(), 1);
    QCOMPARE(base.value2BSet().value(QStringLiteral("/tmp/app.desktop")).size(), 2);
    QCOMPARE(base.value2BSet().value(QStringLiteral("/tmp/app.desktop")).value(ApplicationProperty::LocalName).toString(),
             QStringLiteral("Old Name"));
    QCOMPARE(base.value2BSet().value(QStringLiteral("/tmp/app.desktop")).value(ApplicationProperty::Exec).toString(),
             QStringLiteral("/usr/bin/new"));
}

void PendingAppInfoTest::testSetHandleTypeFromOtherInstance()
{
    PendingAppInfo source(QStringLiteral("/tmp/source.desktop"), PendingAppInfo::UpdateLocaleData);
    PendingAppInfo target(QStringLiteral("/tmp/target.desktop"), PendingAppInfo::Insert);

    target.setHandleType(source);

    QCOMPARE(target.handleType(), PendingAppInfo::HandleTypes(PendingAppInfo::UpdateLocaleData));
}

void PendingAppInfoTest::testMergeRetainsExistingScalarsWhenIncomingUnset()
{
    PendingAppInfo base(QStringLiteral("/tmp/app.desktop"),
                        PendingAppInfo::UpdateFavorites | PendingAppInfo::UpdateLaunchTimes,
                        7,
                        6,
                        false,
                        3);
    PendingAppInfo incoming(QStringLiteral("/tmp/app.desktop"),
                            PendingAppInfo::UpdateTop,
                            -1,
                            -1,
                            false,
                            99);

    base.merge(incoming);

    QVERIFY(base.handleType().testFlag(PendingAppInfo::UpdateFavorites));
    QVERIFY(base.handleType().testFlag(PendingAppInfo::UpdateLaunchTimes));
    QVERIFY(base.handleType().testFlag(PendingAppInfo::UpdateTop));
    QCOMPARE(base.favoritesState(), 7);
    QCOMPARE(base.topState(), 6);
    QCOMPARE(base.launchTimes(), 3);
    QVERIFY(base.value2BSet().isEmpty());
}

void PendingAppInfoTest::testRefreshDatabaseAndValueMapAccessors()
{
    PendingAppInfo info(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::RefreshDataBase);
    info.setPathsNeedRefreshData({QStringLiteral("/tmp/a.desktop"), QStringLiteral("/tmp/b.desktop")});
    info.setDBUpdate(true);

    ApplicationPropertyMap properties;
    properties.insert(ApplicationProperty::Icon, QStringLiteral("app-icon"));
    properties.insert(ApplicationProperty::NameEn, QStringLiteral("App"));

    ApplicationInfoMap values;
    values.insert(QStringLiteral("/tmp/app.desktop"), properties);
    info.setValue(values);

    QCOMPARE(info.handleType(), PendingAppInfo::HandleTypes(PendingAppInfo::RefreshDataBase));
    QCOMPARE(info.pathsNeedRefreshData(),
             QStringList({QStringLiteral("/tmp/a.desktop"), QStringLiteral("/tmp/b.desktop")}));
    QVERIFY(info.dbVersionNeedUpdate());
    QCOMPARE(info.value2BSet().value(QStringLiteral("/tmp/app.desktop")).value(ApplicationProperty::Icon).toString(),
             QStringLiteral("app-icon"));
    QCOMPARE(info.value2BSet().value(QStringLiteral("/tmp/app.desktop")).value(ApplicationProperty::NameEn).toString(),
             QStringLiteral("App"));
}

void PendingAppInfoTest::testMergeAddsNewDesktopValueMaps()
{
    PendingAppInfo base(QStringLiteral("/tmp/base.desktop"), PendingAppInfo::SetValue);
    ApplicationPropertyMap baseProperties;
    baseProperties.insert(ApplicationProperty::LocalName, QStringLiteral("Base"));
    ApplicationInfoMap baseValues;
    baseValues.insert(QStringLiteral("/tmp/base.desktop"), baseProperties);
    base.setValue(baseValues);

    PendingAppInfo incoming(QStringLiteral("/tmp/other.desktop"), PendingAppInfo::SetValue);
    ApplicationPropertyMap incomingProperties;
    incomingProperties.insert(ApplicationProperty::Exec, QStringLiteral("/usr/bin/other"));
    ApplicationInfoMap incomingValues;
    incomingValues.insert(QStringLiteral("/tmp/other.desktop"), incomingProperties);
    incoming.setValue(incomingValues);

    base.merge(incoming);

    QCOMPARE(base.value2BSet().size(), 2);
    QCOMPARE(base.value2BSet().value(QStringLiteral("/tmp/base.desktop")).value(ApplicationProperty::LocalName).toString(),
             QStringLiteral("Base"));
    QCOMPARE(base.value2BSet().value(QStringLiteral("/tmp/other.desktop")).value(ApplicationProperty::Exec).toString(),
             QStringLiteral("/usr/bin/other"));
    QVERIFY(base.handleType().testFlag(PendingAppInfo::SetValue));
}

void PendingAppInfoTest::testMergeLaunchIncrementOnlyWhenRequested()
{
    PendingAppInfo base(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::UpdateLaunchTimes, -1, -1, false, 4);

    PendingAppInfo noIncrement(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::UpdateTop, -1, 1, false, 99);
    base.merge(noIncrement);
    QCOMPARE(base.launchTimes(), 4);

    PendingAppInfo increment(QStringLiteral("/tmp/app.desktop"), PendingAppInfo::UpdateLaunchTimes, -1, -1, true, 99);
    base.merge(increment);
    QCOMPARE(base.launchTimes(), 5);
}

void PendingAppInfoTest::testHandleTypeOrderingAndFlagCombinations()
{
    QVERIFY(PendingAppInfo::Delete < PendingAppInfo::Insert);
    QVERIFY(PendingAppInfo::Insert < PendingAppInfo::UpdateAll);
    QVERIFY(PendingAppInfo::UpdateAll < PendingAppInfo::UpdateLocaleData);

    PendingAppInfo info(QStringLiteral("/tmp/app.desktop"),
                        PendingAppInfo::UpdateLaunchTimes
                        | PendingAppInfo::UpdateFavorites
                        | PendingAppInfo::UpdateTop
                        | PendingAppInfo::SetValue);

    QVERIFY(info.handleType().testFlag(PendingAppInfo::UpdateLaunchTimes));
    QVERIFY(info.handleType().testFlag(PendingAppInfo::UpdateFavorites));
    QVERIFY(info.handleType().testFlag(PendingAppInfo::UpdateTop));
    QVERIFY(info.handleType().testFlag(PendingAppInfo::SetValue));
    QVERIFY(!info.handleType().testFlag(PendingAppInfo::Delete));
}

QTEST_MAIN(PendingAppInfoTest)
