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

#ifndef NOTIFICATION_PORTAL_H
#define NOTIFICATION_PORTAL_H

#include <QDBusAbstractAdaptor>
#include <QString>
#include <QVariantList>
class NotificationService;
class NotificationActionInfo
{
public:
    NotificationActionInfo(const QString &appId, const QString &notificationId, const QString &action, const QVariant &target);
    QString appId() const;
    QString notificationId() const;
    QString action() const;
    QVariant target() const;

private:
    QString m_appId;
    QString m_notificationId;
    QString m_action;
    QVariant m_target;
};
class NotificationPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Notification")
public:
    explicit NotificationPortal(QObject *parent = nullptr);

public Q_SLOTS:
    void AddNotification(const QString &app_id, const QString &id, const QVariantMap &notification);
    void RemoveNotification(const QString &app_id, const QString &id);

private Q_SLOTS:
    void actionInvoked(uint nId, const QString &action);
    void notificationClosed(uint nId, uint reason);

private:
    static QString getAppNameById(const QString &appId);

private:
    NotificationService *m_service = nullptr;
    QHash<uint, QList<NotificationActionInfo>> m_id2Actions;
    QHash<QString, uint> m_notifications;

};



#endif //NOTIFICATION_PORTAL_H
