/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "freedesktophelper.h"
#include <QDebug>

FreedesktopHelper::FreedesktopHelper(bool bSession, QObject *parent)
    : QDBusAbstractInterface(
        FD_DBUS_SERVICE,
        FD_DBUS_PATH,
        FD_DBUS_INTERFACE,
        (bSession ? QDBusConnection::sessionBus() : QDBusConnection::systemBus()),
        parent)
{
    connect(
        this,
        SIGNAL(NameOwnerChanged(QString, QString, QString)),
        this,
        SLOT(onDBusNameOwnerChanged(QString, QString, QString)));
}

bool FreedesktopHelper::NameHasOwner(const QString &strService)
{
    QDBusReply<bool> ret = call("NameHasOwner", strService);
    if (ret.isValid() && ret.value()) {
        return true;
    }
    return false;
}

bool FreedesktopHelper::isServiceActivable(const QString &strService)
{
    bool isActivable = false;
    QDBusReply<QStringList> result = call(QStringLiteral("ListActivatableNames"));
    if (!result.isValid()) {
        qWarning() << "ListActivatableNames error:" << result.error().message();
    } else {
        QStringList listNames = result.value();
        if (listNames.contains(strService)) {
            isActivable = true;
        }
    }
    return isActivable;
}

void FreedesktopHelper::onDBusNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner);
    Q_EMIT serviceStatusChanged(name, !newOwner.isEmpty());
}
