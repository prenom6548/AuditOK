/*
 * Copyright (C) 2026 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef DBUSACCESSCONTROL_H
#define DBUSACCESSCONTROL_H

#include <QString>

class QDBusConnection;
class QDBusMessage;

namespace UniauthDBusAccess
{

int callerUid(const QDBusConnection &conn, const QDBusMessage &msg);
QString callerExecutablePath(const QDBusConnection &conn, const QDBusMessage &msg);
bool isCallerRoot(const QDBusConnection &conn, const QDBusMessage &msg);
bool isCallerProcessWhitelisted(const QDBusConnection &conn, const QDBusMessage &msg);

} // namespace UniauthDBusAccess

#endif // DBUSACCESSCONTROL_H
