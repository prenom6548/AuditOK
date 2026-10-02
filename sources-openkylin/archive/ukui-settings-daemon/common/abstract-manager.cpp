/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDBusConnection>
#include <QDBusInterface>

#include "abstract-manager.h"
#include "clib-syslog.h"
AbstractManager::AbstractManager(QObject *parent) :
    QObject(parent)
{

}

void AbstractManager::sendSessionDbus()
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.gnome.SessionManager",
                                                          "/org/gnome/SessionManager",
                                                          "org.gnome.SessionManager",
                                                          "startupfinished");
    QList<QVariant> args;
    args.append("ukui-settings-daemon");
    args.append("startupfinished");
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}
