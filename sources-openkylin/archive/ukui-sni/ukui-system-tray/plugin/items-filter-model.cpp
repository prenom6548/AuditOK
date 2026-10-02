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

#include "items-filter-model.h"
#include "tray-items-model.h"
#include "item-group-model.h"
#include "security-function-control.h"

#include <QDebug>

ItemsFilterModel *ItemsFilterModel::instance()
{
    static ItemsFilterModel model;
    return &model;
}

QVariant ItemsFilterModel::data(const QModelIndex &index, int role) const
{
    return sourceModel()->data(mapToSource(index), role);
}

void ItemsFilterModel::activate(const QModelIndex &index)
{
    if (SecurityFunctionControl::instance()->canLeftClick(index.data(TrayItemsModel::Desktop).toString())) {
        m_sourceModel->activate(mapToSource(index));
    }
}

void ItemsFilterModel::showContextMenu(const QModelIndex &index, QQuickItem *item)
{
    if (SecurityFunctionControl::instance()->canRightClick(index.data(TrayItemsModel::Desktop).toString())) {
        m_sourceModel->showContextMenu(mapToSource(index), item);
    }
}

void ItemsFilterModel::setOrderInGroup(ItemGroupModel *group, const QModelIndex &groupIndex, int order)
{
    int end = group->groupEnd();
    int newOrder = group->groupBegin() + order;
    if(newOrder > end) {
        newOrder = end;
    }
    m_sourceModel->setOrder(mapToSource(group->mapToSource(groupIndex)),
                            mapToSource(this->index(newOrder, 0)).row());
}

void ItemsFilterModel::setOrderBetweenGroups(ItemGroupModel *fromGroup, const QModelIndex &beginIndex, ItemGroupModel *toGroup, int order)
{
    int newOrder = toGroup->groupBegin() + order;
    if(newOrder > toGroup->groupEnd()) {
        newOrder = toGroup->groupEnd() + 1;
    }
    m_sourceModel->setOrder(mapToSource(fromGroup->mapToSource(beginIndex)),
                            mapToSource(this->index(newOrder, 0)).row());
}

void ItemsFilterModel::setSeparateIndex(int index)
{
    if (index > rowCount() - 1) {
        index = rowCount() - 1;
    }
    m_sourceModel->setSeparateIndex(index);
}

int ItemsFilterModel::getSeparateIndex()
{
    return m_sourceModel->getSeparateIndex();
}

bool ItemsFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    return SecurityFunctionControl::instance()->canAppDisplay(sourceIndex.data(TrayItemsModel::Desktop).toString()) &&
            !m_sourceModel->getControlPanelItems().contains(sourceIndex.data(TrayItemsModel::Id).toString());
}

ItemsFilterModel::ItemsFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setSourceModel(TrayItemsModel::instance());
    m_sourceModel = TrayItemsModel::instance();

    connect(m_sourceModel, &TrayItemsModel::menuStateChanged, this, &ItemsFilterModel::menuStateChanged);
    connect(SecurityFunctionControl::instance(), &SecurityFunctionControl::appWhiteListChanged, this, &ItemsFilterModel::invalidateFilter);
    connect(SecurityFunctionControl::instance(), &SecurityFunctionControl::appBlackListChanged, this, &ItemsFilterModel::invalidateFilter);
}
