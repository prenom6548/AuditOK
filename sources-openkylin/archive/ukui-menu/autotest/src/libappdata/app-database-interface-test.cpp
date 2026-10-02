/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <QtTest>

#include "fakes/application-info.h"
#include "fakes/settings.h"
#include "fakes/user-config.h"

#include "../../../src/libappdata/app-database-interface.h"

using namespace UkuiMenu;

using AppUpdateVector = QVector<QPair<UkuiMenu::DataEntity, QVector<int>>>;
Q_DECLARE_METATYPE(AppUpdateVector)

class appDatabaseInterface_test : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void test_apps_returnsFilteredMappedEntities();
    void test_firstStartup_initializesPreinstalledAndDefaultFavorites();
    void test_getApp_returnsFalseForEmptyId();
    void test_propertyWriters_forwardAndClampValues();
    void test_databaseSignals_areForwarded();
    void test_updateSignals_coverAllPropertyRoles();
    void test_databaseSignals_coverVisibilityAndReplacementBranches();
    void test_updateAll_withoutSpecialProperties_emitsFullUpdate();
};

void appDatabaseInterface_test::init()
{
    qRegisterMetaType<AppUpdateVector>("QVector<QPair<UkuiMenu::DataEntity,QVector<int> > >");

    UkuiSearch::FakeAppInfoStore::reset();
    UserConfig::instance()->setFirstStartUp(false);
    UserConfig::instance()->initPreInstalledLists({});
    GlobalSetting::instance()->setSystemApps({});
    GlobalSetting::instance()->setDefaultFavoriteApps({});

    UkuiSearch::FakeAppInfoStore::g_apps.insert(QStringLiteral("/tmp/a.desktop"),
                                                UkuiSearch::FakeAppInfoStore::createApp(QStringLiteral("/tmp/a.desktop"),
                                                                                        QStringLiteral("Alpha"),
                                                                                        QStringLiteral("Office"),
                                                                                        QStringLiteral("A"),
                                                                                        QStringLiteral("2024-01-01 00:00:00")));
    UkuiSearch::FakeAppInfoStore::g_apps.insert(QStringLiteral("/tmp/b.desktop"),
                                                UkuiSearch::FakeAppInfoStore::createApp(QStringLiteral("/tmp/b.desktop"),
                                                                                        QStringLiteral("Beta"),
                                                                                        QStringLiteral("Utility"),
                                                                                        QStringLiteral("B"),
                                                                                        QStringLiteral("2024-01-02 00:00:00"),
                                                                                        1,
                                                                                        0));
}

void appDatabaseInterface_test::test_apps_returnsFilteredMappedEntities()
{
    GlobalSetting::instance()->setSystemApps({QStringLiteral("/tmp/a.desktop")});
    AppDatabaseInterface interface;

    const DataEntityVector apps = interface.apps();

    QCOMPARE(apps.size(), 1);
    QCOMPARE(apps.at(0).id(), QStringLiteral("/tmp/a.desktop"));
    QCOMPARE(apps.at(0).name(), QStringLiteral("Alpha"));
    QCOMPARE(apps.at(0).category(), QStringLiteral("Office"));
    QCOMPARE(apps.at(0).firstLetter(), QStringLiteral("A"));
    QCOMPARE(apps.at(0).dontDisplay(), 0);
    QCOMPARE(apps.at(0).autoStart(), 0);
}

void appDatabaseInterface_test::test_firstStartup_initializesPreinstalledAndDefaultFavorites()
{
    UserConfig::instance()->setFirstStartUp(true);
    GlobalSetting::instance()->setDefaultFavoriteApps({QStringLiteral("/tmp/a.desktop"),
                                                       QStringLiteral("/tmp/favorite.desktop")});

    AppDatabaseInterface interface;

    const auto preInstalled = UserConfig::instance()->preInstalledApps();
    QVERIFY(preInstalled.contains(QStringLiteral("/tmp/a.desktop")));
    QVERIFY(!preInstalled.contains(QStringLiteral("/tmp/b.desktop")));

    QCOMPARE(UkuiSearch::FakeAppInfoStore::g_favoriteCalls.size(), 2);
    QCOMPARE(UkuiSearch::FakeAppInfoStore::g_favoriteCalls.at(0).first, QStringLiteral("/tmp/a.desktop"));
    QCOMPARE(UkuiSearch::FakeAppInfoStore::g_favoriteCalls.at(1).first, QStringLiteral("/tmp/favorite.desktop"));
    QCOMPARE(UkuiSearch::FakeAppInfoStore::g_launchedCalls.size(), 2);
    QCOMPARE(UkuiSearch::FakeAppInfoStore::g_launchedCalls.at(0).second, true);
    QCOMPARE(UkuiSearch::FakeAppInfoStore::g_launchedCalls.at(1).second, true);
}

void appDatabaseInterface_test::test_getApp_returnsFalseForEmptyId()
{
    AppDatabaseInterface interface;
    DataEntity app;

    QCOMPARE(interface.getApp(QString(), app), false);
    QCOMPARE(interface.getApp(QStringLiteral("/tmp/a.desktop"), app), true);
    QCOMPARE(app.id(), QStringLiteral("/tmp/a.desktop"));
    QCOMPARE(app.name(), QStringLiteral("Alpha"));
}

void appDatabaseInterface_test::test_propertyWriters_forwardAndClampValues()
{
    AppDatabaseInterface interface;

    interface.fixAppToTop(QStringLiteral("/tmp/a.desktop"), -5);
    interface.fixAppToFavorite(QStringLiteral("/tmp/a.desktop"), -2);
    interface.updateApLaunchedState(QStringLiteral("/tmp/a.desktop"), true);

    QCOMPARE(UkuiSearch::FakeAppInfoStore::g_topCalls.size(), 1);
    QCOMPARE(UkuiSearch::FakeAppInfoStore::g_topCalls.first().first, QStringLiteral("/tmp/a.desktop"));
    QCOMPARE(UkuiSearch::FakeAppInfoStore::g_topCalls.first().second, 0);
    QCOMPARE(UkuiSearch::FakeAppInfoStore::g_favoriteCalls.first().second, 0);
    QCOMPARE(UkuiSearch::FakeAppInfoStore::g_launchedCalls.first().second, true);
}

void appDatabaseInterface_test::test_databaseSignals_areForwarded()
{
    AppDatabaseInterface interface;
    QSignalSpy addSpy(&interface, &AppDatabaseInterface::appAdded);
    QSignalSpy updateSpy(&interface, &AppDatabaseInterface::appUpdated);
    QSignalSpy deleteSpy(&interface, &AppDatabaseInterface::appDeleted);
    QSignalSpy failedSpy(&interface, &AppDatabaseInterface::appDatabaseOpenFailed);

    auto *appInfo = interface.findChild<UkuiSearch::ApplicationInfo *>();
    QVERIFY(appInfo != nullptr);

    Q_EMIT appInfo->appDBItems2BAdd({QStringLiteral("/tmp/a.desktop"), QStringLiteral("/tmp/b.desktop")});

    UkuiSearch::ApplicationInfoMap updateMap;
    UkuiSearch::ApplicationPropertyMap updateItem;
    updateItem.insert(UkuiSearch::ApplicationProperty::LocalName, QStringLiteral("Alpha Updated"));
    updateItem.insert(UkuiSearch::ApplicationProperty::Category, QStringLiteral("Network"));
    updateMap.insert(QStringLiteral("/tmp/a.desktop"), updateItem);
    Q_EMIT appInfo->appDBItems2BUpdate(updateMap);

    Q_EMIT appInfo->appDBItems2BDelete({QStringLiteral("/tmp/a.desktop")});
    Q_EMIT appInfo->DBOpenFailed();

    QCOMPARE(addSpy.count(), 1);
    QCOMPARE(updateSpy.count(), 1);
    QCOMPARE(deleteSpy.count(), 1);
    QCOMPARE(failedSpy.count(), 1);

    const auto addedApps = qvariant_cast<DataEntityVector>(addSpy.at(0).at(0));
    QCOMPARE(addedApps.size(), 2);
    QCOMPARE(addedApps.at(0).id(), QStringLiteral("/tmp/a.desktop"));
    QCOMPARE(addedApps.at(1).id(), QStringLiteral("/tmp/b.desktop"));
    QCOMPARE(addedApps.at(1).dontDisplay(), 1);

    const auto updates = qvariant_cast<AppUpdateVector>(updateSpy.at(0).at(0));
    QCOMPARE(updates.size(), 1);
    QCOMPARE(updates.at(0).first.id(), QStringLiteral("/tmp/a.desktop"));
    QVERIFY(updates.at(0).second.contains(DataEntity::Name));
    QVERIFY(updates.at(0).second.contains(DataEntity::Category));

    const auto deletedApps = deleteSpy.at(0).at(0).toStringList();
    QCOMPARE(deletedApps, QStringList({QStringLiteral("/tmp/a.desktop")}));
}

void appDatabaseInterface_test::test_updateSignals_coverAllPropertyRoles()
{
    AppDatabaseInterface interface;
    QSignalSpy updateSpy(&interface, &AppDatabaseInterface::appUpdated);

    auto *appInfo = interface.findChild<UkuiSearch::ApplicationInfo *>();
    QVERIFY(appInfo != nullptr);

    UkuiSearch::ApplicationInfoMap updateMap;
    UkuiSearch::ApplicationPropertyMap updateItem;
    updateItem.insert(UkuiSearch::ApplicationProperty::FirstLetterAll, QStringLiteral("Z"));
    updateItem.insert(UkuiSearch::ApplicationProperty::Icon, QStringLiteral("icon-updated"));
    updateItem.insert(UkuiSearch::ApplicationProperty::InsertTime, QStringLiteral("2024-02-01 00:00:00"));
    updateItem.insert(UkuiSearch::ApplicationProperty::LaunchTimes, 7);
    updateItem.insert(UkuiSearch::ApplicationProperty::Favorites, 3);
    updateItem.insert(UkuiSearch::ApplicationProperty::Launched, 1);
    updateItem.insert(UkuiSearch::ApplicationProperty::Top, 2);
    updateItem.insert(UkuiSearch::ApplicationProperty::Lock, true);
    updateMap.insert(QStringLiteral("/tmp/a.desktop"), updateItem);

    Q_EMIT appInfo->appDBItems2BUpdate(updateMap);

    QCOMPARE(updateSpy.count(), 1);
    const auto updates = qvariant_cast<AppUpdateVector>(updateSpy.takeFirst().at(0));
    QCOMPARE(updates.size(), 1);
    QCOMPARE(updates.at(0).first.id(), QStringLiteral("/tmp/a.desktop"));
    QCOMPARE(updates.at(0).first.firstLetter(), QStringLiteral("Z"));
    QCOMPARE(updates.at(0).first.icon(), QStringLiteral("icon-updated"));
    QCOMPARE(updates.at(0).first.insertTime(), QStringLiteral("2024-02-01 00:00:00"));
    QCOMPARE(updates.at(0).first.launchTimes(), 7);
    QCOMPARE(updates.at(0).first.favorite(), 3);
    QCOMPARE(updates.at(0).first.launched(), 1);
    QCOMPARE(updates.at(0).first.top(), 2);
    QCOMPARE(updates.at(0).first.isLock(), true);

    const QVector<int> roles = updates.at(0).second;
    QVERIFY(roles.contains(DataEntity::FirstLetter));
    QVERIFY(roles.contains(DataEntity::Icon));
    QVERIFY(roles.contains(DataEntity::LaunchTimes));
    QVERIFY(roles.contains(DataEntity::Favorite));
    QVERIFY(roles.contains(DataEntity::IsLaunched));
    QVERIFY(roles.contains(DataEntity::Top));
    QVERIFY(roles.contains(DataEntity::IsLocked));
}

void appDatabaseInterface_test::test_databaseSignals_coverVisibilityAndReplacementBranches()
{
    AppDatabaseInterface interface;
    QSignalSpy addSpy(&interface, &AppDatabaseInterface::appAdded);
    QSignalSpy updateSpy(&interface, &AppDatabaseInterface::appUpdated);
    QSignalSpy deleteSpy(&interface, &AppDatabaseInterface::appDeleted);

    auto *appInfo = interface.findChild<UkuiSearch::ApplicationInfo *>();
    QVERIFY(appInfo != nullptr);

    Q_EMIT appInfo->appDBItems2BAdd({});
    QCOMPARE(addSpy.count(), 0);

    UkuiSearch::ApplicationInfoMap emptyUpdateMap;
    Q_EMIT appInfo->appDBItems2BUpdate(emptyUpdateMap);
    QCOMPARE(updateSpy.count(), 0);

    Q_EMIT appInfo->appDBItems2BUpdateAll({});
    QCOMPARE(updateSpy.count(), 0);

    Q_EMIT appInfo->appDBItems2BDelete({});
    QCOMPARE(deleteSpy.count(), 0);

    UkuiSearch::ApplicationPropertyMap hiddenApp =
        UkuiSearch::FakeAppInfoStore::createApp(QStringLiteral("/tmp/hidden.desktop"),
                                                QStringLiteral("Hidden"),
                                                QStringLiteral("Utility"),
                                                QStringLiteral("H"),
                                                QStringLiteral("2024-01-03 00:00:00"),
                                                1,
                                                0);
    UkuiSearch::FakeAppInfoStore::g_apps.insert(QStringLiteral("/tmp/hidden.desktop"), hiddenApp);

    Q_EMIT appInfo->appDBItems2BAdd({QStringLiteral("/tmp/hidden.desktop")});
    QCOMPARE(addSpy.count(), 1);
    auto addedApps = qvariant_cast<DataEntityVector>(addSpy.takeFirst().at(0));
    QCOMPARE(addedApps.size(), 1);
    QCOMPARE(addedApps.at(0).id(), QStringLiteral("/tmp/hidden.desktop"));
    QCOMPARE(addedApps.at(0).dontDisplay(), 1);

    UkuiSearch::ApplicationPropertyMap autostartApp =
        UkuiSearch::FakeAppInfoStore::createApp(QStringLiteral("/tmp/autostart.desktop"),
                                                QStringLiteral("Autostart"),
                                                QStringLiteral("Utility"),
                                                QStringLiteral("A"),
                                                QStringLiteral("2024-01-03 00:00:00"),
                                                0,
                                                1);
    UkuiSearch::FakeAppInfoStore::g_apps.insert(QStringLiteral("/tmp/autostart.desktop"), autostartApp);

    Q_EMIT appInfo->appDBItems2BAdd({QStringLiteral("/tmp/autostart.desktop")});
    QCOMPARE(addSpy.count(), 1);
    addedApps = qvariant_cast<DataEntityVector>(addSpy.takeFirst().at(0));
    QCOMPARE(addedApps.size(), 1);
    QCOMPARE(addedApps.at(0).id(), QStringLiteral("/tmp/autostart.desktop"));
    QCOMPARE(addedApps.at(0).autoStart(), 1);

    UkuiSearch::ApplicationInfoMap unknownOnly;
    UkuiSearch::ApplicationPropertyMap unsupportedUpdate;
    unsupportedUpdate.insert(UkuiSearch::ApplicationProperty::DesktopFilePath, QStringLiteral("/tmp/a.desktop"));
    unknownOnly.insert(QStringLiteral("/tmp/a.desktop"), unsupportedUpdate);
    Q_EMIT appInfo->appDBItems2BUpdate(unknownOnly);
    QCOMPARE(updateSpy.count(), 0);

    UkuiSearch::ApplicationPropertyMap visibleApp =
        UkuiSearch::FakeAppInfoStore::createApp(QStringLiteral("/tmp/special.desktop"),
                                                QStringLiteral("Special"),
                                                QStringLiteral("Game"),
                                                QStringLiteral("S"),
                                                QStringLiteral("2024-01-04 00:00:00"));
    visibleApp.insert(UkuiSearch::ApplicationProperty::DontDisplay, 1);
    UkuiSearch::FakeAppInfoStore::g_apps.insert(QStringLiteral("/tmp/special.desktop"), visibleApp);

    UkuiSearch::ApplicationInfoMap specialDeleteMap;
    UkuiSearch::ApplicationPropertyMap specialDeleteUpdate;
    specialDeleteUpdate.insert(UkuiSearch::ApplicationProperty::DontDisplay, 1);
    specialDeleteMap.insert(QStringLiteral("/tmp/special.desktop"), specialDeleteUpdate);
    Q_EMIT appInfo->appDBItems2BUpdate(specialDeleteMap);
    QCOMPARE(updateSpy.count(), 1);
    auto updates = qvariant_cast<AppUpdateVector>(updateSpy.takeFirst().at(0));
    QCOMPARE(updates.size(), 1);
    QCOMPARE(updates.at(0).first.id(), QStringLiteral("/tmp/special.desktop"));
    QCOMPARE(updates.at(0).first.dontDisplay(), 1);
    QCOMPARE(updates.at(0).second, QVector<int>());
    QCOMPARE(deleteSpy.count(), 0);

    visibleApp.insert(UkuiSearch::ApplicationProperty::DontDisplay, 0);
    UkuiSearch::FakeAppInfoStore::g_apps.insert(QStringLiteral("/tmp/special.desktop"), visibleApp);

    UkuiSearch::ApplicationInfoMap specialAddMap;
    UkuiSearch::ApplicationPropertyMap specialAddUpdate;
    specialAddUpdate.insert(UkuiSearch::ApplicationProperty::DontDisplay, 0);
    specialAddMap.insert(QStringLiteral("/tmp/special.desktop"), specialAddUpdate);
    Q_EMIT appInfo->appDBItems2BUpdate(specialAddMap);
    QCOMPARE(updateSpy.count(), 1);
    updates = qvariant_cast<AppUpdateVector>(updateSpy.takeFirst().at(0));
    QCOMPARE(updates.size(), 1);
    QCOMPARE(updates.at(0).first.id(), QStringLiteral("/tmp/special.desktop"));
    QCOMPARE(updates.at(0).first.dontDisplay(), 0);
    QCOMPARE(updates.at(0).second, QVector<int>());

    visibleApp.insert(UkuiSearch::ApplicationProperty::DontDisplay, 1);
    visibleApp.insert(UkuiSearch::ApplicationProperty::LocalName, QStringLiteral("Special Hidden"));
    UkuiSearch::FakeAppInfoStore::g_apps.insert(QStringLiteral("/tmp/special.desktop"), visibleApp);
    Q_EMIT appInfo->appDBItems2BUpdateAll({QStringLiteral("/tmp/special.desktop")});
    QCOMPARE(updateSpy.count(), 1);
    updates = qvariant_cast<AppUpdateVector>(updateSpy.takeFirst().at(0));
    QCOMPARE(updates.size(), 1);
    QCOMPARE(updates.at(0).first.id(), QStringLiteral("/tmp/special.desktop"));
    QCOMPARE(updates.at(0).first.dontDisplay(), 1);
    QCOMPARE(updates.at(0).first.name(), QStringLiteral("Special Hidden"));
    QCOMPARE(updates.at(0).second, QVector<int>());
    QCOMPARE(deleteSpy.count(), 0);

    visibleApp.insert(UkuiSearch::ApplicationProperty::DontDisplay, 0);
    visibleApp.insert(UkuiSearch::ApplicationProperty::LocalName, QStringLiteral("Special Updated"));
    UkuiSearch::FakeAppInfoStore::g_apps.insert(QStringLiteral("/tmp/special.desktop"), visibleApp);
    Q_EMIT appInfo->appDBItems2BUpdateAll({QStringLiteral("/tmp/special.desktop")});
    QCOMPARE(updateSpy.count(), 1);
    updates = qvariant_cast<AppUpdateVector>(updateSpy.takeFirst().at(0));
    QCOMPARE(updates.size(), 1);
    QCOMPARE(updates.at(0).first.id(), QStringLiteral("/tmp/special.desktop"));
    QCOMPARE(updates.at(0).first.name(), QStringLiteral("Special Updated"));
    QCOMPARE(updates.at(0).first.dontDisplay(), 0);

    UkuiSearch::ApplicationPropertyMap duplicateApp =
        UkuiSearch::FakeAppInfoStore::createApp(QStringLiteral("/tmp/duplicated.desktop"),
                                                QStringLiteral("Duplicated"),
                                                QStringLiteral("Utility"),
                                                QStringLiteral("D"),
                                                QStringLiteral("2024-01-05 00:00:00"));
    UkuiSearch::ApplicationPropertyMap replacementApp =
        UkuiSearch::FakeAppInfoStore::createApp(QStringLiteral("/usr/share/applications/duplicated.desktop"),
                                                QStringLiteral("Replacement"),
                                                QStringLiteral("Utility"),
                                                QStringLiteral("R"),
                                                QStringLiteral("2024-01-05 00:00:00"));
    UkuiSearch::FakeAppInfoStore::g_apps.insert(QStringLiteral("/tmp/duplicated.desktop"), duplicateApp);
    UkuiSearch::FakeAppInfoStore::g_apps.insert(QStringLiteral("/usr/share/applications/duplicated.desktop"), replacementApp);
    UkuiSearch::FakeAppInfoStore::g_desktopPathByName.insert(QStringLiteral("duplicated"),
                                                             QStringLiteral("/usr/share/applications/duplicated.desktop"));

    Q_EMIT appInfo->appDBItems2BAdd({QStringLiteral("/tmp/duplicated.desktop")});
    QCOMPARE(addSpy.count(), 1);
    addedApps = qvariant_cast<DataEntityVector>(addSpy.takeFirst().at(0));
    QCOMPARE(addedApps.size(), 1);
    QCOMPARE(addedApps.at(0).id(), QStringLiteral("/usr/share/applications/duplicated.desktop"));
    QCOMPARE(addedApps.at(0).name(), QStringLiteral("Replacement"));

    Q_EMIT appInfo->appDBItems2BDelete({QStringLiteral("/tmp/duplicated.desktop"), QString()});
    QCOMPARE(updateSpy.count(), 1);
    updates = qvariant_cast<AppUpdateVector>(updateSpy.takeFirst().at(0));
    QCOMPARE(updates.size(), 1);
    QCOMPARE(updates.at(0).first.id(), QStringLiteral("/usr/share/applications/duplicated.desktop"));
    QCOMPARE(updates.at(0).second, QVector<int>());
    QCOMPARE(deleteSpy.count(), 1);
    QCOMPARE(deleteSpy.takeFirst().at(0).toStringList(), QStringList({QStringLiteral("/tmp/duplicated.desktop")}));
}

void appDatabaseInterface_test::test_updateAll_withoutSpecialProperties_emitsFullUpdate()
{
    AppDatabaseInterface interface;
    QSignalSpy updateSpy(&interface, &AppDatabaseInterface::appUpdated);

    auto *appInfo = interface.findChild<UkuiSearch::ApplicationInfo *>();
    QVERIFY(appInfo != nullptr);

    UkuiSearch::ApplicationPropertyMap app;
    app.insert(UkuiSearch::ApplicationProperty::DesktopFilePath, QStringLiteral("/tmp/full.desktop"));
    app.insert(UkuiSearch::ApplicationProperty::LocalName, QStringLiteral("Full Update"));
    app.insert(UkuiSearch::ApplicationProperty::Category, QStringLiteral("Network"));
    app.insert(UkuiSearch::ApplicationProperty::FirstLetterAll, QStringLiteral("F"));
    app.insert(UkuiSearch::ApplicationProperty::InsertTime, QStringLiteral("2024-03-01 00:00:00"));
    app.insert(UkuiSearch::ApplicationProperty::Top, 4);
    app.insert(UkuiSearch::ApplicationProperty::Lock, 1);
    app.insert(UkuiSearch::ApplicationProperty::Favorites, 5);
    app.insert(UkuiSearch::ApplicationProperty::LaunchTimes, 6);
    app.insert(UkuiSearch::ApplicationProperty::Icon, QStringLiteral("full-icon"));
    app.insert(UkuiSearch::ApplicationProperty::Launched, 1);
    UkuiSearch::FakeAppInfoStore::g_apps.insert(QStringLiteral("/tmp/full.desktop"), app);

    Q_EMIT appInfo->appDBItems2BUpdateAll({QStringLiteral("/tmp/full.desktop")});

    QCOMPARE(updateSpy.count(), 1);
    const auto updates = qvariant_cast<AppUpdateVector>(updateSpy.takeFirst().at(0));
    QCOMPARE(updates.size(), 1);
    QCOMPARE(updates.at(0).first.id(), QStringLiteral("/tmp/full.desktop"));
    QCOMPARE(updates.at(0).first.name(), QStringLiteral("Full Update"));
    QCOMPARE(updates.at(0).first.category(), QStringLiteral("Network"));
    QCOMPARE(updates.at(0).first.firstLetter(), QStringLiteral("F"));
    QCOMPARE(updates.at(0).first.insertTime(), QStringLiteral("2024-03-01 00:00:00"));
    QCOMPARE(updates.at(0).first.icon(), QStringLiteral("full-icon"));
    QCOMPARE(updates.at(0).first.launched(), 1);
    QCOMPARE(updates.at(0).first.favorite(), 5);
    QCOMPARE(updates.at(0).first.top(), 4);
    QCOMPARE(updates.at(0).first.isLock(), true);
    QCOMPARE(updates.at(0).second, QVector<int>());
}

QTEST_MAIN(appDatabaseInterface_test)

#include "app-database-interface-test.moc"
