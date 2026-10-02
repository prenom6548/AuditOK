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

#ifndef UKUI_NOTIFICATION_NOTIFICATION_CLIENT_PRIVATE_H
#define UKUI_NOTIFICATION_NOTIFICATION_CLIENT_PRIVATE_H
#include <QObject>
#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include "notifications_interface.h"
#include "notification-client.h"
namespace UkuiNotification {
class NotificationClientPrivate : public QObject
{
    Q_OBJECT
public:
    explicit NotificationClientPrivate(NotificationClient *q);
    ~NotificationClientPrivate() override;
    /**
     * 初始化客户端dbus服务并注册成为客户端
     * @return
     */
    bool init();
    /**
     * 注销
     */
    void unregisterClient();
    static QString clientServicePath();
    static QString clientServiceInterface();

    void Notify(const QString &app_name,
                uint replaces_id,
                const QString &app_icon,
                const QString &summary,
                const QString &body,
                const QStringList &actions,
                const QVariantMap &hints,
                int timeout);
    bool closeNotification(uint id, NotificationCloseReason::CloseReason reason);
    bool invokeAction(uint id, const QString &action_key);

private Q_SLOTS:
    void notificationClosed(uint id, uint reason);
    void serviceChange(const QString &service, const QString &oldOwner, const QString &newOwner);
    /**
     * 用于注册客户端，或在服务发生变化（重启）后重新注册客户端
     * @return true 成功 false 失败
     */
    bool registerClient();

private:
    OrgFreedesktopNotificationsInterface* m_notificationsInterface = nullptr;
    QDBusInterface *m_serverInterface = nullptr;
    QDBusServiceWatcher *m_watcher = nullptr;
    bool m_registered = false;
    NotificationClient *q;
};
}
#endif //UKUI_NOTIFICATION_NOTIFICATION_CLIENT_PRIVATE_H
