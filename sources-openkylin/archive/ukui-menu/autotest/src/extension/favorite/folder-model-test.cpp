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

#include "../../../src/data-entity.h"
#include "../../../src/commons.h"
#include "../../../src/extension/context-menu-extension.h"
#define private public
#include "../../../src/extension/favorite/favorite-folder-helper.h"
#include "../../../src/extension/favorite/folder-model.h"
#include "../../../src/libappdata/app-database-interface.h"
#include "../../../src/utils/security-function-control.h"
#undef private

using namespace UkuiMenu;

namespace {

QMap<int, FavoritesFolder> g_folders;
QStringList g_knownApps;
QMap<QString, DataEntity> g_appMap;
int g_renamedFolderId = -1;
QString g_renamedFolderName;
int g_exchangedFrom = -1;
int g_exchangedTo = -1;
int g_exchangedFolderId = -1;

DataEntity createApp(const QString &id)
{
    DataEntity entity;
    entity.setId(id);
    entity.setType(DataType::Normal);
    entity.setIcon(id + QStringLiteral("-icon"));
    entity.setName(id + QStringLiteral("-name"));
    entity.setComment(id + QStringLiteral("-comment"));
    entity.setExtraData(id + QStringLiteral("-extra"));
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
    Q_UNUSED(folder);
    return {};
}

bool FavoriteFolderHelper::getFolderFromId(const int &folderId, FavoritesFolder &folder)
{
    if (!g_folders.contains(folderId)) {
        return false;
    }
    folder = g_folders.value(folderId);
    return true;
}

bool FavoriteFolderHelper::containsFolder(const int &folderId)
{
    return g_folders.contains(folderId);
}

bool FavoriteFolderHelper::containApp(const QString &appId)
{
    Q_UNUSED(appId);
    return false;
}

bool FavoriteFolderHelper::deleteFolder(const int &folderId)
{
    Q_UNUSED(folderId);
    return false;
}

QList<FavoritesFolder> FavoriteFolderHelper::folderData()
{
    return g_folders.values();
}

void FavoriteFolderHelper::addAppToFolder(const QString &appId, const int &folderId)
{
    Q_UNUSED(appId);
    Q_UNUSED(folderId);
}

void FavoriteFolderHelper::addAppToNewFolder(const QString &appId, const QString &folderName)
{
    Q_UNUSED(appId);
    Q_UNUSED(folderName);
}

void FavoriteFolderHelper::addAppsToNewFolder(const QString &idFrom, const QString &idTo, const QString &folderName)
{
    Q_UNUSED(idFrom);
    Q_UNUSED(idTo);
    Q_UNUSED(folderName);
}

void FavoriteFolderHelper::removeAppFromFolder(const QString &appId)
{
    Q_UNUSED(appId);
}

QString FavoriteFolderHelper::getFolderName(const int &folderId)
{
    return g_folders.contains(folderId) ? g_folders.value(folderId).getName() : QString();
}

void FavoriteFolderHelper::renameFolder(const int &folderId, const QString &folderName)
{
    g_renamedFolderId = folderId;
    g_renamedFolderName = folderName;
}

void FavoriteFolderHelper::exchangedOrder(const int &indexFrom, const int &indexTo, const int &folderId)
{
    g_exchangedFrom = indexFrom;
    g_exchangedTo = indexTo;
    g_exchangedFolderId = folderId;
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
bool SecurityFunctionControl::canAppDisplay(const QString &desktop) const { Q_UNUSED(desktop); return true; }

}

#define UKUI_MENU_APP_FOLDER_HELPER_H
#define UKUI_MENU_APP_DATA_MANAGER_H
#define UKUI_MENU_CONTEXT_MENU_MANAGER_H
#define UKUI_MENU_BASIC_APP_MODEL_H
#define SECURITYFUNCTIONCONTROL_H
#include "../../../src/extension/favorite/folder-model.cpp"

class folderModel_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_setFolderId_loadsDataAndAccessors();
    void test_setFolderId_rejectsInvalidOrMissingFolder();
    void test_data_coversRolesAndMissingApps();
    void test_rename_and_exchange_forward();
    void test_folderSignals_reloadAndReset();
    void test_loadFolderData_reloadsCurrentFolder();
    void test_instance_returnsStableSingleton();
};

void folderModel_test::init()
{
    g_folders.clear();
    g_knownApps = QStringList{QStringLiteral("app.one"), QStringLiteral("app.two")};
    g_appMap.clear();
    g_appMap.insert(QStringLiteral("app.one"), createApp(QStringLiteral("app.one")));
    g_appMap.insert(QStringLiteral("app.two"), createApp(QStringLiteral("app.two")));
    g_renamedFolderId = -1;
    g_renamedFolderName.clear();
    g_exchangedFrom = -1;
    g_exchangedTo = -1;
    g_exchangedFolderId = -1;

    FavoritesFolder folder;
    folder.setId(5);
    folder.name = QStringLiteral("Tools");
    folder.apps = QStringList{QStringLiteral("app.one"), QStringLiteral("app.two")};
    g_folders.insert(5, folder);
}

void folderModel_test::test_setFolderId_loadsDataAndAccessors()
{
    FolderModel model;
    QSignalSpy countSpy(&model, &FolderModel::countChanged);

    model.setFolderId(QStringLiteral("5"));
    QCOMPARE(countSpy.count(), 1);
    QCOMPARE(model.count(), 2);
    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QCOMPARE(model.folderName(), QStringLiteral("Tools"));
    QCOMPARE(model.data(model.index(0, 0), DataEntity::Id).toString(), QStringLiteral("app.one"));
    QCOMPARE(model.data(model.index(0, 0), DataEntity::Name).toString(), QStringLiteral("app.one-name"));
    QCOMPARE(model.data(model.index(0, 0), DataEntity::DesktopName).toString(), QStringLiteral("app.one.desktop"));
    QVERIFY(!model.data(model.index(99, 0), DataEntity::Id).isValid());
    QCOMPARE(model.roleNames(), DataEntity::AppRoleNames());
}

void folderModel_test::test_setFolderId_rejectsInvalidOrMissingFolder()
{
    FolderModel model;
    QSignalSpy countSpy(&model, &FolderModel::countChanged);

    model.setFolderId(QStringLiteral("invalid"));
    QCOMPARE(model.count(), 0);
    QCOMPARE(countSpy.count(), 0);

    model.setFolderId(QStringLiteral("404"));
    QCOMPARE(model.count(), 0);
    QCOMPARE(countSpy.count(), 0);
    QCOMPARE(model.folderName(), QString());
}

void folderModel_test::test_data_coversRolesAndMissingApps()
{
    FolderModel model;
    model.setFolderId(QStringLiteral("5"));

    const QModelIndex first = model.index(0, 0);
    QCOMPARE(model.data(first, DataEntity::Id).toString(), QStringLiteral("app.one"));
    QCOMPARE(model.data(first, DataEntity::Type).toInt(), static_cast<int>(DataType::Normal));
    QCOMPARE(model.data(first, DataEntity::Icon).toString(), QStringLiteral("app.one-icon"));
    QCOMPARE(model.data(first, DataEntity::Name).toString(), QStringLiteral("app.one-name"));
    QCOMPARE(model.data(first, DataEntity::Comment).toString(), QStringLiteral("app.one-comment"));
    QCOMPARE(model.data(first, DataEntity::ExtraData).toString(), QStringLiteral("app.one-extra"));
    QCOMPARE(model.data(first, DataEntity::DesktopName).toString(), QStringLiteral("app.one.desktop"));
    QVERIFY(!model.data(first, Qt::UserRole - 1).isValid());

    g_appMap.remove(QStringLiteral("app.one"));
    QVERIFY(!model.data(first, DataEntity::Id).isValid());
    QVERIFY(!model.data(model.index(-1, 0), DataEntity::Id).isValid());
}

void folderModel_test::test_rename_and_exchange_forward()
{
    FolderModel model;
    model.setFolderId(QStringLiteral("5"));

    model.renameFolder(QStringLiteral("Renamed"));
    QCOMPARE(g_renamedFolderId, 5);
    QCOMPARE(g_renamedFolderName, QStringLiteral("Renamed"));

    model.exchangedOrder(0, 1);
    QCOMPARE(g_exchangedFrom, 0);
    QCOMPARE(g_exchangedTo, 1);
    QCOMPARE(g_exchangedFolderId, 5);
}

void folderModel_test::test_folderSignals_reloadAndReset()
{
    FolderModel model;
    model.setFolderId(QStringLiteral("5"));

    g_folders[5].apps = QStringList{QStringLiteral("app.two")};
    Q_EMIT FavoriteFolderHelper::instance()->folderAppsChanged(5);
    QCOMPARE(model.count(), 1);
    QCOMPARE(model.data(model.index(0, 0), DataEntity::Id).toString(), QStringLiteral("app.two"));

    Q_EMIT FavoriteFolderHelper::instance()->folderToBeDeleted(5, QStringList{QStringLiteral("app.two")});
    QCOMPARE(model.count(), 0);

    model.setFolderId(QStringLiteral("invalid"));
    QCOMPARE(model.count(), 0);
}

void folderModel_test::test_loadFolderData_reloadsCurrentFolder()
{
    FolderModel model;
    model.setFolderId(QStringLiteral("5"));

    g_folders[5].apps = QStringList{QStringLiteral("app.two")};
    model.loadFolderData(5);
    QCOMPARE(model.count(), 1);
    QCOMPARE(model.data(model.index(0, 0), DataEntity::Id).toString(), QStringLiteral("app.two"));

    g_folders[5].apps = QStringList{QStringLiteral("app.one"), QStringLiteral("app.two")};
    model.loadFolderData(5);
    QCOMPARE(model.count(), 2);

    g_folders[5].apps = QStringList{QStringLiteral("app.one")};
    model.loadFolderData(5);
    QCOMPARE(model.count(), 1);

    model.loadFolderData(9);
    QCOMPARE(model.count(), 1);
}

void folderModel_test::test_instance_returnsStableSingleton()
{
    FolderModel &first = FolderModel::instance();
    FolderModel &second = FolderModel::instance();

    QCOMPARE(&first, &second);
}

QTEST_MAIN(folderModel_test)

#include "folder-model-test.moc"
