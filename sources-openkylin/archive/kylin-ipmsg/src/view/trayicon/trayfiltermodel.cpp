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

#include "trayfiltermodel.h"
#include "model/friendlistmodel.h"
#include "view/friendlist/mysortfiltermodel.h"

TrayFilterModel::TrayFilterModel(QSortFilterProxyModel *parent) : QSortFilterProxyModel(parent) {}

TrayFilterModel::~TrayFilterModel() {}

bool TrayFilterModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    // int onlineStateL = sourceLeft.data(FriendListModel::OnlineState).toInt();
    // int priorityL    = sourceLeft.data(FriendListModel::Priority).toInt();
    QString msgTimeL = sourceLeft.data(FriendListModel::RecentMsgTime).toString();

    // int onlineStateR = sourceRight.data(FriendListModel::OnlineState).toInt();
    // int priorityR    = sourceRight.data(FriendListModel::Priority).toInt();
    QString msgTimeR = sourceRight.data(FriendListModel::RecentMsgTime).toString();

    // 仅按照时间进行排序
    qint64 compareDataL = MySortFilterModel::getCompareData(0, 0, msgTimeL);
    qint64 compareDataR = MySortFilterModel::getCompareData(0, 0, msgTimeR);

    // qDebug() << onlineStateL << priorityL << msgTimeL << compareDataL;
    // qDebug() << onlineStateR << priorityR << msgTimeR << compareDataR;

    if (compareDataL > compareDataR) {
        return true;
    } else {
        return false;
    }
    return true;
}

bool TrayFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex friendIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    int unreadMsgNum = friendIndex.data(FriendListModel::UnreadMsgNum).toInt();

    return unreadMsgNum > 0;
}
