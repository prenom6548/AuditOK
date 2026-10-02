/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#include "notification-manager.h"
#include "app-manager.h"

#include <QDebug>
#include <QDBusReply>
#include <QDBusConnection>
#include <QProcess>

using namespace Notify;
NotificationManager::NotificationManager(QObject *parent) : QObject(parent)
{
    connect(NotificationHelper::instance(), &NotificationHelper::newMessage, this, &NotificationManager::newMessageRecived);
}

QStringList NotificationManager::allAppNames()
{
    return m_nameID.keys();
}

QList<Message> NotificationManager::getMessages(QString &appName)
{
    QList<Message> messages;
    for(QString id : m_nameID.values(appName)) {
        messages.append(m_messages.value(id));
    }
    return messages;
}

void NotificationManager::newMessageRecived(Message &message)
{
    m_messages.insert(message.getID(), message);
    m_nameID.insert(message.getAppName(), message.getID());

    Q_EMIT newMessage(message);
    Q_EMIT iHaveUnreadMessage();
}

bool NotificationManager::action(const QString &ID)
{
    if(m_messages.contains(ID)) {
        return m_messages.value(ID).executeDefaultAction();
    }
    return true;
}

void NotificationManager::deleteMessage(const QString &ID,const QString &appName)
{
    m_nameID.remove(m_messages.value(ID).getAppName(), ID);
    m_messages.remove(ID);

    Q_EMIT syncDeleteMessage(ID,appName);
}

void NotificationManager::deleteAllMessage()
{
    m_messages.clear();
    m_nameID.clear();

    Q_EMIT syncDeleteAllMessage();
}

void NotificationManager::deleteAppMessage(const QString &appName)
{
//    QMap<Key, T>::iterator find_index = m_nameID.find(appName);
    for (const QString &messageID : m_nameID.values(appName)) {
        m_messages.remove(messageID);
    }
    m_nameID.remove(appName);

    Q_EMIT syncDeleteAppMessage(appName);
}

void NotificationManager::openSystemSetting()
{
    QStringList args;
    args<<"-m"<<"Notice";
    Sidebar::AppManager::getInstance(this)->launchAppWithArguments("/usr/share/applications/ukui-control-center.desktop", args, "ukui-control-center");
}
