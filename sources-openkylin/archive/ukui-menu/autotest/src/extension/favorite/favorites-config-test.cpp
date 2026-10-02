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
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include "../../../src/data-entity.h"
#define private public
#include "../../../src/extension/favorite/favorites-config.h"
#include "../../../src/extension/favorite/favorite-folder-helper.h"
#include "../../../src/libappdata/app-database-interface.h"
#undef private

using namespace UkuiMenu;

namespace {

QSet<int> g_knownFolders;
QSet<QString> g_knownFolderApps;
QStringList g_knownApps;
QTemporaryDir *g_tempDir = nullptr;

QJsonObject versionedObject(const QStringList &favorites)
{
    QJsonObject root;
    root.insert(QStringLiteral("version"), QStringLiteral("1.0.0"));
    root.insert(QStringLiteral("favorites"), QJsonArray::fromStringList(favorites));
    return root;
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
        Q_UNUSED(appid);
        Q_UNUSED(app);
        return false;
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
    Q_UNUSED(folder);
    return g_knownFolders.contains(folderId);
}

bool FavoriteFolderHelper::containsFolder(const int &folderId)
{
    return g_knownFolders.contains(folderId);
}

bool FavoriteFolderHelper::containApp(const QString &appId)
{
    return g_knownFolderApps.contains(appId);
}

bool FavoriteFolderHelper::deleteFolder(const int &folderId)
{
    Q_UNUSED(folderId);
    return false;
}

QList<FavoritesFolder> FavoriteFolderHelper::folderData()
{
    return {};
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
    Q_UNUSED(folderId);
    return {};
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

#define UKUI_MENU_BASIC_APP_MODEL_H
#define UKUI_MENU_FAVORITE_FOLDER_HELPER_H
#include "../../../src/extension/favorite/favorites-config.cpp"

class favoritesConfig_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void test_sync_and_getConfig();
    void test_sync_ignoresUnchangedList();
    void test_sync_handlesUncreatableConfigDirectory();
    void test_initConfig_readsVersionedObject();
    void test_initConfig_filtersInvalidEntriesFromObject();
    void test_initConfig_repairsLegacyArrayFormat();
    void test_initConfig_ignoresEmptyInvalidAndUnsupportedFiles();
    void test_initConfig_objectWithoutExpectedVersionLeavesConfigEmpty();
    void test_updateInvalidUrlList();
};

void favoritesConfig_test::init()
{
    delete g_tempDir;
    g_tempDir = new QTemporaryDir;
    QVERIFY(g_tempDir->isValid());

    QDir homeDir(QDir::homePath());
    QVERIFY(homeDir.mkpath(QStringLiteral(".config/ukui-menu")));

    FavoritesConfig::s_favoritesConfigFile = g_tempDir->path() + QStringLiteral("/favorite.json");
    g_knownFolders = QSet<int>{7};
    g_knownFolderApps = QSet<QString>{QStringLiteral("folder.app")};
    g_knownApps = QStringList{QStringLiteral("app.one"), QStringLiteral("app.two"), QStringLiteral("folder.app")};
}

void favoritesConfig_test::cleanup()
{
    delete g_tempDir;
    g_tempDir = nullptr;
}

void favoritesConfig_test::test_sync_and_getConfig()
{
    FavoritesConfig config;
    const QStringList list{QStringLiteral("app://app.one"), QStringLiteral("folder://7")};

    config.sync(list);
    QCOMPARE(config.getConfig(), list);

    QFile file(FavoritesConfig::s_favoritesConfigFile);
    QVERIFY(file.open(QFile::ReadOnly));
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QVERIFY(doc.isObject());
    QCOMPARE(doc.object().value(QStringLiteral("version")).toString(), QStringLiteral("1.0.0"));
}

void favoritesConfig_test::test_sync_ignoresUnchangedList()
{
    FavoritesConfig config;
    const QStringList list{QStringLiteral("app://app.one")};

    config.sync(list);
    QFile file(FavoritesConfig::s_favoritesConfigFile);
    QVERIFY(file.open(QFile::ReadOnly));
    const QByteArray original = file.readAll();
    file.close();

    QVERIFY(file.open(QFile::WriteOnly | QFile::Truncate));
    file.write("sentinel");
    file.close();

    config.sync(list);
    QVERIFY(file.open(QFile::ReadOnly));
    QCOMPARE(file.readAll(), QByteArray("sentinel"));

    QVERIFY(!original.isEmpty());
}

void favoritesConfig_test::test_sync_handlesUncreatableConfigDirectory()
{
    FavoritesConfig config;
    const QString previousHome = QDir::homePath();
    qputenv("HOME", g_tempDir->path().toUtf8() + "/not-a-directory");
    QFile blocker(QDir::homePath());
    QVERIFY(blocker.open(QFile::WriteOnly));
    blocker.close();

    config.sync(QStringList{QStringLiteral("app://app.one")});
    QCOMPARE(config.getConfig(), QStringList({QStringLiteral("app://app.one")}));
    QVERIFY(!QFile::exists(QDir::homePath() + QStringLiteral("/.config/ukui-menu/favorite.json")));

    qputenv("HOME", previousHome.toUtf8());
}

void favoritesConfig_test::test_initConfig_readsVersionedObject()
{
    QFile file(FavoritesConfig::s_favoritesConfigFile);
    QVERIFY(file.open(QFile::WriteOnly));
    file.write(QJsonDocument(versionedObject({QStringLiteral("app://app.one"), QStringLiteral("folder://7")})).toJson());
    file.close();

    FavoritesConfig config;
    QCOMPARE(config.getConfig(), QStringList({QStringLiteral("app://app.one"), QStringLiteral("folder://7")}));
}

void favoritesConfig_test::test_initConfig_filtersInvalidEntriesFromObject()
{
    QFile file(FavoritesConfig::s_favoritesConfigFile);
    QVERIFY(file.open(QFile::WriteOnly));
    file.write(QJsonDocument(versionedObject({
        QStringLiteral("app://app.one"),
        QStringLiteral("app://missing.app"),
        QStringLiteral("app://folder.app"),
        QStringLiteral("folder://9")
    })).toJson());
    file.close();

    FavoritesConfig config;
    QCOMPARE(config.getConfig(), QStringList({QStringLiteral("app://app.one")}));
}

void favoritesConfig_test::test_initConfig_repairsLegacyArrayFormat()
{
    QFile file(FavoritesConfig::s_favoritesConfigFile);
    QVERIFY(file.open(QFile::WriteOnly));
    file.write(QJsonDocument(QJsonArray{
        QStringLiteral("app://app.one"),
        QStringLiteral("app://missing.app"),
        QStringLiteral("folder://7")
    }).toJson());
    file.close();

    FavoritesConfig config;
    QCOMPARE(config.getConfig(), QStringList({QStringLiteral("app://app.one"), QStringLiteral("folder://7")}));
}

void favoritesConfig_test::test_initConfig_ignoresEmptyInvalidAndUnsupportedFiles()
{
    {
        QFile file(FavoritesConfig::s_favoritesConfigFile);
        QVERIFY(file.open(QFile::WriteOnly));
        file.close();

        FavoritesConfig config;
        QCOMPARE(config.getConfig(), QStringList());
    }

    {
        QFile file(FavoritesConfig::s_favoritesConfigFile);
        QVERIFY(file.open(QFile::WriteOnly | QFile::Truncate));
        file.write("{not-json");
        file.close();

        FavoritesConfig config;
        QCOMPARE(config.getConfig(), QStringList());
    }

}

void favoritesConfig_test::test_initConfig_objectWithoutExpectedVersionLeavesConfigEmpty()
{
    QFile file(FavoritesConfig::s_favoritesConfigFile);
    QVERIFY(file.open(QFile::WriteOnly));
    QJsonObject root;
    root.insert(QStringLiteral("version"), QStringLiteral("0.9.0"));
    root.insert(QStringLiteral("favorites"), QJsonArray::fromStringList({QStringLiteral("app://app.one")}));
    file.write(QJsonDocument(root).toJson());
    file.close();

    FavoritesConfig config;
    QCOMPARE(config.getConfig(), QStringList());
}

void favoritesConfig_test::test_updateInvalidUrlList()
{
    FavoritesConfig config;
    QStringList list{
        QStringLiteral("app://app.one"),
        QStringLiteral("app://missing.app"),
        QStringLiteral("app://folder.app"),
        QStringLiteral("folder://7"),
        QStringLiteral("folder://abc")
    };

    const QStringList invalid = config.updateInvalidUrlList(list);
    QVERIFY(invalid.contains(QStringLiteral("app://missing.app")));
    QVERIFY(invalid.contains(QStringLiteral("app://folder.app")));
    QVERIFY(invalid.contains(QStringLiteral("folder://abc")));
    QVERIFY(!invalid.contains(QStringLiteral("app://app.one")));
    QVERIFY(!invalid.contains(QStringLiteral("folder://7")));
}

QTEST_MAIN(favoritesConfig_test)

#include "favorites-config-test.moc"
