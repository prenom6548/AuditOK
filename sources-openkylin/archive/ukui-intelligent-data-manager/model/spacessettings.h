/*
 * UKUI Intelligent Data Manager
 *
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef SPACESSETTINGS_H
#define SPACESSETTINGS_H

#include <QObject>

class QTimer;

class SpacesSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Qt::SortOrder sortOrder READ getSortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
    Q_PROPERTY(int sortType READ getSortType WRITE setSortType NOTIFY sortTypeChanged)
    Q_PROPERTY(bool useGlobalSorting READ getUseGlobalSorting WRITE setUseGlobalSorting NOTIFY useGlobalSortingChanged)
    Q_PROPERTY(bool userAddedFirst READ getUserAddedFirst WRITE setUserAddedFirst NOTIFY userAddedFirstChanged)

    Q_PROPERTY(bool isSpacesCollapsed READ getIsSpacesCollapsed WRITE setIsSpacesCollapsed NOTIFY isSpacesCollapsedChanged)
    Q_PROPERTY(bool isRecentCollapsed READ getIsRecentCollapsed WRITE setIsRecentCollapsed NOTIFY isRecentCollapsedChanged)
    Q_PROPERTY(bool isSpaceListMode READ getIsSpaceListMode WRITE setIsSpaceListMode NOTIFY isSpaceListModeChanged)
    Q_PROPERTY(bool isPreviewMode READ getIsPreviewMode WRITE setIsPreviewMode NOTIFY isPreviewModeChanged)
public:
    static SpacesSettings *globalInstance();
    explicit SpacesSettings(QObject *parent = nullptr);

    Q_INVOKABLE Qt::SortOrder getSortOrder() const;
    Q_INVOKABLE void setSortOrder(Qt::SortOrder newSortOrder);

    Q_INVOKABLE int getSortType() const;
    Q_INVOKABLE void setSortType(int newSortType);

    Q_INVOKABLE bool getUseGlobalSorting() const;
    Q_INVOKABLE void setUseGlobalSorting(bool newUseGlobalSorting);

    Q_INVOKABLE bool getUserAddedFirst() const;
    Q_INVOKABLE void setUserAddedFirst(bool newUserAddedFirst);

    void setSpacesSortConfig(int spaceId, int sortType, int sortOrder, bool userAddedFirst);
    QVariantHash getSpacesSortConfig(int spaceId);

    bool getIsSpacesCollapsed() const;
    void setIsSpacesCollapsed(bool newIsSpacesCollapsed);

    bool getIsRecentCollapsed() const;
    void setIsRecentCollapsed(bool newIsRecentCollapsed);

    bool getIsSpaceListMode() const;
    void setIsSpaceListMode(bool newIsSpaceListMode);

    bool getIsPreviewMode() const;
    void setIsPreviewMode(bool newIsPreviewMode);

signals:

    void sortOrderChanged();

    void sortTypeChanged();

    void useGlobalSortingChanged();

    void userAddedFirstChanged();

    void sortConfigChanged();

    void isSpacesCollapsedChanged();

    void isRecentCollapsedChanged();

    void isSpaceListModeChanged();

    void isPreviewModeChanged();

private:
    Qt::SortOrder sortOrder = Qt::DescendingOrder;
    int sortType = 0;
    bool useGlobalSorting = true;
    bool userAddedFirst = true;

    bool isSpacesCollapsed = false;
    bool isRecentCollapsed = false;

    bool isSpaceListMode = false;

    bool isPreviewMode = false;

    QTimer *delayer = nullptr;
};

#endif // SPACESSETTINGS_H
