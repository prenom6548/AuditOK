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

#ifndef UKUI_NOTIFICATION_SERVER_PRIVATE_H
#define UKUI_NOTIFICATION_SERVER_PRIVATE_H
#include <QObject>
#include <QDBusContext>
#include <QMap>
#include "notification-control-register.h"
#include "notification.h"
#include "notification-default-dir-watcher.h"
#include "notificaiton-settings-query.h"
#include "security-control-manager.h"
class QDBusServiceWatcher;
namespace NotificationServer {
class ServerPrivate : public QObject, protected QDBusContext
{
    Q_OBJECT
//dbus interface
public:
    QStringList GetCapabilities() const;
    uint Notify(const QString &app_name,
                uint replaces_id,
                const QString &app_icon,
                const QString &summary,
                const QString &body,
                const QStringList &actions,
                const QVariantMap &hints,
                int timeout);
    void CloseNotification(uint id);
    QString GetServerInformation(QString &vendor, QString &version, QString &spec_version) const;

    /**
     * 注册成为通知中心客户端
     */
    void RegisterClient();
    /**
     * 注销
     */
    void UnRegisterClient();
    /**
     * 通知中心关闭通知
     * @param id 通知id
     * @param reason 关闭原因
     */
    void CloseNotification(uint id, uint);
    void InvokeAction(uint id, const QString &action_key);
    void UpdateUnreadMessagesNumber(const QString &desktopEntry, uint number);

    /**
     * @brief RegisterNotificationControl
     * @param desktop 文件名，不带后缀
     */
    void RegisterNotificationControl(const QString &desktopEntry);

    /*!
     * @brief 是否允许闹钟播放提示音
     * @return
     */
    bool IsAlarmSoundAllowed();

    /*!
     *@brief 获取单个应用是否允许播放声音
     * @param desktop 文件名，不带后缀
     * @return bool
     */
    bool IsSoundAllowed(const QString &desktopEntry);
    /*!
     * @brief 设置勿扰模式
     * @param enable 是否开启勿扰模式
     */
    void SetDND(bool enable);
    bool IsDND();
Q_SIGNALS:
    void NotificationClosed(uint id, uint reason);
    void ActionInvoked(uint id, const QString &actionKey);
    void ActivationToken(uint id, const QString &ActivationToken);
    void UnreadMessagesNumberUpdated(const QString &desktopEntry, uint number);
    void AlarmSoundAllowedChanged(bool);
    void DNDChanged(bool dnd);
public:
    explicit ServerPrivate(QObject *parent = nullptr);
    ~ServerPrivate() override;
    bool init();

    static QString notificationServiceName();
    static QString notificationServicePath();
    static QString notificationServiceInterface();
private:
    void sendCache();
    QDBusServiceWatcher *m_notificationWatchers = nullptr;
    uint m_increasedNotificationId = 1;
    QMap<uint, NotificationPtr> m_notificationsCache;
    std::unique_ptr<NotificationControlRegister> m_notificationControlRegister;
    NotificationDefaultDirWatcher* m_notificationDefaultDirWatcher{nullptr};
    NotificaitonSettingsQuery* m_notificationSettingsQuery{nullptr};
    std::unique_ptr<SecurityControlManager> m_securityControlManager;

};



} // NotificationServer

#endif //UKUI_NOTIFICATION_SERVER_PRIVATE_H
