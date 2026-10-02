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

#include "basic-app-filter-model.h"
#include "basic-app-model.h"
#include "security-function-control.h"
#include <QDebug>

namespace UkuiMenu {

BasicAppFilterModel *BasicAppFilterModel::instance()
{
    static BasicAppFilterModel model;
    return &model;
}

const AppDatabaseInterface *BasicAppFilterModel::databaseInterface() const
{
    return m_sourceModel->databaseInterface();
}

DataEntity BasicAppFilterModel::appOfIndex(int row) const
{
    return m_sourceModel->appOfIndex(mapToSource(index(row, 0)).row());
}

int BasicAppFilterModel::indexOfApp(const QString &appid) const
{
    return mapFromSource(m_sourceModel->index(m_sourceModel->indexOfApp(appid))).row();
}

bool BasicAppFilterModel::getAppById(const QString &appid, DataEntity &app) const
{
    return m_sourceModel->getAppById(appid, app);
}

QVariant BasicAppFilterModel::data(const QModelIndex &index, int role) const
{
    return sourceModel()->data(mapToSource(index), role);
}

bool BasicAppFilterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return m_sourceModel->setData(mapToSource(index), value, role);
}

bool BasicAppFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    return SecurityFunctionControl::instance()->canAppDisplay(sourceIndex.data(DataEntity::Id).toString());
}

BasicAppFilterModel::BasicAppFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setSourceModel(BasicAppModel::instance());
    m_sourceModel = BasicAppModel::instance();

    connect(SecurityFunctionControl::instance(), &SecurityFunctionControl::appWhiteListChanged, this, &BasicAppFilterModel::invalidateFilter);
    connect(SecurityFunctionControl::instance(), &SecurityFunctionControl::appBlackListChanged, this, &BasicAppFilterModel::invalidateFilter);
}
} // UkuiMenu
