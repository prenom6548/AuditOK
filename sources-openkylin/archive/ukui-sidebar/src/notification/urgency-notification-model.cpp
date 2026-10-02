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
#include "urgency-notification-model.h"

#include <QDebug>

using namespace UkuiNotification;

UrgencyNotificationModel::UrgencyNotificationModel(QObject *parent)
    :QSortFilterProxyModel(parent)
{

}

void UrgencyNotificationModel::closeNotification(uint id)
{
    NotificationModel::instance()->closeNotification(id);
}

void UrgencyNotificationModel::execAction(uint id, QString action)
{
    NotificationModel::instance()->execAction(id, action);
}

bool UrgencyNotificationModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
     QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

     if (sourceIndex.data(NotificationItem::Urgency).toInt() == PopupNotification::Urgency::CriticalUrgency) {
         return true;
     }
     return false;
}

bool UrgencyNotificationModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    return sourceLeft.data(NotificationItem::CreateTime).toDateTime() < sourceRight.data(NotificationItem::CreateTime).toDateTime();
}
