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

#include "notification-client.h"
#include "notification-client-private.h"
#include <QDBusConnection>
#include <QQmlEngine>
#include "notificationclientadaptor.h"
#include <dbus-connector.h>
using namespace UkuiNotification;

NotificationClientPrivate::NotificationClientPrivate(NotificationClient *q) : QObject(q), q(q)
{
}

NotificationClientPrivate::~NotificationClientPrivate()
{
    unregisterClient();
}

bool NotificationClientPrivate::init()
{
    if(m_registered) {
        return false;
    }
    m_registered = true;
    new NotificationClientAdaptor(this);
    QDBusConnection conn = QDBusConnection::sessionBus();

    if(!conn.registerObject(clientServicePath(), clientServiceInterface(), this)) {
        qWarning() << "Failed to register NotificationClient DBus object!" << conn.lastError();
        return false;
    }
    m_watcher = new QDBusServiceWatcher(QStringLiteral("org.freedesktop.Notifications"),
                                                           conn,
                                                           QDBusServiceWatcher::WatchForOwnerChange,
                                                           this);
    connect(m_watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &NotificationClientPrivate::serviceChange);
    return registerClient();
}

bool NotificationClientPrivate::registerClient()
{
    QDBusConnection conn = QDBusConnection::sessionBus();
    if(!m_notificationsInterface) {
        m_notificationsInterface = new OrgFreedesktopNotificationsInterface(QStringLiteral("org.freedesktop.Notifications"),
                                                                            QStringLiteral("/org/freedesktop/Notifications"),
                                                                            conn);

        connect(m_notificationsInterface, &OrgFreedesktopNotificationsInterface::NotificationClosed,
                this, &NotificationClientPrivate::notificationClosed);
    }

    auto *connector = new UkuiQuick::DbusConnector(QStringLiteral("org.freedesktop.Notifications"),
                                        QStringLiteral("/org/freedesktop/Notifications"),
                                        QStringLiteral("org.ukui.NotificationServer"),
                                        conn);

    connect(connector, &UkuiQuick::DbusConnector::ready, this, [this, connector](QDBusInterface* interface)
    {
        if (interface && interface->isValid()) {
            m_serverInterface = interface;
            QDBusPendingCall reply = m_serverInterface->asyncCall(QStringLiteral("RegisterClient"));
            auto *watcher = new QDBusPendingCallWatcher(reply, this);
            connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
                if (watcher->isError()) {
                    qWarning() << "Failed to register client:" << watcher->error().message();
                    // Handle registration failure if needed
                    return;
                }
               watcher->deleteLater();
            });
        } else {
            qWarning() << "Failed to get NotificationServer interface!" << QDBusConnection::sessionBus().lastError();
        }
        connector->deleteLater();
    });
    connector->start();
    return true;
}

void NotificationClientPrivate::unregisterClient()
{
    if(m_notificationsInterface) {
        disconnect(m_notificationsInterface, &OrgFreedesktopNotificationsInterface::NotificationClosed,
                   this, &NotificationClientPrivate::notificationClosed);
        delete m_notificationsInterface;
        m_notificationsInterface = nullptr;
    }
    QDBusConnection conn = QDBusConnection::sessionBus();
    if(m_serverInterface) {
        QDBusMessage reply = m_serverInterface->call(QStringLiteral("UnRegisterClient"));
        if (reply.type() == QDBusMessage::ErrorMessage || reply.type() == QDBusMessage::InvalidMessage) {
            qWarning() << "Failed to call UnRegisterClient!" << conn.lastError() << reply.type();
            return;
        }
    }
    conn.unregisterObject(clientServicePath(), QDBusConnection::UnregisterTree);

    m_registered = false;
}

QString NotificationClientPrivate::clientServicePath()
{
    return QStringLiteral("/NotificationClient");
}

QString NotificationClientPrivate::clientServiceInterface()
{
    return QStringLiteral("org.ukui.NotificationClient");
}

void NotificationClientPrivate::Notify(const QString &app_name, uint replaces_id, const QString &app_icon,
                                       const QString &summary, const QString &body, const QStringList &actions,
                                       const QVariantMap &hints, int timeout)
{
    PopupNotification notification(replaces_id);
    notification.setSummary(summary);
    notification.setBody(body);
    notification.setApplicationName(app_name);
    notification.setApplicationIconName(app_icon);
    notification.setActions(actions);
    notification.setTimeout(timeout);
    notification.setHints(hints);
    Q_EMIT q->newNotification(notification);
}

void NotificationClientPrivate::notificationClosed(uint id, uint reason)
{
    auto closeReason = NotificationCloseReason::CloseReason::Undefined;
    switch (reason) {
        case 1:
            closeReason = NotificationCloseReason::CloseReason::Expired;
            break;
        case 2:
            closeReason = NotificationCloseReason::CloseReason::DismissedByUser;
            break;
        case 3:
            closeReason = NotificationCloseReason::CloseReason::Revoked;
            break;
    }
    Q_EMIT q->notificationClosed(id, closeReason);
}

bool NotificationClientPrivate::closeNotification(uint id, NotificationCloseReason::CloseReason reason)
{
    if(!m_serverInterface) {
        return false;
    }
    QDBusMessage reply = m_serverInterface->callWithArgumentList(QDBus::BlockWithGui, QStringLiteral("CloseNotification"), {id, static_cast<uint32_t>(reason)});
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "Failed to call CloseNotification!" << QDBusConnection::sessionBus().lastError();
        return false;
    }
    return true;
}

void NotificationClientPrivate::serviceChange(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    qDebug() << "Notification Service" << service << "status change, old owner:" << oldOwner << "new:" << newOwner;
    if(oldOwner.isEmpty() && m_registered) {
        registerClient();
    }
}

bool NotificationClientPrivate::invokeAction(uint id, const QString &action_key)
{
    if(!m_serverInterface) {
        return false;
    }
    qDebug() << "invokeAction" << id << action_key;
    QDBusMessage reply = m_serverInterface->callWithArgumentList(QDBus::NoBlock,
                                                                        QStringLiteral("InvokeAction"),
                                                                        {id, action_key});
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "Failed to call InvokedAction!" << QDBusConnection::sessionBus().lastError();
        return false;
    }
    return true;
}

/*!
    \class UkuiNotification::NotificationClient
    \inmodule UkuiNotification
    \inheaderfile notification-client.h
    \brief UKUI 通知系统客户端

    这个类封装了与通知服务交互的细节，并提供交互接口。 通过 \l {registerClient()} 向通知服务注册为客户端，即可接收通知服务发送的消息。
 */

/*!
    \fn void UkuiNotification::NotificationClient::newNotification(const UkuiNotification::PopupNotification &notification)

    此信号在通知服务收到新通知时发送.
 */

/*!
    \fn void UkuiNotification::NotificationClient::notificationClosed(uint id, UkuiNotification::NotificationCloseReason::CloseReason closeReason)

    此信号在通知被关闭时发送.
 */
NotificationClient::NotificationClient(QObject *parent) : QObject(parent), d(new NotificationClientPrivate(this))
{
    qRegisterMetaType<PopupNotification>("PopupNotification");
    qRegisterMetaType<UkuiNotification::PopupNotification>("const UkuiNotification::PopupNotification&");
    qRegisterMetaType<UkuiNotification::ActionList>("ActionList");
    qmlRegisterUncreatableType<UkuiNotification::NotificationCloseReason>("org.ukui.notification.client", 1, 0, "NotificationCloseReason", "");
}

NotificationClient::~NotificationClient() = default;

/*!
    \brief 注册为通知服务客户端.

    通过向 DBus总线注册为 DBus 服务，并调用通知服务的 DBus 接口注册为客户端。通知服务通过客户端的 DBus 接口，向客户端发送通知消息。
 */
bool NotificationClient::registerClient()
{
    return d->init();
}

/*!
    \brief 调用此接口关闭通知.

    \a id： 通知ID， \a reason：通知关闭的原因
 */
bool NotificationClient::closeNotification(uint id, NotificationCloseReason::CloseReason reason)
{
    return d->closeNotification(id, reason);
}

/*!
    \brief 调用通知对应的 Action

    \a id： 通知ID， \a action_key：Action对应的 key, 由通知发送者定义。
 */
bool NotificationClient::invokeAction(uint id, const QString &action_key)
{
    return d->invokeAction(id, action_key);
}

/*!
    \brief 注销通知客户端

    调用此接口注销客户端，注销之后不在接收通知消息
 */
void NotificationClient::unregisterClient()
{
    d->unregisterClient();
}
