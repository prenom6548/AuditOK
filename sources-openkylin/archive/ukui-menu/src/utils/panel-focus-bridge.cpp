/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#include "panel-focus-bridge.h"

#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

namespace UkuiMenu {

namespace {

constexpr auto kMenuFocusReturnKey = "org.ukui.panel.menu.window";

#define UKUI_PANEL_DBUS_SERVICE        "org.ukui.panel"
#define UKUI_PANEL_DBUS_PATH           "/panel"
#define UKUI_PANEL_DBUS_INTERFACE      "org.ukui.panel"

QDBusMessage makePanelFocusMessage(const QString &method, const QString &externalKey)
{
    QDBusMessage message = QDBusMessage::createMethodCall(UKUI_PANEL_DBUS_SERVICE,
                                                          UKUI_PANEL_DBUS_PATH,
                                                          UKUI_PANEL_DBUS_INTERFACE,
                                                          method);
    message << externalKey;
    return message;
}

QDBusPendingCall asyncCallPanelFocusMethod(const QString &method, const QString &externalKey)
{
    return QDBusConnection::sessionBus().asyncCall(makePanelFocusMessage(method, externalKey));
}

} // namespace

PanelFocusBridge *PanelFocusBridge::instance()
{
    static PanelFocusBridge instance(nullptr);
    return &instance;
}

PanelFocusBridge::PanelFocusBridge(QObject *parent) : QObject(parent)
{
}

void PanelFocusBridge::scheduleRestorePanelFocusOnHide()
{
    m_restorePanelFocusOnNextHide = true;
}

void PanelFocusBridge::handleMenuHidden()
{
    if (m_restorePanelFocusOnNextHide) {
        requestPanelFocusRestore();
        return;
    }

    clearPanelFocusReturn();
}

void PanelFocusBridge::requestPanelFocusRestore()
{
    m_restorePanelFocusOnNextHide = false;
    auto *watcher = new QDBusPendingCallWatcher(asyncCallPanelFocusMethod(QStringLiteral("restoreFocusByExternalKey"), QString::fromLatin1(kMenuFocusReturnKey)), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        const QDBusPendingReply<bool> reply(*watcher);
        watcher->deleteLater();

        if (!reply.isError() && reply.argumentAt<0>()) {
            return;
        }

        clearPanelFocusReturn();
    });
}

void PanelFocusBridge::clearPanelFocusReturn()
{
    asyncCallPanelFocusMethod(QStringLiteral("clearFocusReturnByExternalKey"),
                              QString::fromLatin1(kMenuFocusReturnKey));
}

} // namespace UkuiMenu
