/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#include "favorites-config.h"
#include "app-favorite-model.h"
#include "favorite-folder-helper.h"
#include "../context-menu-manager.h"

#include <QMimeDatabase>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>

namespace UkuiMenu {
class Q_DECL_HIDDEN AppFavoritesModel::Private
{
public:
    explicit Private(AppFavoritesModel *q = nullptr);
    ~Private();
    void init();
    QPersistentModelIndex getIndexFromAppId(const QString &id) const;

    void addFavoriteApp(const QPersistentModelIndex &modelIndex, const int &index = 0);
    void removeFavoriteApp(const QString &appId);
    void onAppRemoved(const QModelIndex &parent, int first, int last);
    void onAppUpdated(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    void updateFavoritesApps(const QModelIndex &sourceIndex);

    void onFolderAdded(const int &folderId, int order);
    void onFolderDeleted(const int &folderId, const QStringList &apps);
    void onFolderAppChanged(const int &folderId, const QString &appId, bool isAdded);

    QVector<QPersistentModelIndex> m_favoritesApps; // 已收藏应用在baseModel的对应index
    QVector<int> m_folders;  // 应用组的唯一Id
    QVector<QString> m_favoritesFiles; // 收藏文件夹的唯一路径
    BasicAppModel *m_sourceModel = nullptr;
    QStringList m_items;

private:
    AppFavoritesModel *q = nullptr;
};

AppFavoritesModel::Private::Private(AppFavoritesModel* q) : q(q)
{
}

AppFavoritesModel::Private::~Private()
{

}

void AppFavoritesModel::Private::init()
{
    m_items = FavoritesConfig::instance().getConfig();
    QSet<QString> existingItems(m_items.begin(), m_items.end());
    QStringList favoritesNeedsAdd;
    m_favoritesApps.clear();

    for (int i = 0; i < m_sourceModel->rowCount(QModelIndex()); i++) {
        const auto index = m_sourceModel->index(i, 0);
        const QString id = index.data(DataEntity::Id).toString();
        const QString fullId = APP_ID_SCHEME + id;
        const int favorite = index.data(DataEntity::Favorite).toInt();

        if (favorite > 0) {
            QPersistentModelIndex persistentIndex(index);
            m_favoritesApps.append(persistentIndex);

            if (!existingItems.contains(fullId) && !FavoriteFolderHelper::instance()->containApp(id)) {
                favoritesNeedsAdd.append(fullId);
                existingItems.insert(fullId);
            }
            // 处理配置中有，但数据库为未收藏的情况
        } else if (existingItems.contains(fullId)) {
            QPersistentModelIndex persistentIndex(index);
            m_favoritesApps.append(persistentIndex);
            m_sourceModel->databaseInterface()->fixAppToFavorite(id, 1);
        }
    }

    // 如果有新增收藏项，更新配置
    if (!favoritesNeedsAdd.isEmpty()) {
        m_items.append(favoritesNeedsAdd);
        FavoritesConfig::instance().sync(m_items);
    }

    QVector<int> foldersId;
    for (const auto &folder : FavoriteFolderHelper::instance()->folderData()) {
        foldersId.append(folder.getId());

        if (!m_items.contains(FOLDER_ID_SCHEME + QString::number(folder.getId()))) {
            m_items.insert(m_items.count(), FOLDER_ID_SCHEME + QString::number(folder.getId()));
            FavoritesConfig::instance().sync(m_items);
        }
    }
    m_folders.swap(foldersId);
}

void AppFavoritesModel::Private::onAppUpdated(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    for (int row = topLeft.row(); row <= bottomRight.row(); row++) {
        auto index = m_sourceModel->index(row, 0);
        if (roles.isEmpty() || roles.contains(DataEntity::Favorite)) {
            updateFavoritesApps(index);
        }

        if (roles.isEmpty() || !roles.contains(DataEntity::Favorite)) {
            int favoriteIndex = m_items.indexOf(APP_ID_SCHEME + index.data(DataEntity::Id).toString());
            if (favoriteIndex < 0) {
                continue;
            }

            Q_EMIT q->dataChanged(q->index(favoriteIndex, 0, QModelIndex()), q->index(favoriteIndex, 0, QModelIndex()), roles);
        }
    }
}

void AppFavoritesModel::Private::updateFavoritesApps(const QModelIndex &sourceIndex)
{
    auto id = sourceIndex.data(DataEntity::Id).toString();
    auto favorite = sourceIndex.data(DataEntity::Favorite).toInt();

    if (id.isEmpty()) {
        return;
    }
    QPersistentModelIndex index(sourceIndex);

    if (favorite > 0 && !m_favoritesApps.contains(index)) {
        addFavoriteApp(index, m_items.count());

    } else if (favorite == 0) {
        if (FavoriteFolderHelper::instance()->containApp(id)) {
            FavoriteFolderHelper::instance()->removeAppFromFolder(id);
        }
        m_favoritesApps.removeOne(index);
        removeFavoriteApp(index.data(DataEntity::Id).toString());
    }
}

void AppFavoritesModel::Private::onAppRemoved(const QModelIndex &parent, int first, int last)
{
    for (int row = first; row <= last; ++row) {
        QModelIndex index = m_sourceModel->index(row, 0, {});

        if (index.data(DataEntity::Favorite).toInt() > 0) {
            QString appId = index.data(DataEntity::Id).toString();

            if (FavoriteFolderHelper::instance()->containApp(appId)) {
                FavoriteFolderHelper::instance()->removeAppFromFolder(appId);
            }

            QPersistentModelIndex modelIndex(index);
            m_favoritesApps.removeOne(modelIndex);
            removeFavoriteApp(index.data(DataEntity::Id).toString());
        }
    }
}

void AppFavoritesModel::Private::addFavoriteApp(const QPersistentModelIndex &modelIndex, const int &index)
{
    if (!m_favoritesApps.contains(modelIndex) && modelIndex.isValid()) {
        m_favoritesApps.append(modelIndex);
    }

    auto id = APP_ID_SCHEME + modelIndex.data(DataEntity::Id).toString();
    if (!m_items.contains(id)) {
        q->beginInsertRows(QModelIndex(), index, index);
        m_items.insert(index, id);
        q->endInsertRows();
        FavoritesConfig::instance().sync(m_items);
    }
}

void AppFavoritesModel::Private::removeFavoriteApp(const QString &appId)
{
    int index = m_items.indexOf(APP_ID_SCHEME + appId);
    if (index > -1 && index < m_items.count()) {
        q->beginRemoveRows(QModelIndex(), index, index);
        m_items.takeAt(index);
        q->endRemoveRows();
        FavoritesConfig::instance().sync(m_items);
    }
}

void AppFavoritesModel::Private::onFolderAdded(const int &folderId, int order)
{
    if (!m_folders.contains(folderId)) {
        m_folders.append(folderId);

        FavoritesFolder folder;
        FavoriteFolderHelper::instance()->getFolderFromId(folderId, folder);
        for (auto app : folder.getApps()) {
            removeFavoriteApp(app);
        }

        order = std::max(0, order);
        q->beginInsertRows(QModelIndex(), order, order);
        m_items.insert(order, FOLDER_ID_SCHEME + QString::number(folderId));
        q->endInsertRows();
        FavoritesConfig::instance().sync(m_items);
    } else {
        qWarning() << "Favorites Add New Folder Error: " << folderId << "Already Have!";
    }
}

void AppFavoritesModel::Private::onFolderDeleted(const int &folderId, const QStringList &apps)
{
    if (m_folders.contains(folderId)) {
        m_folders.removeOne(folderId);

        int index = m_items.indexOf(FOLDER_ID_SCHEME + QString::number(folderId));

        q->beginRemoveRows(QModelIndex(), index, index);
        m_items.takeAt(index);
        q->endRemoveRows();
        FavoritesConfig::instance().sync(m_items);

        for (int i = 0; i < apps.count(); i++) {
            QPersistentModelIndex modelIndex(m_sourceModel->index(m_sourceModel->indexOfApp(apps.at(i)), 0));
            addFavoriteApp(modelIndex, index + i);
        }
    } else {
        qWarning() << "Favorites Remove Folder Error: " << folderId << "Does Not Exist!";
    }
}

void AppFavoritesModel::Private::onFolderAppChanged(const int &folderId, const QString &appId, bool isAdded)
{
    // 添加、移除应用到应用组
    if (isAdded) {
        removeFavoriteApp(appId);
    } else {
        QPersistentModelIndex modelIndex(m_sourceModel->index(m_sourceModel->indexOfApp(appId), 0));
        addFavoriteApp(modelIndex, m_items.indexOf(FOLDER_ID_SCHEME + QString::number(folderId)) + 1);
    }

    int row = m_items.indexOf(FOLDER_ID_SCHEME + QString::number(folderId));
    Q_EMIT q->dataChanged(q->index(row), q->index(row), QVector<int>{DataEntity::Icon});
}

QPersistentModelIndex AppFavoritesModel::Private::getIndexFromAppId(const QString &id) const
{
    auto modelIndex = std::find_if(m_favoritesApps.constBegin(), m_favoritesApps.constEnd(), [&id] (const QPersistentModelIndex &index) {
        return index.data(DataEntity::Id).toString() == id;
    });

    if (modelIndex == m_favoritesApps.constEnd()) {
        return {};
    }

    return *modelIndex;
}

AppFavoritesModel &AppFavoritesModel::instance()
{
    static AppFavoritesModel appFavoritesModel;
    return appFavoritesModel;
}

AppFavoritesModel::AppFavoritesModel(QObject *parent) : QAbstractListModel(parent), d(new Private(this))
{
    d->m_sourceModel = BasicAppModel::instance();
    d->init();

    connect(BasicAppModel::instance(), &BasicAppModel::dataChanged, this, [&](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
        d->onAppUpdated(topLeft, bottomRight, roles);
    });
    connect(BasicAppModel::instance(), &BasicAppModel::rowsAboutToBeRemoved, this, [&](const QModelIndex&parent, int first, int last) {
        d->onAppRemoved(parent, first, last);
    });
    connect(BasicAppModel::instance(), &BasicAppModel::rowsInserted, this, [&](const QModelIndex&parent, int first, int last) {
        for (int row = first; row <= last; ++row) {
            QModelIndex index = d->m_sourceModel->index(row, 0, {});
            if (index.data(DataEntity::Favorite).toInt() > 0) {
                d->addFavoriteApp(QPersistentModelIndex(index), rowCount());
            }
        }
    });
    connect(FavoriteFolderHelper::instance(), &FavoriteFolderHelper::folderAdded, this, [&] (int folderId, int order) {
        d->onFolderAdded(folderId, order);
    });
    connect(FavoriteFolderHelper::instance(), &FavoriteFolderHelper::folderToBeDeleted, this, [&](int folderId, const QStringList& apps) {
        d->onFolderDeleted(folderId, apps);
    });
    connect(FavoriteFolderHelper::instance(), &FavoriteFolderHelper::folderAppChanged, this, [&] (int folderId, const QString& app, bool isAdded) {
        d->onFolderAppChanged(folderId, app, isAdded);
    });
    connect(FavoriteFolderHelper::instance(), &FavoriteFolderHelper::folderAppsChanged, this, [&] (int folderId) {
        int row = d->m_items.indexOf(FOLDER_ID_SCHEME + QString::number(folderId));
        Q_EMIT dataChanged(index(row), index(row), QVector<int>{DataEntity::Icon});
    });
    connect(FavoriteFolderHelper::instance(), &FavoriteFolderHelper::folderNameChanged, this, [&] (int folderId) {
        int row = d->m_items.indexOf(FOLDER_ID_SCHEME + QString::number(folderId));
        Q_EMIT dataChanged(index(row), index(row), QVector<int>{DataEntity::Name});
    });
}

AppFavoritesModel::~AppFavoritesModel()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

QHash<int, QByteArray> AppFavoritesModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names.insert(DataEntity::Id, "id");
    names.insert(DataEntity::Icon, "icon");
    names.insert(DataEntity::Name, "name");
    names.insert(DataEntity::Type, "type");
    names.insert(DataEntity::DesktopName, "desktopName");
    return names;
}

int AppFavoritesModel::rowCount(const QModelIndex &parent) const
{
    return d->m_items.count();
}

QVariant AppFavoritesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return {};
    }
    QString id = d->m_items.at(index.row());

    // 检查并提取不同前缀后的ID
    if (id.startsWith(APP_ID_SCHEME)) {
        auto appModelIndex = d->getIndexFromAppId(id.mid(6));
        return appModelIndex.data(role);

    } else if (id.startsWith(FOLDER_ID_SCHEME)) {
        return folderDataFromId(id.mid(9), role);

    } else if (id.startsWith(FILE_ID_SCHEME)) {
        return fileDataFromUrl(id.mid(7), role);
    } else {
        return {};
    }
}

QVariant AppFavoritesModel::folderDataFromId(const QString &folderId, int role) const
{
    FavoritesFolder folder;
    if (!FavoriteFolderHelper::instance()->getFolderFromId(folderId.toInt(), folder)) {
        return {};
    }

    switch (role) {
        case DataEntity::Id:
            return QString::number(folder.getId());
        case DataEntity::Icon:
            return FavoriteFolderHelper::folderIcon(folder);
        case DataEntity::Name:
            return folder.getName();
        case DataEntity::Type:
            return DataType::Folder;
        default:
            break;
    }

    return {};
}

QVariant AppFavoritesModel::fileDataFromUrl(const QString &url, int role) const
{
    switch (role) {
        case DataEntity::Id:
            return url;
        case DataEntity::Icon: {
            QMimeDatabase mimeDatabase;
            return mimeDatabase.mimeTypeForFile(url).iconName();
        }
        case DataEntity::Name:
            return QUrl(url).fileName();
        case DataEntity::Type:
            return DataType::Files;
        default:
            break;
    }

    return {};
}

void AppFavoritesModel::changeFileState(const QString &url, const bool &favorite)
{
    if (url.isEmpty()) {
        return;
    }
    QString fileId;
    if (url.startsWith(FILE_ID_SCHEME)) {
        fileId = url;
    } else {
        fileId = FILE_ID_SCHEME + url;
    }

    if (favorite) {
        d->m_favoritesFiles.append(fileId);
        d->m_items.append(fileId);
        FavoritesConfig::instance().sync(d->m_items);
    } else {
        d->m_favoritesFiles.removeAll(fileId);
        d->m_items.removeAll(fileId);
        FavoritesConfig::instance().sync(d->m_items);
    }
}

int AppFavoritesModel::getOrderById(const QString& id)
{
    return d->m_items.indexOf(id);
}

void AppFavoritesModel::openMenu(const int& row)
{
    if (row < 0 || row >= rowCount()) {
        return;
    }

    if (data(index(row, 0), DataEntity::Type).value<DataType::Type>() == UkuiMenu::DataType::Normal) {
        ContextMenuManager::instance()->showMenu(data(index(row, 0), DataEntity::Entity).value<DataEntity>(), MenuInfo::Extension, "favorite");
    } else {
        DataEntity appData;
        appData.setId(data(index(row, 0), DataEntity::Id).toString());
        appData.setFavorite(data(index(row, 0), DataEntity::Favorite).toInt());
        appData.setType(data(index(row, 0), DataEntity::Type).value<DataType::Type>());

        ContextMenuManager::instance()->showMenu(appData, MenuInfo::Extension, "favorite");
    }
}

void AppFavoritesModel::addAppToFavorites(const QString &id, int index)
{
    if (d->getIndexFromAppId(id).isValid() || FavoriteFolderHelper::instance()->containApp(id)) {
        qWarning() << "This application is already included in the favorite apps!";
        return;
    }

    if (index == -1) {
        index = rowCount();
    }

    QPersistentModelIndex modelIndex(d->m_sourceModel->index(d->m_sourceModel->indexOfApp(id), 0));
    d->addFavoriteApp(modelIndex, index);
    d->m_sourceModel->databaseInterface()->fixAppToFavorite(id, 1);
    BasicAppModel::instance()->databaseInterface()->updateApLaunchedState(id, true);
}

void AppFavoritesModel::removeAppFromFavorites(const QString &id)
{
    if (id.isEmpty()) {
        return;
    }

    d->removeFavoriteApp(id);
    d->m_sourceModel->databaseInterface()->fixAppToFavorite(id, 0);
}

void AppFavoritesModel::exchangedAppsOrder(int indexFrom, int indexTo)
{
    indexFrom = qMin(indexFrom, d->m_items.size() -1);
    indexTo = qMin(indexTo, d->m_items.size() -1);
    if (indexFrom == indexTo || indexFrom < 0 || indexTo < 0) {
        return;
    }

    if (indexFrom < indexTo) {
        beginMoveRows(QModelIndex(), indexFrom, indexFrom, QModelIndex(), indexTo + 1);
    } else {
        beginMoveRows(QModelIndex(), indexFrom, indexFrom, QModelIndex(), indexTo);
    }
    d->m_items.move(indexFrom, indexTo);
    endMoveRows();
    FavoritesConfig::instance().sync(d->m_items);
}

void AppFavoritesModel::addAppsToNewFolder(const QString& idFrom, const QString& idTo)
{
    if (idFrom == idTo) {
        return;
    }

    if (d->getIndexFromAppId(idFrom).isValid() && d->getIndexFromAppId(idTo).isValid()) {
        FavoriteFolderHelper::instance()->addAppsToNewFolder(idFrom, idTo, "");
    }
}

void AppFavoritesModel::addAppToFolder(const QString& appId, const QString& folderId)
{
    if (folderId == "") {
        FavoriteFolderHelper::instance()->addAppToNewFolder(appId, "");
        return;
    }

    FavoriteFolderHelper::instance()->addAppToFolder(appId, folderId.toInt());
}

void AppFavoritesModel::clearFavorites()
{
    // 先处理所有应用组中的应用，从应用组中移除
    QSet<QString> allFolderApps;
    for (const auto &folder : FavoriteFolderHelper::instance()->folderData()) {
        for (const auto &appId : folder.getApps()) {
            allFolderApps.insert(appId);
        }
    }
    for (const auto &appId : allFolderApps) {
        FavoriteFolderHelper::instance()->removeAppFromFolder(appId);
    }

    // 再处理独立应用（此时包含从应用组移出的应用）
    auto favApps = d->m_favoritesApps;
    for (const auto &appIndex : favApps) {
        QString id = appIndex.data(DataEntity::Id).toString();
        removeAppFromFavorites(id);
    }
    d->m_favoritesFiles.clear();
}

QString AppFavoritesModel::getDesktopFile(const QString& dragSourceUrl)
{
    QUrl url = QUrl(dragSourceUrl);
    return url.isValid() ? url.path() : "";
}
} // UkuiMenu
#include "moc_app-favorite-model.cpp"
