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
UsersModel::UsersModel(QObject *parent) : QAbstractListModel(parent), m_model(nullptr) {}

QVariant UsersModel::data(const QModelIndex &index, int role) const
{
    return m_model->index(index.row(), 0).data(role);
}

int UsersModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return sourceRowCount();
}

QHash<int, QByteArray> UsersModel::roleNames() const
{
    return m_model == nullptr ? QHash<int, QByteArray>() : m_model->roleNames();
}

void UsersModel::setSourceModel(QAbstractListModel *sourceModel)
{
    if (m_model) {
        disconnect(
            m_model,
            SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this,
            SLOT(onSourceRowsInserted(const QModelIndex &, int, int)));
        disconnect(
            m_model,
            SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this,
            SLOT(onSourceRowsRemoved(const QModelIndex &, int, int)));
        disconnect(
            m_model,
            SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this,
            SLOT(onSourceDataChanged(const QModelIndex &, const QModelIndex &)));
    }
    m_model = sourceModel;
    beginResetModel();
    endResetModel();

    connect(
        m_model,
        SIGNAL(rowsInserted(const QModelIndex &, int, int)),
        this,
        SLOT(onSourceRowsInserted(const QModelIndex &, int, int)));
    connect(
        m_model,
        SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
        this,
        SLOT(onSourceRowsRemoved(const QModelIndex &, int, int)));
    connect(
        m_model,
        SIGNAL(dataChanged(QModelIndex, QModelIndex)),
        this,
        SLOT(onSourceDataChanged(const QModelIndex &, const QModelIndex &)));
}

int UsersModel::sourceRowCount() const
{
    return m_model == nullptr ? 0 : m_model->rowCount();
}

void UsersModel::onSourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    dataChanged(createIndex(topLeft.row(), 0), createIndex(bottomRight.row(), 0));
    Q_EMIT userInfoChanged();
}

void UsersModel::onSourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    beginInsertRows(parent, start, end);
    endInsertRows();
    Q_EMIT userInfoChanged();
}

void UsersModel::onSourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    beginRemoveRows(parent, start, end);
    endRemoveRows();
    Q_EMIT userInfoChanged();
}
