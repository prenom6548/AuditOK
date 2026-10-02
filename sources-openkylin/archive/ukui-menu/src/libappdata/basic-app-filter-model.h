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

#ifndef BASICAPPFILTERMODEL_H
#define BASICAPPFILTERMODEL_H

#include <QSortFilterProxyModel>

#include "app-database-interface.h"

namespace UkuiMenu {

class BasicAppModel;

class BasicAppFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    static BasicAppFilterModel *instance();

    const AppDatabaseInterface *databaseInterface() const;
    DataEntity appOfIndex(int row) const;
    int indexOfApp(const QString &appid) const;
    bool getAppById(const QString &appid, DataEntity &app) const;

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    BasicAppFilterModel(QObject *parent = nullptr);
    BasicAppModel *m_sourceModel = nullptr;
};

} // UkuiMenu

#endif // BASICAPPFILTERMODEL_H
