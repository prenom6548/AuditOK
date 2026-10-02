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
#ifndef APPWINDOWFILTERMODEL_H
#define APPWINDOWFILTERMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>
#include "appwindowlistmodel.h"

class AppWindowFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString desktopId READ desktopId WRITE setDesktopId)
    Q_PROPERTY(int screenId READ screenId WRITE setScreenId)

public:
    explicit AppWindowFilterModel(QObject *parent = nullptr);
    ~AppWindowFilterModel() = default;
    Q_INVOKABLE void setSourceModel(QAbstractItemModel *sourceModel) override;

    QString desktopId() const;
    void setDesktopId(const QString id);
    int screenId() const;
    void setScreenId(const int id);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());

    AppWindowListModel *m_sourceModel = nullptr;
    QString m_desktopId;
    int m_screenId = 0;
};

#endif // APPWINDOWFILTERMODEL_H
