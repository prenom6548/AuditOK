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

#include "notification-group-model.h"

#include "notification-model.h"

#include <QTextDocument>
#include <QMutableVectorIterator>

#include <QDebug>

using namespace UkuiNotification;

NotificationGroupModel::NotificationGroupModel(QObject *parent) : QAbstractProxyModel(parent)
{
    qRegisterMetaType<UkuiNotification::NotificationGroupModel*>();
}

void NotificationGroupModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (sourceModel == QAbstractProxyModel::sourceModel()) {
        return;
    }

    beginResetModel();

    if (QAbstractProxyModel::sourceModel()) {
        QAbstractProxyModel::sourceModel()->disconnect(this);
    }

    qDeleteAll(m_groups);
    QAbstractProxyModel::setSourceModel(sourceModel);

    if (QAbstractProxyModel::sourceModel()) {
        rebuildGroups();

        connect(sourceModel, &QAbstractItemModel::dataChanged, this, &NotificationGroupModel::onDataChanged);
        connect(sourceModel, &QAbstractItemModel::rowsInserted, this, &NotificationGroupModel::onRowInserted);
        connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &NotificationGroupModel::onRowRemoved);
        connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, [=] (const QModelIndex &parent, int first, int last) {
            if (parent.isValid()) {
                return ;
            }

            adjustSourceIndex(first, -(last - first + 1));
        });

        connect(sourceModel, &QAbstractItemModel::modelReset, this, [this] {
            beginResetModel();
            qDeleteAll(m_groups);
            rebuildGroups();
            endResetModel();
        });
    }

    endResetModel();
}

void NotificationGroupModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                           const QVector<int> &roles)
{
    if (topLeft.parent() != bottomRight.parent()) {
        return;
    }

    if (topLeft == bottomRight) {
        if (roles.contains(NotificationItem::NoFold) || roles.contains(NotificationItem::AppName)) {
            int row = topLeft.row();
            int groupIndex = findGroupIndexByRow(row);
            removeItemFromModel(groupIndex, row);
            addNewItemToModel(findAppGroupIndex(row), row);
            return;
        }
        QModelIndex top = mapFromSource(topLeft);
        if (!top.isValid()) {
            return;
        }
        Q_EMIT dataChanged(top, top, roles);
    }

    QModelIndex top = mapFromSource(topLeft);
    if (!top.isValid()) {
        return;
    }

    QModelIndex bottom = mapFromSource(bottomRight);
    if (!bottom.isValid()) {
        return;
    }

    if (top.parent() == bottom.parent()) {
        Q_EMIT dataChanged(top, bottom, roles);

    } else {
        Q_EMIT dataChanged(top.parent(), top.parent(), roles);
        Q_EMIT dataChanged(bottom.parent(), bottom.parent(), roles);
    }
}

/**
 * 调整内部存储索引指向
 * @param base
 * @param offset
 */
void NotificationGroupModel::adjustSourceIndex(int base, int offset)
{
    for (QVector<int> *group : m_groups) {
        QMutableVectorIterator<int> it(*group);
        while (it.hasNext()) {
            const int &value = it.next();
            if (value >= base) {
                it.setValue(value + offset);
            }
        }
    }
}

void NotificationGroupModel::onRowInserted(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid()) {
        return;
    }

    if (first < (sourceModel()->rowCount() -1)) {
        // relocation
        adjustSourceIndex(first, last - first + 1);
    }

    for (int i = first; i <= last; ++i) {
        addNewItemToModel(findAppGroupIndex(i), i);
    }
}

void NotificationGroupModel::onRowRemoved(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid()) {
        return ;
    }

    for (int sourceIndex = first; sourceIndex <= last; ++sourceIndex) {
        QModelIndex removedIndex = sourceModel()->index(sourceIndex, 0, parent);

        int groupIndex = findAppGroupIndex(removedIndex);
        if (groupIndex < 0) {
            continue;
        }

        removeItemFromModel(groupIndex, sourceIndex);
    }
}

void NotificationGroupModel::rebuildGroups()
{
    m_groupStatus.clear();
    int count = sourceModel()->rowCount();
    for (int i = 0; i < count; ++i) {
        addNewItemToModel(findAppGroupIndex(i), i);
    }
}

void NotificationGroupModel::addNewItemToModel(int groupIndex, int sourceIndex)
{
    QVector<int> *group;

    if (groupIndex < 0) {
        group = new QVector<int>(1, sourceIndex);

        // 插入group
        beginInsertRows(QModelIndex(), 0, 0);
        m_groups.prepend(group);
        m_groupStatus.prepend(false);
        endInsertRows();

//        // TODO: update group count
//        beginInsertRows(index(m_groups.size() - 1, 0, QModelIndex()), 0, 0);
//        group->append(sourceIndex);
//        endInsertRows();

    } else {
        group = m_groups.at(groupIndex);

        if (groupIndex > 0) {
            beginMoveRows(QModelIndex(), groupIndex, groupIndex, QModelIndex(), 0);
            m_groups.move(groupIndex, 0);
            m_groupStatus.move(groupIndex, 0);
            endMoveRows();
        }

        QModelIndex groupModelIndex = index(0, 0, QModelIndex());

        beginInsertRows(groupModelIndex, 0, 0);
        group->prepend(sourceIndex);
        endInsertRows();

        Q_EMIT dataChanged(groupModelIndex, groupModelIndex);
    }

    Q_EMIT dataChanged(index(0, 0, QModelIndex()), index(m_groups.size() - 1, 0, QModelIndex()), {NotificationItem::GroupIndex});
}

void NotificationGroupModel::removeItemFromModel(int groupIndex, int sourceIndex)
{
    if (groupIndex < 0) return;

    QModelIndex groupModelIndex = index(groupIndex, 0, QModelIndex());
    QVector<int>* group = m_groups[groupIndex];
    if (group->size() > 1) {
        int appIndex = group->indexOf(sourceIndex);

        beginRemoveRows(groupModelIndex, appIndex, appIndex);
        group->removeAt(appIndex);
        endRemoveRows();

        Q_EMIT dataChanged(groupModelIndex, groupModelIndex);

    } else {
        beginRemoveRows(QModelIndex(), groupIndex, groupIndex);
        delete m_groups.takeAt(groupIndex);
        m_groupStatus.removeAt(groupIndex);
        endRemoveRows();

        Q_EMIT dataChanged(index(0, 0, QModelIndex()), index(m_groups.size() - 1, 0, QModelIndex()), {NotificationItem::GroupIndex});
    }
}

int NotificationGroupModel::findAppGroupIndex(int sourceIndex) const
{
    if (sourceModel()) {
        return findAppGroupIndex(sourceModel()->index(sourceIndex, 0));
    }

    return -1;
}

int NotificationGroupModel::findAppGroupIndex(const QModelIndex &sourceIndex) const
{
    for (int i = 0; i < m_groups.size(); ++i) {
        QModelIndex group = sourceModel()->index(m_groups.at(i)->at(0), 0);
        if (compareApp(group, sourceIndex)) {
            return i;
        }
    }

    return -1;
}

bool NotificationGroupModel::compareApp(const QModelIndex &a, const QModelIndex &b) const
{
    bool allowFold = !a.data(NotificationItem::NoFold).toBool() && !b.data(NotificationItem::NoFold).toBool();

    return allowFold ? a.data(NotificationItem::AppName).toString() == b.data(NotificationItem::AppName).toString()
                      : a.data(NotificationItem::Id).toUInt() == b.data(NotificationItem::Id).toUInt();
}

QModelIndex NotificationGroupModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return {};
    }

    if (parent.isValid()) {
        return createIndex(row, column, m_groups.value(parent.row()));
    }

    return createIndex(row, column);
}

int NotificationGroupModel::findParentIndex(const QModelIndex &child) const
{
    QVector<int> *group = static_cast<QVector<int> *>(child.internalPointer());
    if (group) {
        return m_groups.indexOf(group);
    }

    return -1;
}

int NotificationGroupModel::findGroupIndexByRow(int row)
{
    for (int i = 0; i < m_groups.size(); i++ ){
        for (int j = 0; j < m_groups[i]->size(); j++ ){
            if (m_groups[i]->at(j) == row){
                return i;
            }
        }
    }
    return -1;
}

QModelIndex NotificationGroupModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return {};
    }

    int r = findParentIndex(child);
    if (r < 0) {
        return {};
    }

    return createIndex(r, 0);
}

bool NotificationGroupModel::hasChildren(const QModelIndex &parent) const
{
    if (!sourceModel()) {
        return false;
    }

    // root
    if (!parent.isValid()) {
        return !m_groups.isEmpty();
    }

    // child
    if (parent.parent().isValid()) {
        return false;
    }

    return true;
}

QModelIndex NotificationGroupModel::mapToSource(const QModelIndex &proxyIndex) const
{
    // root
    if (!sourceModel() || !proxyIndex.isValid()) {
        return {};
    }

    // child
    if (proxyIndex.parent().isValid()) {
        int r = m_groups.at(proxyIndex.parent().row())->at(proxyIndex.row());
        return sourceModel()->index(r, 0);
    }

    // group
    return {};
}

QModelIndex NotificationGroupModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    int i = findAppGroupIndex(sourceIndex);
    if (i < 0) {
        return {};
    }

    int r = m_groups.at(i)->indexOf(sourceIndex.row());
    if (r < 0) {
        return {};
    }

    return index(r, 0, index(i, 0, QModelIndex()));
}

int NotificationGroupModel::rowCount(const QModelIndex &parent) const
{
    if (!sourceModel()) {
        return 0;
    }

    // root
    if (!parent.isValid()) {
        return m_groups.size();
    }

    // child
    if (parent.parent().isValid()) {
        return 0;
    }


    // group
    int r = parent.row();
    if (r < 0 || r >= m_groups.size()) {
        return 0;
    }

    return m_groups.at(r)->size();
}

int NotificationGroupModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant NotificationGroupModel::data(const QModelIndex &proxyIndex, int role) const
{
    if (!proxyIndex.isValid()) {
        return {};
    }

    if (proxyIndex.parent().isValid()) {
        return QAbstractProxyModel::data(proxyIndex, role);
    }

    int groupIndex = proxyIndex.row();
    switch (role) {
        case NotificationItem::GroupIndex:
            return groupIndex;
        case NotificationItem::GroupName:
            return sourceModel()->index(m_groups.at(groupIndex)->at(0), 0, QModelIndex()).data(NotificationItem::AppName);
        case NotificationItem::GroupCount:
            return m_groups.at(groupIndex)->size();
        case NotificationItem::GroupIsExpand:
            return m_groupStatus.at(groupIndex);
        default:
            break;
    }

    return {};
}

bool NotificationGroupModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == NotificationItem::GroupIsExpand) {
        if (!index.isValid() || index.parent().isValid()) {
            return false;
        }

        m_groupStatus[index.data(NotificationItem::GroupIndex).toInt()] = value.toBool();
        Q_EMIT dataChanged(index, index, {NotificationItem::GroupIsExpand});
        return true;
    }

    return QAbstractProxyModel::setData(index, value, role);
}

void NotificationGroupModel::clearGroup(const QModelIndex &groupIndex)
{
    if (!checkIndex(groupIndex, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return;
    }

    int r = groupIndex.row();
    beginRemoveRows(QModelIndex(), r, r);

    QVector<int> *group = m_groups.takeAt(r);
    for (const auto &sourceRow : *group) {
        NotificationModel::instance()->removeNotification(sourceModel()->index(sourceRow, 0, QModelIndex()).data(NotificationItem::Id).toUInt());
    }

    delete group;
    endRemoveRows();

    Q_EMIT dataChanged(index(0, 0, QModelIndex()), index(m_groups.size() - 1, 0, QModelIndex()), {NotificationItem::GroupIndex});
}

QString NotificationGroupModel::stripRichText(const QString &str) const
{
    if (Qt::mightBeRichText(str)) {
        QTextDocument doc;
        doc.setHtml(str);
        return doc.toPlainText();
    }
    return str;
}
