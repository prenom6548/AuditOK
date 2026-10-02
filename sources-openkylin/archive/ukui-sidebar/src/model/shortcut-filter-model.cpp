/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

//
// Created by hxf on 22-7-28.
//

#include "shortcut-filter-model.h"
#include "shortcut-item.h"
#include "global-settings.h"
#include "shortcut-model.h"

#include <QDebug>

using namespace UkuiShortcut;

ShortcutFilterModel::ShortcutFilterModel(ShortcutModel *sourceModel, PluginStateType::FixState fixState, QObject *parent) : QSortFilterProxyModel(parent)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);

    m_sourceModel = sourceModel;
    m_fixState = fixState;
    connect(m_sourceModel, &ShortcutModel::dataChanged, this, &ShortcutFilterModel::onSourceModelDataChanged);
    sort(0);
}

QVariant ShortcutFilterModel::data(const QModelIndex &index, int role) const
{
    return sourceModel()->data(mapToSource(index), role);
}

void ShortcutFilterModel::active(int i, PluginMetaType::Action action)
{
    m_sourceModel->active(mapToSource(index(i, 0)).row(), action);
}

void ShortcutFilterModel::setValue(int i, int value)
{
    m_sourceModel->setValue(mapToSource(index(i, 0)).row(), value);
}

void ShortcutFilterModel::disableValueUpdate(bool disable)
{
    m_sourceModel->disableValueUpdate(disable);
}

void ShortcutFilterModel::updateValue(int idx)
{
    if (idx < 0 || idx >= rowCount()) return;
    QModelIndex current = index(idx, 0);
    Q_EMIT dataChanged(current, current, {ShortcutModel::Value});
}

bool ShortcutFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex modelIndex = sourceModel()->index(source_row, 0, source_parent);
    return (modelIndex.data(ShortcutModel::FixState).toInt() == m_fixState)
            && modelIndex.data(ShortcutModel::IsEnable).toBool()
            && modelIndex.data(ShortcutModel::IsVisible).toBool();
}

bool ShortcutFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    return source_left.data(ShortcutModel::PluginIndex).toInt() < source_right.data(ShortcutModel::PluginIndex).toInt();
}

void ShortcutFilterModel::onSourceModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (roles.contains(ShortcutModel::PluginIndex)) {
        invalidate();
    }
}
