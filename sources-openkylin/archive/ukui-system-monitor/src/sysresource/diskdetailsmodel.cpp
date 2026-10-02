/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "diskdetailsmodel.h"

DiskDetailsModel::DiskDetailsModel(QObject *parent) : QAbstractTableModel(parent)
{

}

DiskDetailsModel::~DiskDetailsModel()
{

}
QVariant DiskDetailsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();

    if (role == Qt::DisplayRole) {
        switch (row) {
        case 0:
            if (column == 0)
                return tr("Model"); //型号
            else if (column == 1)
                return tr("Available Capacity"); // 可用容量
            break;
        case 1:
            if (column == 0)
                return tr("Read Speed"); //读取速度
            else if (column == 1)
                return tr("Write Speed"); //写入速度
            break;
        case 2:
            if (column == 0)
                return tr("Type"); //类型
            break;
        default:
            break;
        }
    } else if (role == Qt::UserRole) {
        switch (row) {
        case 0:
            if (column == 0)
                return m_showinfo.model;
            else if (column == 1)
                return m_showinfo.size;
            break;
        case 1:
            if (column == 0)
                return m_showinfo.readSpeed;
            else if (column == 1)
                return m_showinfo.writeSpeed;
            break;
        case 2:
            if (column == 0)
                return m_showinfo.type;
            break;
        default:
            break;
        }
    } else if (role == Qt::TextColorRole) {
        const auto &palette = QPalette();
        return palette.color(QPalette::Text);
    }

    return QVariant();
}

void DiskDetailsModel::onModelUpdated()
{
    QAbstractTableModel::dataChanged(index(0, 0), index(2, 1));
}

void DiskDetailsModel::refreshDiskInfo(const QString &strKey)
{
    m_showinfo.model = diskInfo::getDiskModel(strKey);
    m_showinfo.size = diskInfo::getDiskCapacity(strKey);
    m_showinfo.readSpeed = DiskSpeedInfo::instance()->readSpeedStr(strKey);
    m_showinfo.writeSpeed = DiskSpeedInfo::instance()->writeSpeedStr(strKey);
    m_showinfo.type = diskInfo::getDiskType(strKey);
}
