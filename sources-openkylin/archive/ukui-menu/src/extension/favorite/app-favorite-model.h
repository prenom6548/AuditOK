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

#ifndef UKUI_MENU_APP_FAVORITE_MODEL_H
#define UKUI_MENU_APP_FAVORITE_MODEL_H

#include "data-entity.h"
#include "libappdata/basic-app-model.h"

#include <QObject>
#include <QAbstractListModel>

namespace UkuiMenu {

class AppFavoritesModel : public QAbstractListModel
{
Q_OBJECT
public:
    static AppFavoritesModel &instance();
    ~AppFavoritesModel() override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void changeFileState(const QString &url, const bool &favorite); //TODO
    int getOrderById(const QString&id);

    void openMenu(const int &row);
    /**
     * 通过id添加应用到收藏
     * @param id
     * @param index 默认为-1,添加到收藏最后一位;可以添加到收藏指定位置
     */
    void addAppToFavorites(const QString &id, int index = -1);
    /**
     * 通过id从收藏中移除
     * @param id
     */
    void removeAppFromFavorites(const QString &id);
    /**
     * 拖拽交换位置。
     * @param indexFrom
     * @param indexTo
     */
    void exchangedAppsOrder(int indexFrom, int indexTo);
    /**
     * 两个应用推拽合并为应用组。
     * @param idFrom
     * @param idTo 拖拽至某一应用，并在该位置创建应用组
     */
    void addAppsToNewFolder(const QString &idFrom, const QString &idTo);
    /**
     * 添加应用到应用组，包括新建应用组。
     * @param appId
     * @param folderId id为""时,新建应用组；id为数值时，添加至对应应用组
     */
    void addAppToFolder(const QString &appId, const QString& folderId);
    /**
     * 清空所有已收藏应用
     */
    void clearFavorites();
    /**
     * 通过url获取对应的应用名称
     **/
    QString getDesktopFile(const QString &dragSourceUrl);

private:
    explicit AppFavoritesModel(QObject* parent = nullptr);
    QVariant folderDataFromId(const QString &id, int role) const;
    QVariant fileDataFromUrl(const QString &url, int role) const;

    class Private;
    Private *d = nullptr;
};

} // UkuiMenu

#endif //UKUI_MENU_APP_FAVORITE_MODEL_H
