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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

#include "favorite-filter-model.h"
#include "app-favorite-model.h"
#include "favorites-config.h"
#include "security-function-control.h"
#include "favorite-folder-helper.h"

#include <QDebug>

namespace UkuiMenu {

FavoriteFilterModel *FavoriteFilterModel::instance()
{
    static FavoriteFilterModel model;
    return &model;
}

QVariant FavoriteFilterModel::data(const QModelIndex &index, int role) const
{
    return sourceModel()->data(mapToSource(index), role);
}

void FavoriteFilterModel::openMenu(const int &row)
{
    m_sourceModel->openMenu(mapToSource(this->index(row, 0)).row());
}

void FavoriteFilterModel::addAppToFavorites(const QString &id, int index)
{
    if (index != -1) {
        index = mapToSource(this->index(index, 0)).row();
    }
    m_sourceModel->addAppToFavorites(id, index);
}

void FavoriteFilterModel::removeAppFromFavorites(const QString &id)
{
    m_sourceModel->removeAppFromFavorites(id);
}

void FavoriteFilterModel::exchangedAppsOrder(int indexFrom, int indexTo)
{
    indexFrom = mapToSource(this->index(indexFrom, 0)).row();
    indexTo = mapToSource(this->index(indexTo, 0)).row();
    m_sourceModel->exchangedAppsOrder(indexFrom, indexTo);
}

void FavoriteFilterModel::addAppsToNewFolder(const QString &idFrom, const QString &idTo)
{
    m_sourceModel->addAppsToNewFolder(idFrom, idTo);
}

void FavoriteFilterModel::addAppToFolder(const QString &appId, const QString &folderId)
{
    m_sourceModel->addAppToFolder(appId, folderId);
}

void FavoriteFilterModel::clearFavorites()
{
    m_sourceModel->clearFavorites();
}

QString FavoriteFilterModel::getDesktopFile(const QString &dragSourceUrl)
{
    return m_sourceModel->getDesktopFile(dragSourceUrl);
}

void FavoriteFilterModel::addAppFromFolder(const QString& appId, int order)
{
    FavoriteFolderHelper::instance()->removeAppFromFolder(appId);
    order = qBound(0, order, m_sourceModel->rowCount() - 1);
    int from = m_sourceModel->getOrderById(APP_ID_SCHEME + appId);
    if (from > -1 && from != order) {
        exchangedAppsOrder(from, order);
    }
}

bool FavoriteFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (sourceIndex.data(DataEntity::Type) == DataType::Folder) {
        FavoritesFolder folder;
        FavoriteFolderHelper::instance()->getFolderFromId(sourceIndex.data(DataEntity::Id).toInt(), folder);
        return !folder.getDisplayApps().isEmpty();
    }
    return SecurityFunctionControl::instance()->canAppDisplay(sourceIndex.data(DataEntity::Id).toString());
}

void FavoriteFilterModel::onSecurityControlChanged()
{
    invalidateFilter();
    dataChanged(index(0,0), index(rowCount() - 1, 0), {DataEntity::Icon});
}

FavoriteFilterModel::FavoriteFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setSourceModel(&AppFavoritesModel::instance());
    m_sourceModel = &AppFavoritesModel::instance();

    connect(SecurityFunctionControl::instance(), &SecurityFunctionControl::appWhiteListChanged, this, &FavoriteFilterModel::onSecurityControlChanged);
    connect(SecurityFunctionControl::instance(), &SecurityFunctionControl::appBlackListChanged, this, &FavoriteFilterModel::onSecurityControlChanged);
}

} // UkuiMenu
