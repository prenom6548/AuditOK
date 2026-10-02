/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "proxymodel.h"
#include <QDebug>
KyUsersModel::KyUsersModel(QObject *parent) : QAbstractListModel(parent), m_abstractListModel(nullptr) {}

QVariant KyUsersModel::data(const QModelIndex &index, int role) const
{
    return m_abstractListModel->index(index.row(), 0).data(role);
}

int KyUsersModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return sourceRowCount();
}

QHash<int, QByteArray> KyUsersModel::roleNames() const
{
    return m_abstractListModel == nullptr ? QHash<int, QByteArray>() : m_abstractListModel->roleNames();
}

void KyUsersModel::setSourceModel(QAbstractListModel *sourceModel)
{
    if (m_abstractListModel) {
        disconnect(
            m_abstractListModel,
            SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this,
            SLOT(onSourceRowsInserted(const QModelIndex &, int, int)));
        disconnect(
            m_abstractListModel,
            SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this,
            SLOT(onSourceRowsRemoved(const QModelIndex &, int, int)));
        disconnect(
            m_abstractListModel,
            SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this,
            SLOT(onSourceDataChanged(const QModelIndex &, const QModelIndex &)));
    }
    m_abstractListModel = sourceModel;
    beginResetModel();
    endResetModel();

    connect(
        m_abstractListModel,
        SIGNAL(rowsInserted(const QModelIndex &, int, int)),
        this,
        SLOT(onSourceRowsInserted(const QModelIndex &, int, int)));
    connect(
        m_abstractListModel,
        SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
        this,
        SLOT(onSourceRowsRemoved(const QModelIndex &, int, int)));
    connect(
        m_abstractListModel,
        SIGNAL(dataChanged(QModelIndex, QModelIndex)),
        this,
        SLOT(onSourceDataChanged(const QModelIndex &, const QModelIndex &)));
}

int KyUsersModel::sourceRowCount() const
{
    return m_abstractListModel == nullptr ? 0 : m_abstractListModel->rowCount();
}

void KyUsersModel::onSourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    dataChanged(createIndex(topLeft.row(), 0), createIndex(bottomRight.row(), 0));
    Q_EMIT userInfoChanged();
}

void KyUsersModel::onSourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    beginInsertRows(parent, start, end);
    endInsertRows();
    Q_EMIT userInfoChanged();
}

void KyUsersModel::onSourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    beginRemoveRows(parent, start, end);
    endRemoveRows();
    Q_EMIT userInfoChanged();
}
