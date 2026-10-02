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

#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusPendingCallWatcher>
#include "notification-manager.h"
#include "server.h"
#include <QDebug>
using namespace NotificationServer;

NotificationManager& NotificationManager::self()
{
    static NotificationManager instance;
    return instance;
}

void NotificationManager::notify(NotificationPtr notification)
{
    uint id = NotificationServer::Server::self().notify(notification);
    m_notifications.insert(id, notification);
}

void NotificationManager::invokedAction(uint id, const QString& actionId)
{
    auto notification = m_notifications[id];
    if (notification) {
        notification->activeAction(actionId);
    }
}

void NotificationManager::onNotificationClosed(uint id, uint reason)
{
    Q_UNUSED(reason)
    auto iter = m_notifications.find(id);
    if (iter != m_notifications.end()) {
        m_notifications.erase(iter);
    }
}

NotificationManager::NotificationManager(QObject *parent)
    : QObject{parent}
{

}

NotificationManager::~NotificationManager()
{
    m_notifications.clear();
}
