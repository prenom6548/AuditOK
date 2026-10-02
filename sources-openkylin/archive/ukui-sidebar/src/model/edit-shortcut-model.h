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

#ifndef EDITSHORTCUTMODEL_H
#define EDITSHORTCUTMODEL_H

#include <QVariantMap>
#include <QQuickItem>

#include "shortcut-filter-model.h"
#include "plugin-state.h"

namespace UkuiShortcut {

class ShortcutModel;

class EditShortcutModel : public ShortcutFilterModel
{
    Q_OBJECT
public:
    explicit EditShortcutModel(ShortcutModel* sourceModel, PluginStateType::AddState addState, QObject *parent = nullptr);

    Q_INVOKABLE void moveItem(int sourceIndex, int targetIndex);
    Q_INVOKABLE void setOrder(QString fromId, int toIndex);
    Q_INVOKABLE void changeState(QString idFrom, PluginStateType::AddState stateFrom, bool atEnd = true);

    Q_INVOKABLE void updateItemRect(QQuickItem* flowItem);
    Q_INVOKABLE QVariantMap findTargetItem(qreal x, qreal y) const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    int m_itemWidth = 80;
    int m_itemHeight = 72;
    int m_columns = 4;
    int m_rowSpacing = 12;
    int m_columnSpacing = 12;
    int m_flowWidth = 0;

    Qt::LayoutDirection m_layoutDirection{Qt::LayoutDirection::LeftToRight};
    QList<QQuickItem *> m_items;
    PluginStateType::AddState m_addState = PluginStateType::Add;
};
}
#endif // EDITSHORTCUTMODEL_H
