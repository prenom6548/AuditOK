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
#include "../../../src/extension/context-menu-extension.h"
#define protected public
#define private public
#include "../../../src/extension/favorite/favorite-folder-helper.h"
#include "../../../src/extension/favorite/favorite-filter-model.h"
#include "../../../src/utils/security-function-control.h"
#undef private
#undef protected

using namespace UkuiMenu;

namespace {

QList<FavoritesFolder> g_folders;
QStringList g_visibleApps;
int g_openMenuRow = -1;
QString g_addedFavoriteId;
int g_addedFavoriteIndex = -2;
QString g_removedFavoriteId;
int g_exchangeFrom = -1;
int g_exchangeTo = -1;
QString g_newFolderFrom;
QString g_newFolderTo;
QString g_folderAppId;
QString g_folderIdText;
bool g_clearFavoritesCalled = false;
QString g_desktopFileResult = QStringLiteral("/tmp/fake.desktop");

DataEntity createAppEntity(const QString &id, DataType::Type type = DataType::Normal)
{
    DataEntity entity;
    entity.setId(id);
    entity.setType(type);
    entity.setName(id + QStringLiteral("-name"));
    entity.setIcon(id + QStringLiteral("-icon"));
    entity.setDesktopName(id + QStringLiteral(".desktop"));
    return entity;
}

}

namespace UkuiMenu {

class AppFavoritesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    static AppFavoritesModel &instance()
    {
        static AppFavoritesModel model;
        return model;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return m_items.size();
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
            return {};
        }

        return m_items.at(index.row()).getValue(static_cast<DataEntity::PropertyName>(role));
    }

    void setItems(const QVector<DataEntity> &items)
    {
        beginResetModel();
        m_items = items;
        endResetModel();
    }

    void openMenu(const int &row)
    {
        g_openMenuRow = row;
    }

    void addAppToFavorites(const QString &id, int index = -1)
    {
        g_addedFavoriteId = id;
        g_addedFavoriteIndex = index;
    }

    void removeAppFromFavorites(const QString &id)
    {
        g_removedFavoriteId = id;
    }

    void exchangedAppsOrder(int indexFrom, int indexTo)
    {
        g_exchangeFrom = indexFrom;
        g_exchangeTo = indexTo;
    }

    void addAppsToNewFolder(const QString &idFrom, const QString &idTo)
    {
        g_newFolderFrom = idFrom;
        g_newFolderTo = idTo;
    }

    void addAppToFolder(const QString &appId, const QString &folderId)
    {
        g_folderAppId = appId;
        g_folderIdText = folderId;
    }

    void clearFavorites()
    {
        g_clearFavoritesCalled = true;
    }

    QString getDesktopFile(const QString &dragSourceUrl)
    {
        Q_UNUSED(dragSourceUrl);
        return g_desktopFileResult;
    }

    int getOrderById(const QString &id)
    {
        for (int i = 0; i < m_items.size(); ++i) {
            if (m_items.at(i).id() == id || (QStringLiteral("app://") + m_items.at(i).id()) == id) {
                return i;
            }
        }
        return -1;
    }

private:
    QVector<DataEntity> m_items;
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
    g_removedFavoriteId = appId;
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
    QStringList result;
    for (const QString &app : apps) {
        if (g_visibleApps.contains(app)) {
            result << app;
        }
    }
    return result;
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
#define UKUI_MENU_FAVORITE_FOLDER_HELPER_H
#define SECURITYFUNCTIONCONTROL_H
#include "../../../src/extension/favorite/favorite-filter-model.cpp"

class favoriteFilterModel_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_basicForwardersAndMapping();
    void test_addAppFromFolder_reordersWhenNeeded();
    void test_filterAcceptsRow_forFoldersAndVisibleApps();
    void test_onSecurityControlChanged_invalidatesAndEmits();
};

void favoriteFilterModel_test::init()
{
    g_folders.clear();
    g_visibleApps = QStringList{QStringLiteral("app.one"), QStringLiteral("app.two")};
    g_openMenuRow = -1;
    g_addedFavoriteId.clear();
    g_addedFavoriteIndex = -2;
    g_removedFavoriteId.clear();
    g_exchangeFrom = -1;
    g_exchangeTo = -1;
    g_newFolderFrom.clear();
    g_newFolderTo.clear();
    g_folderAppId.clear();
    g_folderIdText.clear();
    g_clearFavoritesCalled = false;
    g_desktopFileResult = QStringLiteral("/tmp/fake.desktop");

    FavoritesFolder folder;
    folder.setId(11);
    folder.name = QStringLiteral("Folder");
    folder.apps = QStringList{QStringLiteral("app.one"), QStringLiteral("app.two")};
    g_folders << folder;

    AppFavoritesModel::instance().setItems({
        createAppEntity(QStringLiteral("11"), DataType::Folder),
        createAppEntity(QStringLiteral("app.one")),
        createAppEntity(QStringLiteral("app.hidden"))
    });
}

void favoriteFilterModel_test::test_basicForwardersAndMapping()
{
    FavoriteFilterModel model;

    QVERIFY(model.rowCount() >= 2);
    QCOMPARE(model.data(model.index(0, 0), DataEntity::Id).toString(), QStringLiteral("11"));

    model.openMenu(1);
    QCOMPARE(g_openMenuRow, 1);

    model.addAppToFavorites(QStringLiteral("new.app"), 1);
    QCOMPARE(g_addedFavoriteId, QStringLiteral("new.app"));
    QCOMPARE(g_addedFavoriteIndex, 1);

    model.removeAppFromFavorites(QStringLiteral("app.one"));
    QCOMPARE(g_removedFavoriteId, QStringLiteral("app.one"));

    model.exchangedAppsOrder(0, 1);
    QCOMPARE(g_exchangeFrom, 0);
    QCOMPARE(g_exchangeTo, 1);

    model.addAppsToNewFolder(QStringLiteral("app.one"), QStringLiteral("app.two"));
    QCOMPARE(g_newFolderFrom, QStringLiteral("app.one"));
    QCOMPARE(g_newFolderTo, QStringLiteral("app.two"));

    model.addAppToFolder(QStringLiteral("app.one"), QStringLiteral("11"));
    QCOMPARE(g_folderAppId, QStringLiteral("app.one"));
    QCOMPARE(g_folderIdText, QStringLiteral("11"));

    model.clearFavorites();
    QCOMPARE(g_clearFavoritesCalled, true);

    QCOMPARE(model.getDesktopFile(QStringLiteral("ignored")), QStringLiteral("/tmp/fake.desktop"));
}

void favoriteFilterModel_test::test_addAppFromFolder_reordersWhenNeeded()
{
    FavoriteFilterModel model;

    model.addAppFromFolder(QStringLiteral("app.one"), 0);
    QCOMPARE(g_removedFavoriteId, QStringLiteral("app.one"));
    QCOMPARE(g_exchangeFrom, 1);
    QCOMPARE(g_exchangeTo, 0);
}

void favoriteFilterModel_test::test_filterAcceptsRow_forFoldersAndVisibleApps()
{
    FavoriteFilterModel model;

    QVERIFY(model.filterAcceptsRow(0, QModelIndex()));
    QVERIFY(model.filterAcceptsRow(1, QModelIndex()));
    QVERIFY(!model.filterAcceptsRow(2, QModelIndex()));

    g_visibleApps.clear();
    QVERIFY(!model.filterAcceptsRow(0, QModelIndex()));
}

void favoriteFilterModel_test::test_onSecurityControlChanged_invalidatesAndEmits()
{
    FavoriteFilterModel model;
    QSignalSpy spy(&model, &FavoriteFilterModel::dataChanged);

    model.onSecurityControlChanged();

    QVERIFY(spy.count() >= 1);
}

QTEST_MAIN(favoriteFilterModel_test)

#include "favorite-filter-model-test.moc"
