/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "notification-control.h"

#include "notificationcontroladaptor.h"

#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(notificationLog, "[ukui-framework.notifications]")

namespace {

constexpr const char *NOTIFICATIONS_DBUS_SERVICE = "org.freedesktop.Notifications";
constexpr const char *NOTIFICATIONS_DBUS_PATH = "/org/freedesktop/Notifications";
constexpr const char *NOTIFICATIONS_DBUS_INTERFACE = "org.ukui.NotificationSettings";
constexpr int NOTIFICATIONS_CALL_TIMEOUT_MS = 3000;

} // namespace

NotificationControl::NotificationControl(QObject *parent)
    : QObject(parent)
{
    new NotificationControlAdaptor(this);
    refreshDNDState();
    connectNotificationSignals();
}

void NotificationControl::SetDND(bool enabled)
{
    asyncSetDND(enabled);
}

bool NotificationControl::GetDND() const
{
    return m_isDND;
}

void NotificationControl::onNotificationDNDChanged(bool enabled)
{
    updateDND(enabled);
}

QDBusPendingCall NotificationControl::asyncCallNotificationsService(const QString &method, const QVariantList &args)
{
    QDBusMessage message = QDBusMessage::createMethodCall(NOTIFICATIONS_DBUS_SERVICE,
                                                          NOTIFICATIONS_DBUS_PATH,
                                                          NOTIFICATIONS_DBUS_INTERFACE,
                                                          method);
    message.setArguments(args);
    return QDBusConnection::sessionBus().asyncCall(message, NOTIFICATIONS_CALL_TIMEOUT_MS);
}

void NotificationControl::connectNotificationSignals()
{
    const bool connected = QDBusConnection::sessionBus().connect(
            NOTIFICATIONS_DBUS_SERVICE,
            NOTIFICATIONS_DBUS_PATH,
            NOTIFICATIONS_DBUS_INTERFACE,
            QStringLiteral("DNDChanged"),
            this,
            SLOT(onNotificationDNDChanged(bool)));

    if (!connected) {
        qCWarning(notificationLog) << "Failed to connect DNDChanged signal from notification service";
    }
}

void NotificationControl::asyncSetDND(bool enabled)
{
    auto *watcher = new QDBusPendingCallWatcher(
            asyncCallNotificationsService(QStringLiteral("SetDND"), {enabled}), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        const QDBusPendingReply<> reply = *watcher;
        if (reply.isError()) {
            qCWarning(notificationLog) << "SetDND failed:" << reply.error().message();
            refreshDNDState();
        }
        watcher->deleteLater();
    });
}

void NotificationControl::refreshDNDState()
{
    auto *watcher = new QDBusPendingCallWatcher(
            asyncCallNotificationsService(QStringLiteral("IsDND")), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<bool> reply = *watcher;
        if (reply.isError()) {
            qCWarning(notificationLog) << "IsDND failed:" << reply.error().message();
        } else {
            updateDND(reply.value());
        }
        watcher->deleteLater();
    });
}

void NotificationControl::updateDND(bool enabled)
{
    if (m_isDND == enabled) {
        return;
    }
    qCDebug(notificationLog) << "DND changed:" << enabled;
    m_isDND = enabled;
    Q_EMIT DNDChanged(enabled);
}
