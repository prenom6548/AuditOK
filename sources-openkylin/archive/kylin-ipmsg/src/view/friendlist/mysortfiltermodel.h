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

#ifndef MYSORTFILTERMODEL_H
#define MYSORTFILTERMODEL_H

#include <QObject>
#include <QWidget>
#include <QVariant>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <qglobal.h>
#include <QDebug>

class MySortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

protected:
    virtual bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const;
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;

public:
    explicit MySortFilterModel(QSortFilterProxyModel *parent = nullptr);
    ~MySortFilterModel();

    static qint64 getCompareData(int onlineState, int priority, QString msgTime);

    friend class TestMySortFilterModel;
};




#endif // MYSORTFILTERMODEL_H
