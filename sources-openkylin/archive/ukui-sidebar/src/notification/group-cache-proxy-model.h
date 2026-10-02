/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef UKUI_SIDEBAR_GROUP_CACHE_PROXY_MODEL_H
#define UKUI_SIDEBAR_GROUP_CACHE_PROXY_MODEL_H

#include <QAbstractProxyModel>
#include <QPersistentModelIndex>
#include <utility>

namespace UkuiNotification {

class GroupCacheProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    explicit GroupCacheProxyModel(QObject *parent = nullptr);
    void setSourceModel(QAbstractItemModel *sourceModel) override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

    QVariant data(const QModelIndex &proxyIndex, int role) const override;

    Q_INVOKABLE void setRootIndex(QModelIndex rootIndex);
    Q_INVOKABLE QModelIndex getRootIndex(int rootIndex);
//    Q_INVOKABLE void removeItem(int index);
    Q_INVOKABLE void removeItem(uint id);

Q_SIGNALS:
//    void removeItem(int row);
    void itemRemoved(int row);

private Q_SLOTS:
    void onRowInserted(const QModelIndex &parent, int first, int last);
    void onRowRemoved(const QModelIndex &parent, int first, int last);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

private:
    class CacheItem {
    public:
        CacheItem(uint i, QModelIndex index) : id(i), sourceIndex(index) {};
        CacheItem(uint i, QPersistentModelIndex index) : id(i), sourceIndex(std::move(index)) {};
        uint id;
        QPersistentModelIndex sourceIndex;
    };

private:
    bool m_isValid = false;
    QPersistentModelIndex m_rootModelIndex;

    QVector<CacheItem> m_items;
//    QVector<QPersistentModelIndex> m_items;
};

}

#endif //UKUI_SIDEBAR_GROUP_CACHE_PROXY_MODEL_H
