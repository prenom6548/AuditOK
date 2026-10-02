/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#ifndef UKUI_NOTIFICATION_SERVER_H
#define UKUI_NOTIFICATION_SERVER_H
#include <QObject>
#include "notification.h"
namespace NotificationServer {
class ServerPrivate;
class Server : public QObject
{
    Q_OBJECT
public:
    static Server &self();
    ~Server();
    bool init();

    uint notify(NotificationPtr notification);
    void closeNotification(uint id);

private:
    explicit Server(QObject *parent = nullptr);
    Q_DISABLE_COPY(Server)
    ServerPrivate *d = nullptr;
};

} // NotificationServer

#endif //UKUI_NOTIFICATION_SERVER_H
