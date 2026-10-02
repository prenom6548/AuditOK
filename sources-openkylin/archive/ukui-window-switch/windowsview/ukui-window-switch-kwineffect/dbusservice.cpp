/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "dbusservice.h"

#include "multitaskviewmanager.h"

DBusService::DBusService(MultitaskViewManager *viewManager, QObject *parent)
    : QObject(parent),
      m_viewManager(viewManager),
      m_connection(QDBusConnection::sessionBus())
{
    startService();
}

DBusService::~DBusService()
{
    stopService();
}

bool DBusService::startService()
{
    return m_connection.registerService(m_serviceName)
            && m_connection.registerObject(m_servicePath, this, QDBusConnection::ExportAllSlots);
}

bool DBusService::stopService()
{
    m_connection.unregisterObject(m_servicePath);
    return true;
}

void DBusService::show()
{
    m_viewManager->open();
}

void DBusService::hide()
{
    m_viewManager->close();
}
