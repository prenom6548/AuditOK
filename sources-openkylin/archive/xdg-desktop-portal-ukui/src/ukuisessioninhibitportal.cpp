/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "ukuisessioninhibitportal.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

InhibitPortal::InhibitPortal(QObject *parent) : QDBusAbstractAdaptor(parent)
{
}

void InhibitPortal::Inhibit(const QDBusObjectPath &handle, const QString &app_id, const QString &window, uint flags, const QVariantMap &options)
{
//    qCDebug(XdgDesktopPortalKdeInhibit) << "Inhibit called with parameters:";
//    qCDebug(XdgDesktopPortalKdeInhibit) << "    handle: " << handle.path();
//    qCDebug(XdgDesktopPortalKdeInhibit) << "    app_id: " << app_id;
//    qCDebug(XdgDesktopPortalKdeInhibit) << "    window: " << window;
//    qCDebug(XdgDesktopPortalKdeInhibit) << "    flags: " << flags;
//    qCDebug(XdgDesktopPortalKdeInhibit) << "    options: " << options;

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.gnome.SessionManager"),
                                                          QStringLiteral("/org/gnome/SessionManager"),
                                                          QStringLiteral("org.gnome.SessionManager"),
                                                          QStringLiteral("Inhibit"));
    //         interrupt session (1)
    message << app_id << (uint)1 << options.value(QStringLiteral("reason")).toString() << flags;

    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    connect(watcher, &QDBusPendingCallWatcher::finished, [handle, this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<uint> reply = *watcher;
        if (reply.isError()) {
//            qCDebug(XdgDesktopPortalKdeInhibit) << "Inhibition error: " << reply.error().message();
        } else {
            QDBusConnection sessionBus = QDBusConnection::sessionBus();
            new Request(handle, this, QStringLiteral("org.freedesktop.impl.portal.Inhibit"), QVariant(reply.value()));
        }
    });
}
