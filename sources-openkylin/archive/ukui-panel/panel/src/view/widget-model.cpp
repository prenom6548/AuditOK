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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "widget-model.h"

#include <island.h>
#include <widget.h>
#include <widget-item.h>
#include <config-loader.h>
#include <function-control.h>
#include <QJsonArray>
#include <widget-container-item.h>

namespace UkuiPanel {

WidgetSortModel::WidgetSortModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool WidgetSortModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    auto leftItem = source_left.data(UkuiQuick::WidgetItemModel::WidgetItemRole).value<UkuiQuick::WidgetQuickItem *>();
    auto rightItem = source_right.data(UkuiQuick::WidgetItemModel::WidgetItemRole).value<UkuiQuick::WidgetQuickItem *>();

    int lIdx = m_widgetOrder.indexOf(leftItem->widget()->instanceId());
    int rIdx = m_widgetOrder.indexOf(rightItem->widget()->instanceId());

    if (lIdx < 0) {
        return false;
    }

    return lIdx < rIdx;
}

bool WidgetSortModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_layoutIndex == -1) {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    auto widget = sourceIndex.data(UkuiQuick::WidgetItemModel::WidgetItemRole).value<UkuiQuick::WidgetQuickItem *>()->widget();
    return widget->config()->getValue("layoutIndex").toInt() == m_layoutIndex;
}
void WidgetSortModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);
    QSortFilterProxyModel::sort(0);
}

QVariant WidgetSortModel::data(const QModelIndex& index, int role) const
{
    if (role == WidgetLayoutIndex) {
        QModelIndex sourceIndex = mapToSource(index);
        auto widget = sourceIndex.data(UkuiQuick::WidgetItemModel::WidgetItemRole).value<UkuiQuick::WidgetQuickItem *>()->widget();
        // 返回 layoutIndex 配置值
        return widget->config()->getValue("layoutIndex").toInt();
    }

    return sourceModel()->data(mapToSource(index), role);
}

QHash<int, QByteArray> WidgetSortModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(WidgetItemRole, "widgetItem");
    roles.insert(WidgetLayoutIndex, "widgetLayoutIndex");
    return roles;
}

QList<int> WidgetSortModel::widgetOrder() const
{
    return m_widgetOrder;
}

void WidgetSortModel::setWidgetOrder(const QList<int> &order)
{
    m_widgetOrder = order;
    invalidate();

    Q_EMIT widgetOrderChanged();
}

int WidgetSortModel::layoutIndex() const
{
    return m_layoutIndex;
}

void WidgetSortModel::setLayoutIndex(int index)
{
    m_layoutIndex = index;
    invalidate();

    Q_EMIT layoutIndexChanged();
}
} // UkuiPanel
