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

#pragma once

#include <vector>
#include <list>

#include "PipewireNode.h"

namespace UkuiAudioFramwork
{
class PipewireNodeListModel
{

    enum NodeRoles {
//        IndexRole = Qt::UserRole + 1,
        NodeRole,
        IDRole,
        ActiveRole,
        RateRole,
        QuantumRole,
        WaitRole,
        BusyRole,
        NameRole,
        DriverIDRole,
    };
    const unsigned int N_ROLES = 10;

private:
    std::vector<PipewireNode*> m_nodes;

public:
    explicit PipewireNodeListModel(/*QObject *parent = nullptr*/);
    virtual ~PipewireNodeListModel();

    inline std::vector<PipewireNode*> &list() { return m_nodes; }
    inline const std::vector<PipewireNode*> &constList() const { return m_nodes; }

//    Q_INVOKABLE
    void sortList();

public:
    // Delegate methods from m_nodes
    void append(PipewireNode *node);
    bool removeOne(PipewireNode *node);
    bool removeAt(int index);
    inline int size() const { return m_nodes.size(); }
    inline PipewireNode*& operator[](int index) { return m_nodes[index]; }
    inline PipewireNode* const& operator[](int index) const { return m_nodes[index]; }

public:
    // QAbstractItemModel interface
//    virtual int rowCount(const QModelIndex& index = QModelIndex()) const override;
//    //virtual int columnCount(const QModelIndex& index = QModelIndex()) const override;
//    virtual QVariant data(const QModelIndex& index, int role) const override;
//    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
//    virtual bool insertRows(int row, int count, const QModelIndex& parent) override;
//    virtual bool removeRows(int row, int count, const QModelIndex& parent) override;
//    virtual QHash<int, QByteArray> roleNames() const override;

    void rowChanged(PipewireNode* node, int role);

//    Q_INVOKABLE
    void move(int oldIndex, int newIndex);
};
}
