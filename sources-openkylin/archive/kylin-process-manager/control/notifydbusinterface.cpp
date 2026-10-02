/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "notifydbusinterface.h"

#include "loghelper.h"
#include <QDBusReply>
#include <QDebug>

namespace
{
const char *notify_service = "org.freedesktop.Notifications";
const char *notify_path = "/org/freedesktop/Notifications";
const char *notify_interface = "org.freedesktop.Notifications";
}

NotifyDBusInterface::NotifyDBusInterface(QObject *parent)
    : QObject{parent}
    , m_notifyInterface(nullptr)
{
}

void NotifyDBusInterface::initNotifyInterface()
{
    if (m_notifyInterface)
        return;

    m_notifyInterface = new QDBusInterface(notify_service, notify_path, notify_interface,
                                           QDBusConnection::sessionBus(), this);
    if (!m_notifyInterface->isValid()) {
        PRINT_NOTICE("Create Notification service interface failed: %s\n", m_notifyInterface->lastError().message().toStdString().c_str());
        delete m_notifyInterface;
        m_notifyInterface = nullptr;
        return;
    } else {
        PRINT_NOTICE("Create Notification service interface successful.\n");
    }

    if (!initConnections()) {
        PRINT_NOTICE("Connect notification service signals failed\n");
        delete m_notifyInterface;
        m_notifyInterface = nullptr;
        return;
    }
}

bool NotifyDBusInterface::initConnections()
{
    if (!connect(m_notifyInterface, SIGNAL(NotificationClosed(uint, uint)),
                 this, SLOT(handleNotifyClose(uint, uint)))) {
        return false;
    }

    if (!connect(m_notifyInterface, SIGNAL(ActionInvoked(uint, QString)),
                 this, SLOT(handleActionInvoked(uint, QString)))) {
        return false;
    }

    return true;
}

void NotifyDBusInterface::sendNotificationAsync(
    const QString &summary, const QString &body, const QStringList &actions,
    const QVariantMap &hints, uint replace, SendNotificationCallback callback)
{
    if (!m_notifyInterface) {
        initNotifyInterface();

        if (!m_notifyInterface) {
            PRINT_NOTICE("Init Notification service interface failed, can't send notification\n");
            if (callback)
                callback(-1, false);
            return;
        }
    }

    PRINT_NOTICE("Send system notify: %s\n", body.toStdString().c_str());
    const QString app_name = QObject::tr("Settings");
    const QString app_icon = "ukui-control-center";
    const int expire_timeout = 0;

    QDBusPendingCall call =
        m_notifyInterface->asyncCall("Notify", app_name, replace, app_icon,
                                     summary, body, actions, hints,
                                     expire_timeout);
    if (call.isError() || call.isValid()) {
        // qWarning() << "Send system notify failed:"
        //            << call.error().name() << call.error().message();
        PRINT_NOTICE("Send system notify failed: %s %s\n", call.error().name().toStdString().c_str(), call.error().message().toStdString().c_str());
        if (callback)
            callback(-1, false);
        return;
    }

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished,
                     this, [callback](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<uint> reply = *watcher;
        if (reply.isError()) {
            // qWarning() << "Send system notify failed:"
            //            << reply.error().name() << reply.error().message();
            PRINT_NOTICE("Send system notify failed: %s %s\n", reply.error().name().toStdString().c_str(), reply.error().message().toStdString().c_str());
            if (callback)
                callback(-1, false);
            watcher->deleteLater();
            return;
        }

        if (callback) {
            callback(reply.value(), true);
        }
        watcher->deleteLater();
    });
}

void NotifyDBusInterface::setNotifyCloseCallback(NotifyCloseCallback callback)
{
    m_notifyCloseCallbacks.emplace_back(std::move(callback));
}

void NotifyDBusInterface::handleNotifyClose(uint notifyId, uint reason)
{
    for (auto &callback : m_notifyCloseCallbacks) {
        callback(notifyId, reason);
    }
}

void NotifyDBusInterface::setActionInvokedCallback(ActionInvokedCallback callback)
{
    m_actionInvokedCallbacks.emplace_back(std::move(callback));
}

void NotifyDBusInterface::handleActionInvoked(uint notifyId, const QString &actionKey)
{
    for (auto &callback : m_actionInvokedCallbacks) {
        callback(notifyId, actionKey);
    }
}
