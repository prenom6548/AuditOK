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
#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QMap>
#include <QMultiMap>
#include <QDBusInterface>
#include "notification-helper.h"

namespace Notify {

class NotificationManager : public QObject
{
    Q_OBJECT
public:
    explicit NotificationManager(QObject *parent = nullptr);

public:
    Q_INVOKABLE QStringList allAppNames();
    Q_INVOKABLE QList<Message> getMessages(QString &appName);

public Q_SLOTS:
    void newMessageRecived(Message &message);
    bool action(const QString &ID);
    void openSystemSetting();
    void deleteMessage(const QString &ID,const QString &appName);
    void deleteAllMessage();
    void deleteAppMessage(const QString &appName);

Q_SIGNALS:
    void newMessage(const Message &message);
    void iHaveUnreadMessage();
    void syncDeleteMessage(const QString &ID,const QString &appName);
    void syncDeleteAllMessage();
    void syncDeleteAppMessage(const QString &appName);

private:
    QMultiMap<QString, QString> m_nameID; //appName -> IDs
    QMap<QString, Message> m_messages;

};
}
#endif // NOTIFICATIONMANAGER_H
