/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#ifndef TRAYFILTERMODEL_H
#define TRAYFILTERMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>

class TrayFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    friend class TestTrayFilterModel;
protected:
    virtual bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const;

    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

public:
    explicit TrayFilterModel(QSortFilterProxyModel *parent = nullptr);
    ~TrayFilterModel();
};

#endif // TRAYFILTERMODEL_H
