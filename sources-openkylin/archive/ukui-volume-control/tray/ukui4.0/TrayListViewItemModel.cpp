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

#include "TrayListViewItemModel.h"

#include <vector>
#include <utility>
#include <QDebug>

TrayListViewItemModel::TrayListViewItemModel(QObject* parent) : QAbstractItemModel(parent)
{
}

void TrayListViewItemModel::addDevice(const QString& iconPath, const QVariantList& list)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    devices.push_back(std::make_pair(QIcon::fromTheme(iconPath), list));
    endInsertRows();
}

void TrayListViewItemModel::removeDevice(const QString& portName, const QString& devName)
{
//    if (row >= 0 && row < rowCount())
//    {
//        beginRemoveRows(QModelIndex(), row, row);
//        devices.erase(devices.begin() + row);
//        endRemoveRows();
//    }

    std::erase_if(devices, [portName, devName] (const auto& devPair) {
        const auto& [currentIconName, variantList] = devPair;
        return variantList.at(0) == portName && variantList.at(1) == devName;
    });
}

QModelIndex TrayListViewItemModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

QModelIndex TrayListViewItemModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int TrayListViewItemModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return devices.size();
    return 0;
}

int TrayListViewItemModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant TrayListViewItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
        return devices[index.row()].second.at(1);
    else if (role == Qt::DecorationRole && index.column() == 0)
    {
        return devices[index.row()].first;
    }

    return QVariant();
}
