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

#include "../../../../src/data-entity.h"
#include "../../../../src/extension/context-menu-extension.h"
#define private public
#include "../../../../src/extension/favorite/favorite-folder-helper.h"
#include "../../../../src/extension/favorite/favorites-config.h"
#include "../../../../src/libappdata/app-database-interface.h"
#undef private

using namespace UkuiMenu;

namespace {

QStringList g_configItems;
QStringList g_lastSyncedItems;
QString g_fixedFavoriteAppId;
int g_fixedFavoriteValue = -1;
QString g_updatedLaunchedAppId;
bool g_updatedLaunchedState = false;
QList<FavoritesFolder> g_folders;
QStringList g_removedFolderApps;
QString g_addAppToFolderAppId;
int g_addAppToFolderFolderId = -1;
QString g_addAppToNewFolderAppId;
QString g_addAppsToNewFolderFrom;
QString g_addAppsToNewFolderTo;
DataEntity g_lastShowMenuEntity;
MenuInfo::Location g_lastShowMenuLocation = MenuInfo::AppList;
QString g_lastShowMenuLocationId;
int g_showMenuCallCount = 0;

DataEntity createApp(const QString &id, int favorite = 0)
{
    DataEntity entity;
    entity.setId(id);
    entity.setType(DataType::Normal);
    entity.setName(id + QStringLiteral("-name"));
    entity.setIcon(id + QStringLiteral("-icon"));
    entity.setDesktopName(id + QStringLiteral(".desktop"));
    entity.setFavorite(favorite);
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
    g_fixedFavoriteAppId = appid;
    g_fixedFavoriteValue = index;
}

void AppDatabaseInterface::updateApLaunchedState(const QString &appid, bool state) const
{
    g_updatedLaunchedAppId = appid;
    g_updatedLaunchedState = state;
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
        return &m_database;
    }

    DataEntity appOfIndex(int row) const
    {
        return m_apps.value(row);
    }

    int indexOfApp(const QString &appid) const
    {
        for (int i = 0; i < m_apps.size(); ++i) {
            if (m_apps.at(i).id() == appid) {
                return i;
            }
        }

        return -1;
    }

    bool getAppById(const QString &appid, DataEntity &app) const
    {
        const int idx = indexOfApp(appid);
        if (idx < 0) {
            return false;
        }

        app = m_apps.at(idx);
        return true;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return m_apps.size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return 1;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= m_apps.size()) {
            return {};
        }

        return m_apps.at(index.row()).getValue(static_cast<DataEntity::PropertyName>(role));
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= m_apps.size()) {
            return false;
        }

        m_apps[index.row()].setValue(static_cast<DataEntity::PropertyName>(role), value);
        Q_EMIT dataChanged(index, index, QVector<int>{role});
        return true;
    }

    QHash<int, QByteArray> roleNames() const override
    {
        return DataEntity::AppRoleNames();
    }

    void resetApps(const QVector<DataEntity> &apps)
    {
        beginResetModel();
        m_apps = apps;
        endResetModel();
    }

private:
    BasicAppModel()
    {
    }

    QVector<DataEntity> m_apps;
    AppDatabaseInterface m_database;
};

FavoritesConfig &FavoritesConfig::instance()
{
    static FavoritesConfig config;
    return config;
}

FavoritesConfig::FavoritesConfig(QObject *parent)
    : QObject(parent)
{
}

void FavoritesConfig::sync(const QStringList &list)
{
    g_lastSyncedItems = list;
    g_configItems = list;
}

QStringList FavoritesConfig::getConfig()
{
    return g_configItems;
}

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
    Q_UNUSED(folderId);
    return true;
}

QList<FavoritesFolder> FavoriteFolderHelper::folderData()
{
    return g_folders;
}

void FavoriteFolderHelper::addAppToFolder(const QString &appId, const int &folderId)
{
    g_addAppToFolderAppId = appId;
    g_addAppToFolderFolderId = folderId;
}

void FavoriteFolderHelper::addAppToNewFolder(const QString &appId, const QString &folderName)
{
    Q_UNUSED(folderName);
    g_addAppToNewFolderAppId = appId;
}

void FavoriteFolderHelper::addAppsToNewFolder(const QString &idFrom, const QString &idTo, const QString &folderName)
{
    Q_UNUSED(folderName);
    g_addAppsToNewFolderFrom = idFrom;
    g_addAppsToNewFolderTo = idTo;
}

void FavoriteFolderHelper::removeAppFromFolder(const QString &appId)
{
    g_removedFolderApps << appId;
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

class ContextMenuManager : public QObject
{
    Q_OBJECT
public:
    static ContextMenuManager *instance()
    {
        static ContextMenuManager manager;
        return &manager;
    }

    void showMenu(const QString &appid, MenuInfo::Location location, const QString &lid = QString(), const QPoint &point = QPoint())
    {
        Q_UNUSED(appid);
        Q_UNUSED(location);
        Q_UNUSED(lid);
        Q_UNUSED(point);
    }

    void showMenu(const DataEntity &data, MenuInfo::Location location, const QString &lid = QString(), const QPoint &point = QPoint())
    {
        Q_UNUSED(point);
        g_lastShowMenuEntity = data;
        g_lastShowMenuLocation = location;
        g_lastShowMenuLocationId = lid;
        ++g_showMenuCallCount;
    }

    void setMainWindow(QWindow *mainWindow)
    {
        Q_UNUSED(mainWindow);
    }

    bool closeMenu()
    {
        return true;
    }
};

}

#define UKUI_MENU_BASIC_APP_MODEL_H
#define UKUI_MENU_FAVORITES_CONFIG_H
#define UKUI_MENU_FAVORITE_FOLDER_HELPER_H
#define UKUI_MENU_CONTEXT_MENU_MANAGER_H
#define private public
#include "../../../../src/extension/favorite/app-favorite-model.cpp"
#undef private

class appFavoriteModel_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_init_repairsConfigAndAddsMissingFavorites();
    void test_roleNamesAndRowCount();
    void test_data_forAppFolderFileAndInvalidRows();
    void test_changeFileState_addsAndRemovesEntries();
    void test_getOrderByIdAndGetDesktopFile();
    void test_openMenu_forNormalAndFolderEntries();
    void test_addAndRemoveFavoriteApp();
    void test_addAppsToNewFolderAndAddAppToFolder();
    void test_exchangedAppsOrder_and_clearFavorites();
    void test_privateUpdateFavoritesBranches();
    void test_privateOnAppUpdated_emitsDataChangedForNonFavoriteRoles();
    void test_privateOnAppUpdated_emptyRolesRefreshesFavoriteAndData();
    void test_privateOnAppRemoved_handlesFolderAndFavoriteRemoval();
    void test_privateFolderBranchHandlers();
};

void appFavoriteModel_test::init()
{
    g_configItems = QStringList{QStringLiteral("app://app.one"),
                                QStringLiteral("folder://11"),
                                QStringLiteral("file:///tmp/demo.txt")};
    g_lastSyncedItems.clear();
    g_fixedFavoriteAppId.clear();
    g_fixedFavoriteValue = -1;
    g_updatedLaunchedAppId.clear();
    g_updatedLaunchedState = false;
    g_folders.clear();
    g_removedFolderApps.clear();
    g_addAppToFolderAppId.clear();
    g_addAppToFolderFolderId = -1;
    g_addAppToNewFolderAppId.clear();
    g_addAppsToNewFolderFrom.clear();
    g_addAppsToNewFolderTo.clear();
    g_lastShowMenuEntity = DataEntity();
    g_lastShowMenuLocation = MenuInfo::AppList;
    g_lastShowMenuLocationId.clear();
    g_showMenuCallCount = 0;

    FavoritesFolder folder;
    folder.setId(11);
    folder.name = QStringLiteral("Tools");
    folder.apps = QStringList{QStringLiteral("app.two")};
    g_folders << folder;

    BasicAppModel::instance()->resetApps({
        createApp(QStringLiteral("app.one"), 1),
        createApp(QStringLiteral("app.two"), 1),
        createApp(QStringLiteral("app.three"), 0)
    });
}

void appFavoriteModel_test::test_init_repairsConfigAndAddsMissingFavorites()
{
    g_configItems = QStringList{QStringLiteral("app://app.three")};
    g_lastSyncedItems.clear();
    g_fixedFavoriteAppId.clear();
    g_fixedFavoriteValue = -1;
    g_folders.clear();

    BasicAppModel::instance()->resetApps({
        createApp(QStringLiteral("app.one"), 1),
        createApp(QStringLiteral("app.three"), 0)
    });

    AppFavoritesModel model;

    QCOMPARE(model.rowCount(), 2);
    QVERIFY(model.d->m_items.contains(QStringLiteral("app://app.one")));
    QVERIFY(model.d->m_items.contains(QStringLiteral("app://app.three")));
    QCOMPARE(g_fixedFavoriteAppId, QStringLiteral("app.three"));
    QCOMPARE(g_fixedFavoriteValue, 1);
    QVERIFY(g_lastSyncedItems.contains(QStringLiteral("app://app.one")));
}

void appFavoriteModel_test::test_roleNamesAndRowCount()
{
    AppFavoritesModel model;

    QCOMPARE(model.rowCount(), 3);
    const QHash<int, QByteArray> names = model.roleNames();
    QCOMPARE(names.value(DataEntity::Id), QByteArray("id"));
    QCOMPARE(names.value(DataEntity::DesktopName), QByteArray("desktopName"));
}

void appFavoriteModel_test::test_data_forAppFolderFileAndInvalidRows()
{
    AppFavoritesModel model;

    QCOMPARE(model.data(model.index(0, 0), DataEntity::Id).toString(), QStringLiteral("app.one"));
    QCOMPARE(model.data(model.index(0, 0), DataEntity::Name).toString(), QStringLiteral("app.one-name"));
    QCOMPARE(model.data(model.index(1, 0), DataEntity::Id).toString(), QStringLiteral("11"));
    QCOMPARE(model.data(model.index(1, 0), DataEntity::Name).toString(), QStringLiteral("Tools"));
    QCOMPARE(model.data(model.index(1, 0), DataEntity::Type).value<DataType::Type>(), DataType::Folder);
    QCOMPARE(model.data(model.index(2, 0), DataEntity::Id).toString(), QStringLiteral("/tmp/demo.txt"));
    QCOMPARE(model.data(model.index(2, 0), DataEntity::Name).toString(), QStringLiteral("demo.txt"));
    QCOMPARE(model.data(model.index(2, 0), DataEntity::Type).value<DataType::Type>(), DataType::Files);
    QVERIFY(!model.data(QModelIndex(), DataEntity::Id).isValid());
    QVERIFY(!model.data(model.index(99, 0), DataEntity::Id).isValid());
}

void appFavoriteModel_test::test_changeFileState_addsAndRemovesEntries()
{
    AppFavoritesModel model;

    model.changeFileState(QStringLiteral("/tmp/extra.txt"), true);
    QCOMPARE(model.getOrderById(QStringLiteral("file:///tmp/extra.txt")), model.rowCount() - 1);

    model.changeFileState(QStringLiteral("file:///tmp/extra.txt"), false);
    QCOMPARE(model.getOrderById(QStringLiteral("file:///tmp/extra.txt")), -1);
}

void appFavoriteModel_test::test_getOrderByIdAndGetDesktopFile()
{
    AppFavoritesModel model;

    QCOMPARE(model.getOrderById(QStringLiteral("app://app.one")), 0);
    QCOMPARE(model.getOrderById(QStringLiteral("folder://11")), 1);
    QCOMPARE(model.getDesktopFile(QStringLiteral("file:///tmp/test.desktop")), QStringLiteral("/tmp/test.desktop"));
}

void appFavoriteModel_test::test_openMenu_forNormalAndFolderEntries()
{
    AppFavoritesModel model;

    model.openMenu(-1);
    QCOMPARE(g_showMenuCallCount, 0);

    model.openMenu(0);
    QCOMPARE(g_showMenuCallCount, 1);
    QCOMPARE(g_lastShowMenuEntity.id(), QStringLiteral("app.one"));
    QCOMPARE(g_lastShowMenuLocation, MenuInfo::Extension);
    QCOMPARE(g_lastShowMenuLocationId, QStringLiteral("favorite"));

    model.openMenu(1);
    QCOMPARE(g_showMenuCallCount, 2);
    QCOMPARE(g_lastShowMenuEntity.id(), QStringLiteral("11"));
    QCOMPARE(g_lastShowMenuEntity.type(), DataType::Folder);
}

void appFavoriteModel_test::test_addAndRemoveFavoriteApp()
{
    AppFavoritesModel model;

    model.addAppToFavorites(QStringLiteral("app.three"));
    QCOMPARE(g_fixedFavoriteAppId, QStringLiteral("app.three"));
    QCOMPARE(g_fixedFavoriteValue, 1);
    QCOMPARE(g_updatedLaunchedAppId, QStringLiteral("app.three"));
    QCOMPARE(g_updatedLaunchedState, true);
    QVERIFY(model.getOrderById(QStringLiteral("app://app.three")) >= 0);

    model.removeAppFromFavorites(QStringLiteral("app.three"));
    QCOMPARE(g_fixedFavoriteAppId, QStringLiteral("app.three"));
    QCOMPARE(g_fixedFavoriteValue, 0);
    QCOMPARE(model.getOrderById(QStringLiteral("app://app.three")), -1);
}

void appFavoriteModel_test::test_addAppsToNewFolderAndAddAppToFolder()
{
    AppFavoritesModel model;

    model.addAppsToNewFolder(QStringLiteral("app.one"), QStringLiteral("app.two"));
    QCOMPARE(g_addAppsToNewFolderFrom, QStringLiteral("app.one"));
    QCOMPARE(g_addAppsToNewFolderTo, QStringLiteral("app.two"));

    model.addAppsToNewFolder(QStringLiteral("app.one"), QStringLiteral("app.one"));
    QCOMPARE(g_addAppsToNewFolderFrom, QStringLiteral("app.one"));

    model.addAppToFolder(QStringLiteral("app.one"), QString());
    QCOMPARE(g_addAppToNewFolderAppId, QStringLiteral("app.one"));

    model.addAppToFolder(QStringLiteral("app.one"), QStringLiteral("11"));
    QCOMPARE(g_addAppToFolderAppId, QStringLiteral("app.one"));
    QCOMPARE(g_addAppToFolderFolderId, 11);
}

void appFavoriteModel_test::test_exchangedAppsOrder_and_clearFavorites()
{
    AppFavoritesModel model;

    const QString beforeFirst = model.data(model.index(0, 0), DataEntity::Id).toString();
    model.exchangedAppsOrder(0, 2);
    QVERIFY(model.data(model.index(2, 0), DataEntity::Id).toString() == beforeFirst);

    model.exchangedAppsOrder(-1, 2);
    model.exchangedAppsOrder(1, 1);

    model.clearFavorites();
    QVERIFY(g_removedFolderApps.contains(QStringLiteral("app.two")));
    QCOMPARE(model.getOrderById(QStringLiteral("app://app.one")), -1);
}

void appFavoriteModel_test::test_privateUpdateFavoritesBranches()
{
    AppFavoritesModel model;

    model.d->updateFavoritesApps(QModelIndex());
    QCOMPARE(model.rowCount(), 3);

    BasicAppModel::instance()->setData(BasicAppModel::instance()->index(2, 0), 1, DataEntity::Favorite);
    model.d->updateFavoritesApps(BasicAppModel::instance()->index(2, 0));
    QVERIFY(model.d->m_items.contains(QStringLiteral("app://app.three")));

    FavoritesFolder folder;
    folder.setId(22);
    folder.name = QStringLiteral("Temp");
    folder.apps = QStringList{QStringLiteral("app.two")};
    g_folders = {folder};
    model.d->m_items << QStringLiteral("folder://22");
    model.d->m_folders = {22};

    BasicAppModel::instance()->setData(BasicAppModel::instance()->index(1, 0), 0, DataEntity::Favorite);
    model.d->updateFavoritesApps(BasicAppModel::instance()->index(1, 0));
    QVERIFY(g_removedFolderApps.contains(QStringLiteral("app.two")));
}

void appFavoriteModel_test::test_privateOnAppUpdated_emitsDataChangedForNonFavoriteRoles()
{
    AppFavoritesModel model;
    QSignalSpy spy(&model, &AppFavoritesModel::dataChanged);

    model.d->onAppUpdated(BasicAppModel::instance()->index(0, 0),
                          BasicAppModel::instance()->index(0, 0),
                          QVector<int>{DataEntity::Name});

    QCOMPARE(spy.count(), 1);
}

void appFavoriteModel_test::test_privateOnAppUpdated_emptyRolesRefreshesFavoriteAndData()
{
    AppFavoritesModel model;
    QSignalSpy spy(&model, &AppFavoritesModel::dataChanged);

    BasicAppModel::instance()->setData(BasicAppModel::instance()->index(2, 0), 1, DataEntity::Favorite);
    model.d->onAppUpdated(BasicAppModel::instance()->index(2, 0),
                          BasicAppModel::instance()->index(2, 0),
                          {});
    QVERIFY(model.d->m_items.contains(QStringLiteral("app://app.three")));
    QVERIFY(spy.count() >= 1);

    spy.clear();
    BasicAppModel::instance()->setData(BasicAppModel::instance()->index(0, 0), QStringLiteral("app.one-renamed"), DataEntity::Name);
    const int countAfterSourceSignal = spy.count();
    model.d->onAppUpdated(BasicAppModel::instance()->index(0, 0),
                          BasicAppModel::instance()->index(0, 0),
                          {});
    QCOMPARE(spy.count(), countAfterSourceSignal + 1);
    QCOMPARE(model.data(model.index(0, 0), DataEntity::Name).toString(), QStringLiteral("app.one-renamed"));
}

void appFavoriteModel_test::test_privateOnAppRemoved_handlesFolderAndFavoriteRemoval()
{
    AppFavoritesModel model;

    FavoritesFolder folder;
    folder.setId(33);
    folder.name = QStringLiteral("Folder");
    folder.apps = QStringList{QStringLiteral("app.one")};
    g_folders = {folder};
    model.d->m_folders = {33};
    if (!model.d->m_items.contains(QStringLiteral("folder://33"))) {
        model.d->m_items << QStringLiteral("folder://33");
    }

    model.d->onAppRemoved(QModelIndex(), 0, 0);

    QCOMPARE(model.getOrderById(QStringLiteral("app://app.one")), -1);
    QVERIFY(g_removedFolderApps.contains(QStringLiteral("app.one")));
}

void appFavoriteModel_test::test_privateFolderBranchHandlers()
{
    AppFavoritesModel model;
    QSignalSpy spy(&model, &AppFavoritesModel::dataChanged);

    FavoritesFolder folder;
    folder.setId(44);
    folder.name = QStringLiteral("Work");
    folder.apps = QStringList{QStringLiteral("app.one"), QStringLiteral("app.two")};
    g_folders = {folder};

    model.d->onFolderAdded(44, -1);
    QVERIFY(model.d->m_items.contains(QStringLiteral("folder://44")));
    QCOMPARE(model.getOrderById(QStringLiteral("app://app.one")), -1);
    QCOMPARE(model.getOrderById(QStringLiteral("app://app.two")), -1);

    model.d->onFolderAppChanged(44, QStringLiteral("app.one"), false);
    QVERIFY(model.d->m_items.contains(QStringLiteral("app://app.one")));
    QVERIFY(spy.count() >= 1);

    model.d->onFolderAppChanged(44, QStringLiteral("app.one"), true);
    QCOMPARE(model.getOrderById(QStringLiteral("app://app.one")), -1);

    model.d->onFolderDeleted(44, QStringList{QStringLiteral("app.one"), QStringLiteral("app.two")});
    QCOMPARE(model.getOrderById(QStringLiteral("folder://44")), -1);
    QVERIFY(model.d->m_items.contains(QStringLiteral("app://app.one")));
    QVERIFY(model.d->m_items.contains(QStringLiteral("app://app.two")));
}

QTEST_MAIN(appFavoriteModel_test)

#include "app-favorite-model-test.moc"
