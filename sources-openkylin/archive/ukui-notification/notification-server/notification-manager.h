/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 * Authors: sundagao <sundagao@kylinos.cn>
 */

#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QMap>
#include "notification.h"
namespace NotificationServer {

class NotificationManager : public QObject
{
    Q_OBJECT
public:
    static NotificationManager& self();
    Q_DISABLE_COPY(NotificationManager)

    void notify(NotificationPtr notification);
    void invokedAction(const uint, const QString&);

public Q_SLOTS:
    void onNotificationClosed(uint id, uint reason);

private:
    explicit NotificationManager(QObject *parent = nullptr);
    ~NotificationManager();
    QMap<uint, NotificationPtr> m_notifications;
};
}

#endif // NOTIFICATIONMANAGER_H
