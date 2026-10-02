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

#ifndef ITEMSFILTERMODEL_H
#define ITEMSFILTERMODEL_H

#include <QSortFilterProxyModel>
#include <QQuickItem>

class TrayItemsModel;
class ItemGroupModel;

class ItemsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    static ItemsFilterModel *instance();

    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void activate(const QModelIndex &index);
    Q_INVOKABLE void showContextMenu(const QModelIndex &index, QQuickItem *item);

    Q_INVOKABLE void setOrderInGroup(ItemGroupModel *group, const QModelIndex &groupIndex, int order);
    Q_INVOKABLE void setOrderBetweenGroups(ItemGroupModel *fromGroup, const QModelIndex &beginIndex, ItemGroupModel *toGroup, int order);
    Q_INVOKABLE void setSeparateIndex(int index);
    Q_INVOKABLE int getSeparateIndex();

    void setOrder(const QModelIndex &index, int order);

Q_SIGNALS:
    void menuStateChanged(bool state);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    ItemsFilterModel(QObject *parent = nullptr);
    TrayItemsModel *m_sourceModel = nullptr;
};

#endif // ITEMSFILTERMODEL_H
