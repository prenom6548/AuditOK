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

#include "mysortfiltermodel.h"
#include "model/friendlistmodel.h"
#include "global/utils/globalutils.h"

MySortFilterModel::MySortFilterModel(QSortFilterProxyModel *parent) : QSortFilterProxyModel(parent) {}

MySortFilterModel::~MySortFilterModel() {}

bool MySortFilterModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    // qDebug() << "MySortFilterModel::lessThan";
    int onlineStateL = sourceLeft.data(FriendListModel::OnlineState).toInt();
    int priorityL = sourceLeft.data(FriendListModel::Priority).toInt();
    QString msgTimeL = sourceLeft.data(FriendListModel::RecentMsgTime).toString();

    int onlineStateR = sourceRight.data(FriendListModel::OnlineState).toInt();
    int priorityR = sourceRight.data(FriendListModel::Priority).toInt();
    QString msgTimeR = sourceRight.data(FriendListModel::RecentMsgTime).toString();

    qint64 compareDataL = MySortFilterModel::getCompareData(onlineStateL, priorityL, msgTimeL);
    qint64 compareDataR = MySortFilterModel::getCompareData(onlineStateR, priorityR, msgTimeR);

    // qDebug() << onlineStateL << priorityL << msgTimeL << compareDataL;
    // qDebug() << onlineStateR << priorityR << msgTimeR << compareDataR;

    if (compareDataL > compareDataR) {
        return true;
    } else {
        return false;
    }
    return true;
}

bool MySortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // qDebug() << "MySortFilterModel::filterAcceptsRow";

    QModelIndex friendIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    QString username = friendIndex.data(FriendListModel::Username).toString();
    QString nickname = friendIndex.data(FriendListModel::Nickname).toString();

    if (nickname.isEmpty()) {
        return username.contains(filterRegExp());

    } else {
        return nickname.contains(filterRegExp());
    }

    return false;
}

qint64 MySortFilterModel::getCompareData(int onlineState, int priority, QString msgTime)
{
    qint64 resData = 0;

    QString defaultTimeStr = "2020-01-01 00:00:00.000";
    QDateTime defaultTime = QDateTime::fromString(defaultTimeStr, GlobalUtils::getTimeFormat());
    qint64 defaultTimeInt = defaultTime.toMSecsSinceEpoch();

    qint64 msgTimeInt = 0;

    if (msgTime != "") {
        msgTimeInt = QDateTime::fromString(msgTime, GlobalUtils::getTimeFormat()).toMSecsSinceEpoch();
    } else {
        msgTimeInt = defaultTimeInt;
    }

    onlineState = onlineState == 0 ? 1 : 2;
    priority = priority == 0 ? 1 : 3;

    resData = msgTimeInt * onlineState * priority;

    return resData;
}
