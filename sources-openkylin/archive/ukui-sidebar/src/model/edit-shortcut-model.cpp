/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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

#include "edit-shortcut-model.h"
#include "shortcut-model.h"
#include "shortcut-config.h"

#include <QModelIndex>
#include <QtDebug>
#include <QGuiApplication>

using namespace UkuiShortcut;

EditShortcutModel::EditShortcutModel(ShortcutModel *sourceModel, PluginStateType::AddState addState, QObject *parent)
    : ShortcutFilterModel(sourceModel, PluginStateType::NotFix, parent)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);

    m_addState = addState;
    m_layoutDirection = QGuiApplication::layoutDirection();

    connect(qGuiApp, &QGuiApplication::layoutDirectionChanged, this, [this] {
        m_layoutDirection = QGuiApplication::layoutDirection();
    });

    invalidate();
}

void EditShortcutModel::moveItem(int sourceIndex, int targetIndex)
{
    int srcRow = data(index(sourceIndex, 0), ShortcutModel::PluginIndex).toInt();
    int targetRow = data(index(targetIndex, 0), ShortcutModel::PluginIndex).toInt();
    ShortcutConfig::instance().setShortcutOrder(srcRow, targetRow, m_addState);
}

void EditShortcutModel::setOrder(QString fromId, int toIndex)
{
    int targetRow = data(index(toIndex, 0), ShortcutModel::PluginIndex).toInt();
    ShortcutConfig::instance().setShortcutOrder(fromId, targetRow, m_addState);
}

void EditShortcutModel::changeState(QString idFrom, PluginStateType::AddState stateFrom, bool atEnd)
{
    ShortcutConfig::instance().changeShortcutState(idFrom, stateFrom, atEnd);
}

void EditShortcutModel::updateItemRect(QQuickItem *flowItem)
{
    QList<QQuickItem *> items = flowItem->childItems();

    int currentRow = 0;
    int currentColumn = 0;

    for (int i = 0; i < items.count(); ++i) {
        QQuickItem* item = items.at(i);
        const int columnSpan = item->property("columnSpan").toInt();

        if (columnSpan == 0) break;

        if (currentColumn + columnSpan > m_columns) {
            currentRow++;
            currentColumn = 0;
        }

        const qreal x = currentColumn * (m_itemWidth + m_columnSpacing);
        const qreal y = currentRow * (m_itemHeight + m_rowSpacing);

        const QRect rect = QRect(x, y, m_itemWidth * columnSpan + m_columnSpacing * (columnSpan - 1), m_itemHeight);

        item->setProperty("targetRect", rect);

        currentColumn += columnSpan;
    }
    flowItem->setProperty("rows", currentRow + 1);

    m_items = items;
    m_flowWidth = flowItem->width();
}

QVariantMap EditShortcutModel::findTargetItem(qreal x, qreal y) const
{
    QVariantMap result {
        {QStringLiteral("index"), -1},
        {QStringLiteral("offset"), 0}
    };

    if (m_layoutDirection == Qt::LayoutDirection::RightToLeft) {
        x = m_flowWidth - x;
    }

    for (int i = 0; i < m_items.count(); ++i) {
        QQuickItem* item = m_items.at(i);
        const QRect rect = item->property("targetRect").toRect();
        if (rect.contains(x, y)) {
            result.insert(QStringLiteral("index"), i);
            result.insert(QStringLiteral("offset"), rect.width() > 80 ? (rect.x() + rect.width()/2 > x) ? 0 : 1 : -1);
            return result;
        }
    }

    return result;
}

bool EditShortcutModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex modelIndex = sourceModel()->index(source_row, 0, source_parent);
    return (modelIndex.data(ShortcutModel::AddState).toInt() == m_addState)
            && ShortcutFilterModel::filterAcceptsRow(source_row, source_parent);
}
