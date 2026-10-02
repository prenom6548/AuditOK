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
 * Authors: zy-yuan1 <zhangyuanyuan1@kylinos.cn>
 *
 */
#include "appwindowfiltermodel.h"
#include <QDebug>

AppWindowFilterModel::AppWindowFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setFilterRole(AppWindowListModel::DesktopsId);
}

void AppWindowFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    m_sourceModel = qobject_cast<AppWindowListModel*>(sourceModel);
    QSortFilterProxyModel::setSourceModel(sourceModel);
    connect(sourceModel, &QAbstractItemModel::dataChanged, this, &AppWindowFilterModel::onDataChanged);
}

QString AppWindowFilterModel::desktopId() const
{
    return m_desktopId;
}

void AppWindowFilterModel::setDesktopId(const QString id)
{
    if (m_desktopId == id) {
        return;
    }
    m_desktopId = id;
    invalidate();
}

int AppWindowFilterModel::screenId() const
{
    return m_screenId;
}

void AppWindowFilterModel::setScreenId(const int id)
{
    if (m_screenId == id) {
        return;
    }
    m_screenId = id;
    invalidate();
}

bool AppWindowFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QStringList desktopsId = m_sourceModel->index(source_row, 0, source_parent).data(AppWindowListModel::Role::DesktopsId).value<QStringList >();
    int screenId = m_sourceModel->index(source_row, 0, source_parent).data(AppWindowListModel::Role::ScreenId).toUInt();
    return desktopsId.contains(m_desktopId) && screenId == m_screenId;
}

void AppWindowFilterModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_EMIT dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
}
