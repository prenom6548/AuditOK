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

#include "sidebar-dbus-service.h"
#include "sidebaradaptor.h"

#include <QDBusConnection>
#include <QStringLiteral>
#include <QDebug>

#define SIDEBAR_DBUS_SERVICE          "org.ukui.Sidebar"
#define SIDEBAR_DBUS_PATH             "/org/ukui/Sidebar"
#define SIDEBAR_DBUS_INTERFACE        "org.ukui.Sidebar"

namespace Sidebar {

SidebarDbusService::SidebarDbusService(SidebarMain *parent)
    : QObject(parent), QDBusContext(), m_adaptor(new SidebarAdaptor(this))
{
    QDBusReply<QDBusConnectionInterface::RegisterServiceReply> reply =
        QDBusConnection::sessionBus().interface()->registerService(SIDEBAR_DBUS_SERVICE,
                                                                   QDBusConnectionInterface::ReplaceExistingService,
                                                                   QDBusConnectionInterface::DontAllowReplacement);

    if (reply.value() != QDBusConnectionInterface::ServiceNotRegistered) {
        bool res = QDBusConnection::sessionBus().registerObject(SIDEBAR_DBUS_PATH, this);
        if (!res) {
            QDBusConnection::sessionBus().interface()->unregisterService(SIDEBAR_DBUS_SERVICE);
        }
    }

    connect(parent, &SidebarMain::stateChange, m_adaptor, &SidebarAdaptor::stateChange);
}

SidebarMain *SidebarDbusService::parent()
{
    return qobject_cast<SidebarMain*>(QObject::parent());
}

void SidebarDbusService::sidebarActive()
{
    active(QStringLiteral("sidebar"));
}

void SidebarDbusService::shortcutsActive()
{
    active(QStringLiteral("shortcut"));
}

void SidebarDbusService::active(const QString &module)
{
    // 快捷按钮界面
    if (module == QStringLiteral("shortcut")) {
        parent()->requestShortcut(SidebarMain::Active);

        // PC侧边栏（PC通知页面）
    } else if (module == QStringLiteral("sidebar")) {
        parent()->requestSidebar(SidebarMain::Active);

        // 通知中心
    }  else if (module == QStringLiteral("notice")) {
        parent()->requestNotificationCenter(SidebarMain::Active);
    }
}

void SidebarDbusService::shortcutWidgetActive(const QString &id, bool showReturnButton)
{
    if (id == "") return;
    parent()->requestWidgetActive(id, showReturnButton);
}

} // Sidebar
