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

#ifndef SHORTCUTFILTERMODEL_H
#define SHORTCUTFILTERMODEL_H

#include <QSortFilterProxyModel>
#include "plugin-common-data.h"
#include "plugin-state.h"

namespace UkuiShortcut {

class ShortcutModel;

class ShortcutFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ShortcutFilterModel(ShortcutModel* sourceModel, PluginStateType::FixState fixState, QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void active(int i, PluginMetaType::Action action);
    Q_INVOKABLE void setValue(int i, int value);
    Q_INVOKABLE void disableValueUpdate(bool disable);
    Q_INVOKABLE void updateValue(int idx);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

protected Q_SLOTS:
    void onSourceModelDataChanged(const QModelIndex &topLeft,
                                  const QModelIndex &bottomRight,
                                  const QVector<int> &roles = QVector<int>());
private:
    ShortcutModel* m_sourceModel = nullptr;
    PluginStateType::FixState m_fixState = PluginStateType::Fix;
};

} // UkuiShortcut

#endif //SHORTCUTFILTERMODEL_H
