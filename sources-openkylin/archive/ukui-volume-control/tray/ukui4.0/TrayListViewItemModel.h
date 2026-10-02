/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef TRAYLISTVIEWITEMMODEL_H
#define TRAYLISTVIEWITEMMODEL_H

#include <QIcon>
#include <QAbstractItemModel>

class TrayListViewItemModel : public QAbstractItemModel
{
public:
    TrayListViewItemModel(QObject* parent = nullptr);

    void addDevice(const QString& iconPath, const QVariantList& list);
    void removeDevice(const QString& portName, const QString& devName);

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex&) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    std::vector<std::pair<QIcon, QVariantList>> devices;
};


#endif // TRAYLISTVIEWITEMMODEL_H
