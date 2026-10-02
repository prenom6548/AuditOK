/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#ifndef UKUI_SIDEBAR_NOTIFICATION_GROUP_MODEL_H
#define UKUI_SIDEBAR_NOTIFICATION_GROUP_MODEL_H

#include <QAbstractProxyModel>

namespace UkuiNotification {

class NotificationGroupModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    explicit NotificationGroupModel(QObject *parent = nullptr);
    void setSourceModel(QAbstractItemModel *sourceModel) override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    bool hasChildren(const QModelIndex &parent) const override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &proxyIndex, int role) const override;

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Q_INVOKABLE void clearGroup(const QModelIndex &groupIndex);

    Q_INVOKABLE QString stripRichText(const QString &str) const;

private Q_SLOTS:
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void onRowInserted(const QModelIndex &parent, int first, int last);
    void onRowRemoved(const QModelIndex &parent, int first, int last);

private:
    void rebuildGroups();
    void adjustSourceIndex(int base, int offset);
    void addNewItemToModel(int groupIndex, int sourceIndex);
    void removeItemFromModel(int groupIndex, int sourceIndex);

    // 返回group的index
    int findAppGroupIndex(int sourceIndex) const;
    int findAppGroupIndex(const QModelIndex &sourceIndex) const;
    bool compareApp(const QModelIndex &a, const QModelIndex &b) const;

    int findParentIndex(const QModelIndex &child) const;
    int findGroupIndexByRow(int row);

private:
    QVector<QVector<int>*> m_groups;
    QVector<bool> m_groupStatus;
};

} // UkuiNotification

#endif //UKUI_SIDEBAR_NOTIFICATION_GROUP_MODEL_H
