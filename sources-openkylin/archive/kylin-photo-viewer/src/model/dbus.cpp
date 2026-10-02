/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#include "dbus.h"

Dbus::Dbus()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService(Variable::PHOTO_VIEW_DBUS_SERVICENAME)) {
        m_connectSeccess = sessionBus.registerObject(
            Variable::PHOTO_VIEW_DBUS_PARH, Variable::PHOTO_VIEW_DBUS_INTERFACE, this, QDBusConnection::ExportAllSlots);
        // connectSeccess=sessionBus.registerObject(Variable::PHOTO_VIEW_DBUS_PARH,this,QDBusConnection::ExportAllContents);
    }
}

void Dbus::argumentsCommand(const QStringList &arguments)
{
    QStringList cmd = arguments;
    cmd.removeFirst();
    QDBusInterface *interface =
        new QDBusInterface(Variable::PHOTO_VIEW_DBUS_SERVICENAME, Variable::PHOTO_VIEW_DBUS_PARH,
                           Variable::PHOTO_VIEW_DBUS_INTERFACE, QDBusConnection::sessionBus());
    interface->call("getCmdFromOtherMe", cmd);
#ifndef UNUI
    exit(0);
#endif
}

bool Dbus::getConnectSeccess()
{
    return m_connectSeccess;
}

void Dbus::getCmdFromOtherMe(const QStringList &cmd)
{

    Q_EMIT processingCommand(cmd);
    // qDebug()<<"从DBUS接收到命令："<<cmd;
}

void Dbus::fullScreen()
{
    Q_EMIT processingFullScreen();
}
