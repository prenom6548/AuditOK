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

#include "group-cache-proxy-model.h"
#include "notification-model.h"
#include <QDebug>

using namespace UkuiNotification;

GroupCacheProxyModel::GroupCacheProxyModel(QObject *parent) : QAbstractProxyModel(parent)
{

}

void GroupCacheProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (sourceModel == QAbstractProxyModel::sourceModel()) {
        return;
    }

    beginResetModel();

    if (QAbstractProxyModel::sourceModel()) {
        QAbstractProxyModel::sourceModel()->disconnect(this);
    }

    QAbstractProxyModel::setSourceModel(sourceModel);
    if (sourceModel) {
        connect(sourceModel, &QAbstractItemModel::rowsInserted, this, &GroupCacheProxyModel::onRowInserted);
//        connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, [=] (const QModelIndex &parent, int first, int last) {
//            qDebug() << "=rowsRemoved=" << first << last;
//        });
        connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &GroupCacheProxyModel::onRowRemoved);
        connect(sourceModel, &QAbstractItemModel::dataChanged,this, &GroupCacheProxyModel::onDataChanged);
    }

    endResetModel();
}

QModelIndex GroupCacheProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || column > 0) {
        return {};
    }

    return createIndex(row, column, nullptr);
}

QModelIndex GroupCacheProxyModel::parent(const QModelIndex &child) const
{
    return {};
}

int GroupCacheProxyModel::rowCount(const QModelIndex &parent) const
{
    if (sourceModel() && m_rootModelIndex.isValid()) {
        return m_items.size();
    }

    return 0;
}

int GroupCacheProxyModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QModelIndex GroupCacheProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (m_rootModelIndex.isValid()) {
        int r = proxyIndex.row();
        if (r >= 0 && r < m_items.size()) {
            QModelIndex modelIndex = m_items.at(r).sourceIndex;
            return modelIndex;
        }
    }

    return {};
}

QModelIndex GroupCacheProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid() || !sourceIndex.parent().isValid()) {
        return {};
    }

    if (sourceModel()->hasChildren(sourceIndex)) {
        return {};
    }

    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).sourceIndex == sourceIndex) {
            return createIndex(i, sourceIndex.column(), nullptr);
        }
    }

    return {};
}

QVariant GroupCacheProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    if (!checkIndex(proxyIndex, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    return QAbstractProxyModel::data(mapToSource(proxyIndex), role);
}

void GroupCacheProxyModel::onRowInserted(const QModelIndex &parent, int first, int last)
{
    if (!parent.isValid() || (parent != m_rootModelIndex)) {
        return;
    }

    beginInsertRows(QModelIndex(), 0, (last - first));

    for (int i = last; i >= last; --i) {
        QModelIndex si = sourceModel()->index(i, 0, parent);
        m_items.prepend(CacheItem(si.data(NotificationItem::Id).toUInt(), si));
    }

    endInsertRows();
}

void GroupCacheProxyModel::onRowRemoved(const QModelIndex &parent, int first, int last)
{
    if (!parent.isValid() || parent != m_rootModelIndex) {
        return;
    }

    int row = first;
    QVector<int> indexList;
    for (int i = 0; i < m_items.size() && row <= last; ++i) {
        auto item = m_items.at(i);
        if (item.sourceIndex.row() == row) {
            indexList.append(i);
            ++row;
        }
    }

    for (const auto &index : indexList) {
        Q_EMIT itemRemoved(index);
    }
}

void GroupCacheProxyModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                         const QVector<int> &roles)
{
    if ((topLeft.parent() != m_rootModelIndex) || !topLeft.isValid() || !topLeft.parent().isValid()) {
        return;
    }

    Q_EMIT dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
}

void GroupCacheProxyModel::setRootIndex(QModelIndex rootIndex)
{
    if (m_rootModelIndex == rootIndex) {
        return;
    }

    beginResetModel();
    m_rootModelIndex = rootIndex;
    m_items.clear();

    if (sourceModel()) {
        for (int i = 0; i < sourceModel()->rowCount(m_rootModelIndex); ++i) {
            auto si = sourceModel()->index(i, 0, m_rootModelIndex);
            m_items.append(CacheItem(si.data(NotificationItem::Id).toUInt(), si));
        }
    }
    endResetModel();
}

QModelIndex GroupCacheProxyModel::getRootIndex(int rootIndex)
{
    if (sourceModel()) {
        return sourceModel()->index(rootIndex, 0, QModelIndex());
    }

    return {};
}

//void GroupCacheProxyModel::removeItem(int index)
//{
//    if (index >= 0 && index < m_items.size()) {
//        auto item = m_items.at(index);
//        qDebug() << "=removeItem=" << index << m_items.size() << item.sourceIndex.isValid();
//        beginRemoveRows(QModelIndex(), index, 0);
//        m_items.removeAt(index);
//        endRemoveRows();
//    }
//}

void GroupCacheProxyModel::removeItem(uint id)
{
    int i = m_items.size() - 1;
    for (; i >= 0; --i) {
        if (m_items.at(i).id == id) {
            beginRemoveRows(QModelIndex(), i, i);
            m_items.removeAt(i);
            endRemoveRows();
            return;
        }
    }
}
