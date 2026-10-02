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

#ifndef FAVORITEFILTERMODEL_H
#define FAVORITEFILTERMODEL_H

#include <QSortFilterProxyModel>

namespace UkuiMenu {

class AppFavoritesModel;

class FavoriteFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    static FavoriteFilterModel *instance();

    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void openMenu(const int &row);
    Q_INVOKABLE void addAppToFavorites(const QString &id, int index = -1);
    Q_INVOKABLE void removeAppFromFavorites(const QString &id);
    Q_INVOKABLE void exchangedAppsOrder(int indexFrom, int indexTo);
    Q_INVOKABLE void addAppsToNewFolder(const QString &idFrom, const QString &idTo);
    Q_INVOKABLE void addAppToFolder(const QString &appId, const QString& folderId);
    Q_INVOKABLE void clearFavorites();
    Q_INVOKABLE QString getDesktopFile(const QString &dragSourceUrl);
    Q_INVOKABLE void addAppFromFolder(const QString& appId, int order = -1);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private Q_SLOTS:
    void onSecurityControlChanged();

private:
    FavoriteFilterModel(QObject *parent = nullptr);
    AppFavoritesModel *m_sourceModel = nullptr;
};

} // UkuiMenu

#endif // FAVORITEFILTERMODEL_H
