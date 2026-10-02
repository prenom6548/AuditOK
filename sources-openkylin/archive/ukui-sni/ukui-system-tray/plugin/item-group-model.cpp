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

#include "item-group-model.h"
#include "tray-items-model.h"
#include "items-filter-model.h"
#include "security-function-control.h"

#include <QAbstractItemModel>
#include <QDebug>

ItemGroupModel::ItemGroupModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    QSortFilterProxyModel::setSourceModel(ItemsFilterModel::instance());
    m_sourceModel = ItemsFilterModel::instance();
    setFilterRole(TrayItemsModel::Row);
    connect(TrayItemsModel::instance(), &TrayItemsModel::separateIndexChanged, this, &ItemGroupModel::onSeparateIndexChanged);

    setCurrentSeparateIndex(m_sourceModel->getSeparateIndex());
}

void ItemGroupModel::setModelType(GroupType type)
{
    m_groupType = type;
}

ItemGroupModel::GroupType ItemGroupModel::getModelType()
{
    return m_groupType;
}

int ItemGroupModel::currentSeparateIndex()
{
    return m_currentSeparateIndex;
}

void ItemGroupModel::setCurrentSeparateIndex(int index)
{
    if (index < 0) return;
    m_currentSeparateIndex = index;
    invalidateFilter();
    Q_EMIT currentSeparateIndexChanged();
}

int ItemGroupModel::showCount()
{
    return m_showCount;
}

void ItemGroupModel::setShowCount(int index)
{
    m_showCount = index <= 0 ? 0 : index;
    onShowCountChanged();
    Q_EMIT showCountChanged();
}

void ItemGroupModel::onActivate(const QModelIndex &index)
{
    m_sourceModel->activate(mapToSource(index));
}

void ItemGroupModel::onContextMenu(const QModelIndex &index, QQuickItem *item)
{
    m_sourceModel->showContextMenu(mapToSource(index), item);
}

QVariant ItemGroupModel::data(const QModelIndex &index, int role) const
{
    return sourceModel()->data(mapToSource(index), role);
}

QVariant ItemGroupModel::foldIcon()
{
    return QIcon::fromTheme("ukui-end-symbolic");
}

int ItemGroupModel::groupBegin() const
{
    if (m_groupType == GroupType::Show) {
        return 0;
    } else if (m_groupType == GroupType::Fold) {
        return m_currentSeparateIndex + 1;
    }
}

int ItemGroupModel::groupEnd() const
{
    if (m_groupType == GroupType::Show) {
        return m_currentSeparateIndex;
    } else if (m_groupType == GroupType::Fold) {
        return m_sourceModel->rowCount(QModelIndex()) - 1;
    }
}

bool ItemGroupModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_groupType == GroupType::Show) {
        return source_row >= 0 && source_row <= m_currentSeparateIndex;
    } else if (m_groupType == GroupType::Fold) {
        return source_row > m_currentSeparateIndex && source_row <= (m_sourceModel->rowCount(QModelIndex()) - 1);
    }
    return true;
}

void ItemGroupModel::onShowCountChanged()
{
    if (m_showCount < m_currentSeparateIndex) {
        if (m_showCount <= 0) {
            setCurrentSeparateIndex(0);
        } else {
            setCurrentSeparateIndex(m_showCount);
        }
    } else {
        if (m_currentSeparateIndex != m_sourceModel->getSeparateIndex()) {
            if (m_sourceModel->getSeparateIndex() < m_showCount) {
                setCurrentSeparateIndex(m_sourceModel->getSeparateIndex());
            } else {
                setCurrentSeparateIndex(m_showCount);
            }
        }
    }
    Q_EMIT currentSeparateIndexChanged();
}

void ItemGroupModel::onSeparateIndexChanged()
{
    setCurrentSeparateIndex(m_showCount < m_sourceModel->getSeparateIndex() ?
                m_showCount : m_sourceModel->getSeparateIndex());
    Q_EMIT currentSeparateIndexChanged();
}

