/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#include "lockinfo.h"

#include <QDBusInterface>
#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <glib.h>

QDBusArgument &InhibitInfo::operator<<(QDBusArgument &argument, const InhibitInfo::InhibitorInfo &mystruct)
{
    argument.beginStructure();
    argument << mystruct.name << mystruct.icon;
    argument.endStructure();
    return argument;
}

const QDBusArgument &InhibitInfo::operator>>(const QDBusArgument &argument, InhibitInfo::InhibitorInfo &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.name >> mystruct.icon ;
    argument.endStructure();
    return argument;
}

lockinfo::lockinfo()
{
}

//QVector<InhibitInfo::InhibitorInfo> lockinfo::listInhibitorInfo(lockinfo::InhibitorType type)
//{
//    QVector<InhibitInfo::InhibitorInfo> result;
//    switch (type) {
//    case InhibitorType::logout: {
//        //getLogoutInhibitor(result);
//        break;
//    }
//    case InhibitorType::suspend: {
//        getSystemdInhibitor(QString("sleep"), result);
//        break;
//    }
//    case InhibitorType::shutdown: {
//        getSystemdInhibitor(QString("shutdown"), result);
//        break;
//    }
//    }

//    return result;
//}

bool lockinfo::getSystemdInhibitor(QString type)
{
    QDBusInterface loginInterface("org.freedesktop.login1", "/org/freedesktop/login1",
                                  "org.freedesktop.login1.Manager", QDBusConnection::systemBus());

    if (loginInterface.isValid()) {
        qDebug() << "create interface success";
    }

    QDBusMessage result = loginInterface.call("ListInhibitors");
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        SystemdInhibitor::Inhibitor inhibitor;
        dbusArgs >> inhibitor;
        qDebug() << inhibitor.mode << inhibitor.action;
        if (inhibitor.mode == QString("block") && inhibitor.action.contains(type)) {
            dbusArgs.endArray();
            return true;
        }
    }
    return false;
    dbusArgs.endArray();
}


QDBusArgument &SystemdInhibitor::operator<<(QDBusArgument &argument, const SystemdInhibitor::Inhibitor &mystruct)
{
    argument.beginStructure();
    argument << mystruct.action << mystruct.name << mystruct.reason << mystruct.mode << mystruct.uid << mystruct.pid;
    argument.endStructure();
    return argument;
}

const QDBusArgument &SystemdInhibitor::operator>>(const QDBusArgument &argument, SystemdInhibitor::Inhibitor &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.action >> mystruct.name >> mystruct.reason >> mystruct.mode >> mystruct.uid >> mystruct.pid;
    argument.endStructure();
    return argument;
}
