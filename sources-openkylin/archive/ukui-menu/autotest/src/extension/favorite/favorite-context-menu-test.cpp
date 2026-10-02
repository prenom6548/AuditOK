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
#include <QMenu>

#define private public
#include "../../../src/data-entity.h"
#include "../../../src/extension/context-menu-extension.h"
#include "../../../src/extension/favorite/favorite-folder-helper.h"
#include "../../../src/extension/favorite/favorite-context-menu.h"
#undef private

using namespace UkuiMenu;

namespace {

QString g_addedFavoriteApp;
QString g_removedFavoriteApp;
QString g_folderAppId;
QString g_folderIdText;
QString g_deletedFolderId;
QString g_fixedFavoriteAppId;
int g_fixedFavoriteValue = -1;
QString g_eventName;
QString g_eventPage;
QList<FavoritesFolder> g_folders;

QAction *findActionByText(const QList<QAction *> &actions, const QString &text)
{
    for (QAction *action : actions) {
        if (action && action->text() == text) {
            return action;
        }
    }

    return nullptr;
}

}

namespace UkuiMenu {

class AppFavoritesModel
{
public:
    static AppFavoritesModel &instance()
    {
        static AppFavoritesModel model;
        return model;
    }

    void addAppToFavorites(const QString &id, int index = -1)
    {
        Q_UNUSED(index);
        g_addedFavoriteApp = id;
    }

    void removeAppFromFavorites(const QString &id)
    {
        g_removedFavoriteApp = id;
    }

    void addAppToFolder(const QString &appId, const QString &folderId)
    {
        g_folderAppId = appId;
        g_folderIdText = folderId;
    }
};

class EventTrack : public QObject
{
    Q_OBJECT
public:
    static EventTrack *instance()
    {
        static EventTrack track;
        return &track;
    }

    void sendDefaultEvent(const QString &event, const QString &page, const QVariantMap &map = {})
    {
        Q_UNUSED(map);
        g_eventName = event;
        g_eventPage = page;
    }
};

class AppDatabaseInterface : public QObject
{
    Q_OBJECT
public:
    explicit AppDatabaseInterface(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    void fixAppToFavorite(const QString &appid, int index) const
    {
        g_fixedFavoriteAppId = appid;
        g_fixedFavoriteValue = index;
    }
};

class BasicAppModel
{
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
};

FavoriteFolderHelper *FavoriteFolderHelper::instance()
{
    static FavoriteFolderHelper helper;
    return &helper;
}

FavoriteFolderHelper::FavoriteFolderHelper()
{
}

FavoriteFolderHelper::~FavoriteFolderHelper() = default;

QVariantList FavoriteFolderHelper::folderIcon(const FavoritesFolder &folder)
{
    return QVariantList{folder.getName(), folder.getApps().size()};
}

bool FavoriteFolderHelper::getFolderFromId(const int &folderId, FavoritesFolder &folder)
{
    for (const auto &candidate : qAsConst(g_folders)) {
        if (candidate.getId() == folderId) {
            folder = candidate;
            return true;
        }
    }

    return false;
}

bool FavoriteFolderHelper::containsFolder(const int &folderId)
{
    FavoritesFolder folder;
    return getFolderFromId(folderId, folder);
}

bool FavoriteFolderHelper::containApp(const QString &appId)
{
    for (const auto &folder : qAsConst(g_folders)) {
        if (folder.getApps().contains(appId)) {
            return true;
        }
    }

    return false;
}

bool FavoriteFolderHelper::deleteFolder(const int &folderId)
{
    g_deletedFolderId = QString::number(folderId);
    return true;
}

QList<FavoritesFolder> FavoriteFolderHelper::folderData()
{
    return g_folders;
}

void FavoriteFolderHelper::addAppToFolder(const QString &appId, const int &folderId)
{
    g_folderAppId = appId;
    g_folderIdText = QString::number(folderId);
}

void FavoriteFolderHelper::addAppToNewFolder(const QString &appId, const QString &folderName)
{
    g_folderAppId = appId;
    g_folderIdText = folderName;
}

void FavoriteFolderHelper::addAppsToNewFolder(const QString &idFrom, const QString &idTo, const QString &folderName)
{
    Q_UNUSED(idFrom);
    Q_UNUSED(idTo);
    Q_UNUSED(folderName);
}

void FavoriteFolderHelper::removeAppFromFolder(const QString &appId)
{
    g_removedFavoriteApp = appId;
}

QString FavoriteFolderHelper::getFolderName(const int &folderId)
{
    FavoritesFolder folder;
    return getFolderFromId(folderId, folder) ? folder.getName() : QString();
}

void FavoriteFolderHelper::renameFolder(const int &folderId, const QString &folderName)
{
    Q_UNUSED(folderId);
    Q_UNUSED(folderName);
}

void FavoriteFolderHelper::exchangedOrder(const int &indexFrom, const int &indexTo, const int &folderId)
{
    Q_UNUSED(indexFrom);
    Q_UNUSED(indexTo);
    Q_UNUSED(folderId);
}

void FavoriteFolderHelper::forceSync()
{
}

QStringList FavoriteFolderHelper::appsInFolders()
{
    return {};
}

QStringList FavoritesFolder::getDisplayApps() const
{
    return apps;
}

}

#define UKUI_MENU_APP_FAVORITE_MODEL_H
#define UKUI_MENU_EVENT_TRACK_H
#define UKUI_MENU_BASIC_APP_MODEL_H
#include "../../../src/extension/favorite/favorite-context-menu.cpp"

class favoriteContextMenu_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_index();
    void test_actions_returnsEmptyWithoutParent();
    void test_actions_forFolderData();
    void test_actions_addToFavorite();
    void test_actions_removeFromFavorite();
    void test_actions_forFolderPageAndNegativeFavorite();
    void test_actions_forExtension();
    void test_actions_forExtensionWithoutExistingFolders();
    void test_actions_forFolderLocation();
    void test_actions_forUnknownLocation();
};

void favoriteContextMenu_test::init()
{
    g_addedFavoriteApp.clear();
    g_removedFavoriteApp.clear();
    g_folderAppId.clear();
    g_folderIdText.clear();
    g_deletedFolderId.clear();
    g_fixedFavoriteAppId.clear();
    g_fixedFavoriteValue = -1;
    g_eventName.clear();
    g_eventPage.clear();
    g_folders.clear();
}

void favoriteContextMenu_test::test_index()
{
    FavoriteContextMenu menu;
    QCOMPARE(menu.index(), 512);
}

void favoriteContextMenu_test::test_actions_returnsEmptyWithoutParent()
{
    FavoriteContextMenu menu;
    DataEntity data;
    data.setType(DataType::Normal);
    data.setId(QStringLiteral("app.demo"));

    QVERIFY(menu.actions(data, nullptr, MenuInfo::AppList, QString()).isEmpty());
}

void favoriteContextMenu_test::test_actions_forFolderData()
{
    FavoriteContextMenu menu;
    QMenu parent;
    DataEntity data;
    data.setType(DataType::Folder);
    data.setId(QStringLiteral("42"));

    const QList<QAction *> actions = menu.actions(data, &parent, MenuInfo::Extension, QString());

    QCOMPARE(actions.size(), 1);
    QCOMPARE(actions.first()->text(), QStringLiteral("Dissolve folder"));

    actions.first()->trigger();
    QCOMPARE(g_deletedFolderId, QStringLiteral("42"));
}

void favoriteContextMenu_test::test_actions_addToFavorite()
{
    FavoriteContextMenu menu;
    QMenu parent;
    DataEntity data;
    data.setType(DataType::Normal);
    data.setId(QStringLiteral("demo.app"));
    data.setFavorite(0);

    const QList<QAction *> actions = menu.actions(data, &parent, MenuInfo::AppList, QString());

    QCOMPARE(actions.size(), 1);
    QCOMPARE(actions.first()->text(), QStringLiteral("Add to favorite"));

    actions.first()->trigger();
    QCOMPARE(g_addedFavoriteApp, QStringLiteral("demo.app"));
    QCOMPARE(g_eventName, QStringLiteral("fix_to_favorite"));
    QCOMPARE(g_eventPage, QStringLiteral("Right-click Menu"));
}

void favoriteContextMenu_test::test_actions_removeFromFavorite()
{
    FavoriteContextMenu menu;
    QMenu parent;
    DataEntity data;
    data.setType(DataType::Normal);
    data.setId(QStringLiteral("demo.app"));
    data.setFavorite(3);

    const QList<QAction *> actions = menu.actions(data, &parent, MenuInfo::FullScreen, QString());

    QCOMPARE(actions.size(), 1);
    QCOMPARE(actions.first()->text(), QStringLiteral("Remove from favorite"));

    actions.first()->trigger();
    QCOMPARE(g_removedFavoriteApp, QStringLiteral("demo.app"));
    QCOMPARE(g_fixedFavoriteAppId, QStringLiteral("demo.app"));
    QCOMPARE(g_fixedFavoriteValue, 0);
    QCOMPARE(g_eventName, QStringLiteral("remove_from_favorite"));
}

void favoriteContextMenu_test::test_actions_forFolderPageAndNegativeFavorite()
{
    FavoriteContextMenu menu;
    QMenu parent;
    DataEntity data;
    data.setType(DataType::Normal);
    data.setId(QStringLiteral("demo.app"));

    data.setFavorite(0);
    QList<QAction *> actions = menu.actions(data, &parent, MenuInfo::FolderPage, QString());
    QCOMPARE(actions.size(), 1);
    QCOMPARE(actions.first()->text(), QStringLiteral("Add to favorite"));
    actions.first()->trigger();
    QCOMPARE(g_addedFavoriteApp, QStringLiteral("demo.app"));

    init();
    data.setFavorite(1);
    actions = menu.actions(data, &parent, MenuInfo::FolderPage, QString());
    QCOMPARE(actions.size(), 1);
    QCOMPARE(actions.first()->text(), QStringLiteral("Remove from favorite"));
    actions.first()->trigger();
    QCOMPARE(g_removedFavoriteApp, QStringLiteral("demo.app"));
    QCOMPARE(g_fixedFavoriteValue, 0);

    data.setFavorite(-1);
    QVERIFY(menu.actions(data, &parent, MenuInfo::FolderPage, QString()).isEmpty());
}

void favoriteContextMenu_test::test_actions_forExtension()
{
    FavoriteContextMenu menu;
    QMenu parent;

    FavoritesFolder folder;
    folder.setId(7);
    folder.name = QStringLiteral("Folder Seven");
    folder.apps = QStringList{QStringLiteral("another.app")};
    g_folders << folder;

    DataEntity data;
    data.setType(DataType::Normal);
    data.setId(QStringLiteral("demo.app"));
    data.setFavorite(2);

    const QList<QAction *> actions = menu.actions(data, &parent, MenuInfo::Extension, QStringLiteral("favorite"));

    QCOMPARE(actions.size(), 2);
    QCOMPARE(actions.at(0)->text(), QStringLiteral("Move to folder"));
    QVERIFY(actions.at(0)->menu() != nullptr);
    QCOMPARE(actions.at(0)->menu()->actions().size(), 2);
    QCOMPARE(actions.at(0)->menu()->actions().at(0)->text(), QStringLiteral("Create a new folder"));
    QCOMPARE(actions.at(0)->menu()->actions().at(1)->text(), QStringLiteral("Add to \"Folder Seven\""));
    QCOMPARE(actions.at(1)->text(), QStringLiteral("Remove from favorite"));

    actions.at(0)->menu()->actions().at(0)->trigger();
    QCOMPARE(g_folderAppId, QStringLiteral("demo.app"));
    QCOMPARE(g_folderIdText, QString());

    actions.at(0)->menu()->actions().at(1)->trigger();
    QCOMPARE(g_folderAppId, QStringLiteral("demo.app"));
    QCOMPARE(g_folderIdText, QStringLiteral("7"));

    actions.at(1)->trigger();
    QCOMPARE(g_removedFavoriteApp, QStringLiteral("demo.app"));
    QCOMPARE(g_eventName, QStringLiteral("remove_from_favorite"));
}

void favoriteContextMenu_test::test_actions_forExtensionWithoutExistingFolders()
{
    FavoriteContextMenu menu;
    QMenu parent;
    DataEntity data;
    data.setType(DataType::Normal);
    data.setId(QStringLiteral("demo.app"));
    data.setFavorite(2);

    const QList<QAction *> actions = menu.actions(data, &parent, MenuInfo::Extension, QStringLiteral("favorite"));

    QCOMPARE(actions.size(), 2);
    QVERIFY(actions.first()->menu() != nullptr);
    QCOMPARE(actions.first()->menu()->actions().size(), 1);

    actions.first()->menu()->actions().first()->trigger();
    QCOMPARE(g_folderAppId, QStringLiteral("demo.app"));
    QCOMPARE(g_folderIdText, QString());
}

void favoriteContextMenu_test::test_actions_forFolderLocation()
{
    FavoriteContextMenu menu;
    QMenu parent;
    DataEntity data;
    data.setType(DataType::Normal);
    data.setId(QStringLiteral("demo.app"));

    const QList<QAction *> actions = menu.actions(data, &parent, MenuInfo::Folder, QString());

    QCOMPARE(actions.size(), 1);
    QCOMPARE(actions.first()->text(), QStringLiteral("Remove from folder"));

    actions.first()->trigger();
    QCOMPARE(g_removedFavoriteApp, QStringLiteral("demo.app"));
}

void favoriteContextMenu_test::test_actions_forUnknownLocation()
{
    FavoriteContextMenu menu;
    QMenu parent;
    DataEntity data;
    data.setType(DataType::Normal);
    data.setId(QStringLiteral("demo.app"));
    data.setFavorite(0);

    const QList<QAction *> actions = menu.actions(data, &parent, static_cast<MenuInfo::Location>(999), QString());

    QVERIFY(actions.isEmpty());
}

QTEST_MAIN(favoriteContextMenu_test)

#include "favorite-context-menu-test.moc"
