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

#ifndef UKUI_SIDEBAR_SIDEBAR_DBUS_SERVICE_H
#define UKUI_SIDEBAR_SIDEBAR_DBUS_SERVICE_H

#include <QDBusContext>
#include "sidebar-main.h"

class SidebarAdaptor;

namespace Sidebar {

class SidebarDbusService : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit SidebarDbusService(SidebarMain *parent = nullptr);
    SidebarMain *parent();

public Q_SLOTS:
    void sidebarActive();
    void shortcutsActive();
    void active(const QString &module);
    void shortcutWidgetActive(const QString &id, bool showReturnButton);

private:
    SidebarAdaptor *m_adaptor {nullptr};
};

} // Sidebar

#endif //UKUI_SIDEBAR_SIDEBAR_DBUS_SERVICE_H
