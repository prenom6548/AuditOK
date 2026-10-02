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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "../../../src/data-entity.h"
#define private public
#include "../../../src/extension/favorite/favorite-folder-helper.h"
#include "../../../src/extension/favorite/favorites-config.h"
#include "../../../src/libappdata/app-database-interface.h"
#include "../../../src/utils/security-function-control.h"
#include "../../../src/utils/event-track.h"
#undef private

using namespace UkuiMenu;

namespace {

QString g_eventName;
QString g_eventPage;
QStringList g_visibleApps;
QStringList g_knownApps;
QMap<QString, DataEntity> g_appMap;
QTemporaryDir *g_tempDir = nullptr;

DataEntity createApp(const QString &id)
{
    DataEntity entity;
    entity.setId(id);
    entity.setType(DataType::Normal);
    entity.setIcon(id + QStringLiteral("-icon"));
    entity.setDesktopName(id + QStringLiteral(".desktop"));
    return entity;
}

}

namespace UkuiMenu {

AppDatabaseInterface::AppDatabaseInterface(QObject *parent)
    : QObject(parent)
{
}

DataEntityVector AppDatabaseInterface::apps() const
{
    return {};
}

bool AppDatabaseInterface::getApp(const QString &appid, DataEntity &app) const
{
    Q_UNUSED(appid);
    Q_UNUSED(app);
    return false;
}

void AppDatabaseInterface::fixAppToTop(const QString &appid, int index) const
{
    Q_UNUSED(appid);
    Q_UNUSED(index);
}

void AppDatabaseInterface::fixAppToFavorite(const QString &appid, int index) const
{
    Q_UNUSED(appid);
    Q_UNUSED(index);
}

void AppDatabaseInterface::updateApLaunchedState(const QString &appid, bool state) const
{
    Q_UNUSED(appid);
    Q_UNUSED(state);
}

class BasicAppModel : public QAbstractListModel
{
    Q_OBJECT
public:
    static BasicAppModel *instance()
    {
        static BasicAppModel model;
        return &model;
    }

    const AppDatabaseInterface *databaseInterface() const
    {
        static AppDatabaseInterface database;
        return &database;
    }

    DataEntity appOfIndex(int row) const
    {
        Q_UNUSED(row);
        return {};
    }

    int indexOfApp(const QString &appid) const
    {
        return g_knownApps.indexOf(appid);
    }

    bool getAppById(const QString &appid, DataEntity &app) const
    {
        if (!g_appMap.contains(appid)) {
            return false;
        }
        app = g_appMap.value(appid);
        return true;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return g_knownApps.size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return 1;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        Q_UNUSED(index);
        Q_UNUSED(role);
        return {};
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        Q_UNUSED(index);
        Q_UNUSED(value);
        Q_UNUSED(role);
        return false;
    }

    QHash<int, QByteArray> roleNames() const override
    {
        return {};
    }
};

class AppFavoritesModel
{
public:
    static AppFavoritesModel &instance()
    {
        static AppFavoritesModel model;
        return model;
    }

    int getOrderById(const QString &id)
    {
        return g_knownApps.indexOf(id.mid(QStringLiteral("app://").size()));
    }
};

EventTrack *EventTrack::instance()
{
    static EventTrack track;
    return &track;
}

EventTrack::EventTrack(QObject *parent)
    : QObject(parent)
{
}

EventTrack *EventTrack::qmlAttachedProperties(QObject *object)
{
    Q_UNUSED(object);
    return instance();
}

void EventTrack::sendClickEvent(const QString &event, const QString &page, const QVariantMap &map)
{
    Q_UNUSED(event);
    Q_UNUSED(page);
    Q_UNUSED(map);
}

void EventTrack::sendDefaultEvent(const QString &event, const QString &page, const QVariantMap &map)
{
    Q_UNUSED(map);
    g_eventName = event;
    g_eventPage = page;
}

void EventTrack::sendSearchEvent(const QString &event, const QString &page, const QString &content)
{
    Q_UNUSED(event);
    Q_UNUSED(page);
    Q_UNUSED(content);
}

SecurityFunctionControl *SecurityFunctionControl::instance()
{
    static SecurityFunctionControl control;
    return &control;
}

SecurityFunctionControl::SecurityFunctionControl(QObject *parent)
    : QObject(parent)
{
}

bool SecurityFunctionControl::disable() const { return false; }
bool SecurityFunctionControl::disableMouseLeftButton() const { return false; }
bool SecurityFunctionControl::disableMouseRightButton() const { return false; }
bool SecurityFunctionControl::disableMouseHover() const { return false; }
QStringList SecurityFunctionControl::appWhiteList() const { return {}; }
QStringList SecurityFunctionControl::appBlackList() const { return {}; }
bool SecurityFunctionControl::disableUninstallAction() const { return false; }
bool SecurityFunctionControl::windowMaximize() const { return true; }
bool SecurityFunctionControl::showFoldButton() const { return true; }
QString SecurityFunctionControl::powerButtonSoleOperation() const { return {}; }
bool SecurityFunctionControl::compareListBySort(QStringList &list1, QStringList &list2) { return list1 == list2; }
void SecurityFunctionControl::onFunctionControlChanged(const QString &name, bool exit) { Q_UNUSED(name); Q_UNUSED(exit); }
bool SecurityFunctionControl::canAppDisplay(const QString &desktop) const { return g_visibleApps.contains(desktop); }

}

#define UKUI_MENU_APP_FAVORITE_MODEL_H
#define UKUI_MENU_EVENT_TRACK_H
#define UKUI_MENU_BASIC_APP_MODEL_H
#define SECURITYFUNCTIONCONTROL_H
#include "../../../src/extension/favorite/favorite-folder-helper.cpp"

class favoriteFolderHelper_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void test_instance_returnsSingleton();
    void test_emptyInputsAndInvalidFolders_doNothing();
    void test_addAndRemoveAppsAndFolderLifecycle();
    void test_addAppsToNewFolder_and_queries();
    void test_renameExchangeAndFolderIcon();
    void test_init_filtersInvalidConfig_and_appsInFolders();
    void test_init_rewritesOldVersionAndSkipsMalformedFolders();
    void test_directoryConfigPath_isIgnoredForReadAndWrite();
};

void favoriteFolderHelper_test::init()
{
    g_eventName.clear();
    g_eventPage.clear();
    g_visibleApps = QStringList{QStringLiteral("app.one"), QStringLiteral("app.two"), QStringLiteral("app.three")};
    g_knownApps = QStringList{QStringLiteral("app.one"), QStringLiteral("app.two"), QStringLiteral("app.three")};
    g_appMap.clear();
    g_appMap.insert(QStringLiteral("app.one"), createApp(QStringLiteral("app.one")));
    g_appMap.insert(QStringLiteral("app.two"), createApp(QStringLiteral("app.two")));
    g_appMap.insert(QStringLiteral("app.three"), createApp(QStringLiteral("app.three")));

    delete g_tempDir;
    g_tempDir = new QTemporaryDir;
    QVERIFY(g_tempDir->isValid());

    FavoriteFolderHelper::s_folderConfigFile = g_tempDir->path() + QStringLiteral("/folder.json");
}

void favoriteFolderHelper_test::cleanup()
{
    delete g_tempDir;
    g_tempDir = nullptr;
}

void favoriteFolderHelper_test::test_instance_returnsSingleton()
{
    FavoriteFolderHelper *first = FavoriteFolderHelper::instance();
    FavoriteFolderHelper *second = FavoriteFolderHelper::instance();

    QVERIFY(first != nullptr);
    QCOMPARE(first, second);
}

void favoriteFolderHelper_test::test_emptyInputsAndInvalidFolders_doNothing()
{
    FavoriteFolderHelper helper;
    QSignalSpy addedSpy(&helper, &FavoriteFolderHelper::folderAdded);
    QSignalSpy changedSpy(&helper, &FavoriteFolderHelper::folderAppChanged);
    QSignalSpy nameSpy(&helper, &FavoriteFolderHelper::folderNameChanged);
    QSignalSpy appsSpy(&helper, &FavoriteFolderHelper::folderAppsChanged);
    QSignalSpy deleteSpy(&helper, &FavoriteFolderHelper::folderToBeDeleted);

    helper.addAppToFolder(QString(), 1);
    helper.addAppToFolder(QStringLiteral("app.one"), 999);
    helper.addAppToNewFolder(QString(), QStringLiteral("Empty"));
    helper.addAppsToNewFolder(QString(), QStringLiteral("app.two"), QStringLiteral("Pair"));
    helper.addAppsToNewFolder(QStringLiteral("app.one"), QString(), QStringLiteral("Pair"));
    helper.removeAppFromFolder(QString());
    helper.renameFolder(999, QStringLiteral("Missing"));
    helper.exchangedOrder(0, 0, 999);
    helper.exchangedOrder(0, 1, 999);

    QCOMPARE(helper.folderData().size(), 0);
    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(changedSpy.count(), 0);
    QCOMPARE(nameSpy.count(), 0);
    QCOMPARE(appsSpy.count(), 0);
    QCOMPARE(deleteSpy.count(), 0);
    QVERIFY(!helper.containsFolder(999));
    QVERIFY(helper.getFolderName(999).isEmpty());

    FavoritesFolder folder;
    QVERIFY(!helper.getFolderFromId(999, folder));
}

void favoriteFolderHelper_test::test_addAndRemoveAppsAndFolderLifecycle()
{
    FavoriteFolderHelper helper;
    QSignalSpy addedSpy(&helper, &FavoriteFolderHelper::folderAdded);
    QSignalSpy changedSpy(&helper, &FavoriteFolderHelper::folderAppChanged);
    QSignalSpy deleteSpy(&helper, &FavoriteFolderHelper::folderToBeDeleted);

    helper.addAppToNewFolder(QStringLiteral("app.one"), QString());
    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(g_eventName, QStringLiteral("add_app_to_new_folder"));
    QVERIFY(helper.containApp(QStringLiteral("app.one")));

    const int folderId = helper.folderData().first().getId();
    helper.addAppToFolder(QStringLiteral("app.two"), folderId);
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(g_eventName, QStringLiteral("add_app_to_folder"));
    QVERIFY(helper.containApp(QStringLiteral("app.two")));

    helper.addAppToFolder(QStringLiteral("app.two"), folderId);
    QCOMPARE(changedSpy.count(), 1);

    helper.removeAppFromFolder(QStringLiteral("app.two"));
    QVERIFY(changedSpy.count() >= 2);
    QVERIFY(!helper.containApp(QStringLiteral("app.two")));

    helper.removeAppFromFolder(QStringLiteral("app.one"));
    QCOMPARE(deleteSpy.count(), 1);
}

void favoriteFolderHelper_test::test_addAppsToNewFolder_and_queries()
{
    FavoriteFolderHelper helper;
    QSignalSpy addedSpy(&helper, &FavoriteFolderHelper::folderAdded);

    helper.addAppsToNewFolder(QStringLiteral("app.one"), QStringLiteral("app.three"), QStringLiteral("Work"));
    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(helper.folderData().size(), 1);

    const FavoritesFolder folder = helper.folderData().first();
    QCOMPARE(folder.getName(), QStringLiteral("Work"));
    QCOMPARE(folder.getApps(), QStringList({QStringLiteral("app.one"), QStringLiteral("app.three")}));
    QVERIFY(helper.containsFolder(folder.getId()));
    QCOMPARE(helper.getFolderName(folder.getId()), QStringLiteral("Work"));

    FavoritesFolder fetched;
    QVERIFY(helper.getFolderFromId(folder.getId(), fetched));
    QCOMPARE(fetched.getApps().size(), 2);

    helper.addAppsToNewFolder(QStringLiteral("app.three"), QStringLiteral("app.one"), QString());
    QCOMPARE(addedSpy.count(), 2);
    QCOMPARE(helper.folderData().last().getName(), QStringLiteral("New Folder 2"));
}

void favoriteFolderHelper_test::test_renameExchangeAndFolderIcon()
{
    FavoriteFolderHelper helper;
    helper.addAppsToNewFolder(QStringLiteral("app.one"), QStringLiteral("app.two"), QStringLiteral("Work"));
    const int folderId = helper.folderData().first().getId();

    QSignalSpy nameSpy(&helper, &FavoriteFolderHelper::folderNameChanged);
    QSignalSpy appsSpy(&helper, &FavoriteFolderHelper::folderAppsChanged);

    helper.renameFolder(folderId, QStringLiteral("Renamed"));
    QCOMPARE(nameSpy.count(), 1);
    QCOMPARE(helper.getFolderName(folderId), QStringLiteral("Renamed"));

    helper.renameFolder(folderId, QStringLiteral("Renamed"));
    QCOMPARE(nameSpy.count(), 1);

    helper.exchangedOrder(0, 1, folderId);
    QCOMPARE(appsSpy.count(), 1);

    FavoritesFolder folder;
    QVERIFY(helper.getFolderFromId(folderId, folder));
    const QVariantList icons = FavoriteFolderHelper::folderIcon(folder);
    QCOMPARE(icons.size(), 2);
    QVERIFY(icons.first().toMap().value(QStringLiteral("icon")).toString().contains(QStringLiteral("icon")));

    g_visibleApps << QStringLiteral("missing.app");
    folder.apps << QStringLiteral("missing.app");
    QCOMPARE(FavoriteFolderHelper::folderIcon(folder).size(), 2);
}

void favoriteFolderHelper_test::test_init_filtersInvalidConfig_and_appsInFolders()
{
    QJsonObject folderObject;
    folderObject.insert(QStringLiteral("name"), QStringLiteral("Saved"));
    folderObject.insert(QStringLiteral("id"), 5);
    folderObject.insert(QStringLiteral("apps"), QJsonArray{
        QStringLiteral("app.one"),
        QStringLiteral("invalid.app")
    });

    QJsonObject emptyAfterFilteringFolder;
    emptyAfterFilteringFolder.insert(QStringLiteral("name"), QStringLiteral("InvalidOnly"));
    emptyAfterFilteringFolder.insert(QStringLiteral("id"), 6);
    emptyAfterFilteringFolder.insert(QStringLiteral("apps"), QJsonArray{QStringLiteral("invalid.only")});

    QJsonObject malformedFolder;
    malformedFolder.insert(QStringLiteral("name"), QStringLiteral("Malformed"));
    malformedFolder.insert(QStringLiteral("id"), 7);

    QJsonObject root;
    root.insert(QStringLiteral("version"), QStringLiteral("1.0.0"));
    root.insert(QStringLiteral("folders"), QJsonArray{folderObject, emptyAfterFilteringFolder, malformedFolder});

    QFile file(FavoriteFolderHelper::s_folderConfigFile);
    QVERIFY(file.open(QFile::WriteOnly));
    file.write(QJsonDocument(root).toJson());
    file.close();

    FavoriteFolderHelper helper;
    QCOMPARE(helper.folderData().size(), 1);
    QCOMPARE(helper.folderData().first().getApps(), QStringList({QStringLiteral("app.one")}));
    QCOMPARE(helper.appsInFolders(), QStringList({QStringLiteral("app.one")}));

    g_visibleApps.clear();
    QCOMPARE(helper.folderData().first().getDisplayApps(), QStringList());
}

void favoriteFolderHelper_test::test_init_rewritesOldVersionAndSkipsMalformedFolders()
{
    QJsonObject malformedFolder;
    malformedFolder.insert(QStringLiteral("name"), QStringLiteral("Broken"));
    malformedFolder.insert(QStringLiteral("id"), 8);

    QJsonObject root;
    root.insert(QStringLiteral("version"), QStringLiteral("0.9.0"));
    root.insert(QStringLiteral("folders"), QJsonArray{malformedFolder});

    QFile file(FavoriteFolderHelper::s_folderConfigFile);
    QVERIFY(file.open(QFile::WriteOnly));
    file.write(QJsonDocument(root).toJson());
    file.close();

    {
        FavoriteFolderHelper helper;
        QVERIFY(helper.folderData().isEmpty());
    }

    QVERIFY(file.open(QFile::ReadOnly));
    const QJsonDocument savedDocument = QJsonDocument::fromJson(file.readAll());
    file.close();
    QCOMPARE(savedDocument.object().value(QStringLiteral("version")).toString(), QStringLiteral("1.0.0"));
    QCOMPARE(savedDocument.object().value(QStringLiteral("folders")).toArray().size(), 0);
}

void favoriteFolderHelper_test::test_directoryConfigPath_isIgnoredForReadAndWrite()
{
    FavoriteFolderHelper::s_folderConfigFile = g_tempDir->path();

    FavoriteFolderHelper helper;
    QVERIFY(helper.folderData().isEmpty());

    helper.forceSync();
    QVERIFY(QFileInfo(FavoriteFolderHelper::s_folderConfigFile).isDir());
}

QTEST_MAIN(favoriteFolderHelper_test)

#include "favorite-folder-helper-test.moc"
