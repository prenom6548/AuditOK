/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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

#ifndef AUTOTESTS_WINDOWS_STUB_URGENCY_NOTIFICATION_MODEL_H
#define AUTOTESTS_WINDOWS_STUB_URGENCY_NOTIFICATION_MODEL_H

#include <QSortFilterProxyModel>

#include "notification-model.h"

class UrgencyNotificationModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit UrgencyNotificationModel(QObject *parent = nullptr);

    Q_INVOKABLE void closeNotification(uint id);
    Q_INVOKABLE void execAction(uint id, QString action = QString());

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;
};

#endif
