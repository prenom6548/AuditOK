/*
 * Copyright © 2016 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Jan Grulich <jgrulich@redhat.com>
 */
/*
 * SPDX-FileCopyrightText: 2016 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
 */

#include "request.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>

Request::Request(const QDBusObjectPath &handle, QObject *parent, const QString &portalName, const QVariant &data)
    : QDBusVirtualObject(parent)
    , m_data(data)
    , m_portalName(portalName)
{
    auto sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerVirtualObject(handle.path(), this, QDBusConnection::VirtualObjectRegisterOption::SubPath)) {
        qDebug() << "request success";
        connect(this, &Request::closeRequested, this, [this, handle]() {
            QDBusConnection::sessionBus().unregisterObject(handle.path());
            deleteLater();
        });
    } else {
        qDebug() << sessionBus.lastError().message();
        qDebug() << "Failed to register request object for" << handle.path();
        deleteLater();
    }
}

Request::~Request()
{
}

bool Request::handleMessage(const QDBusMessage &message, const QDBusConnection &connection)
{
    Q_UNUSED(connection);

    /* Check to make sure we're getting properties on our interface */
    if (message.type() != QDBusMessage::MessageType::MethodCallMessage) {
        return false;
    }

    qDebug() << message.interface();
    qDebug() << message.member();
    qDebug() << message.path();

    if (message.interface() == QLatin1String("org.freedesktop.impl.portal.Request")) {
        if (message.member() == QLatin1String("Close")) {
            if (m_portalName == QLatin1String("org.freedesktop.impl.portal.Inhibit")) {
                QDBusMessage uninhibitMessage = QDBusMessage::createMethodCall(QStringLiteral("org.gnome.SessionManager"),
                                                                               QStringLiteral("/org/gnome/SessionManager"),
                                                                               QStringLiteral("org.gnome.SessionManager"),
                                                                               QStringLiteral("Uninhibit"));

                uninhibitMessage << m_data.toUInt();

                QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(uninhibitMessage);
                QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
                connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, message](QDBusPendingCallWatcher *watcher) {
                    QDBusPendingReply<> reply = *watcher;
                    QDBusMessage messageReply;
                    if (reply.isError()) {
//                        qCDebug(XdgRequestKdeRequest) << "Uninhibit error: " << reply.error().message();
                        messageReply = message.createErrorReply(reply.error());
                    } else {
                        messageReply = message.createReply();
                        Q_EMIT closeRequested();
                    }

                    QDBusConnection::sessionBus().asyncCall(messageReply);
                });
            } else {
                Q_EMIT closeRequested();
                QDBusMessage reply = message.createReply();
                return connection.send(reply);
            }
        }
    }

    return true;
}

QString Request::introspect(const QString &path) const
{
    QString nodes;

    if (path.startsWith(QLatin1String("/org/freedesktop/portal/desktop/request/"))) {
        nodes = QStringLiteral(
            "<interface name=\"org.freedesktop.impl.portal.Request\">"
            "    <method name=\"Close\">"
            "    </method>"
            "</interface>");
    }

//    qCDebug(XdgRequestKdeRequest) << nodes;

    return nodes;
}
