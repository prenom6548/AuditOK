/*
 * Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
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

#include "lockchecker.h"
#include "loginedusers.h"
#include <sys/file.h>
#include <pwd.h>
#include <QString>
#include <QDebug>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusInterface>

#define SYSTEMD_SERVICE     "org.freedesktop.login1"
#define SYSTEMD_PATH        "/org/freedesktop/login1"
#define SYSTEMD_INTERFACE   "org.freedesktop.login1.Manager"

//QDBusArgument &InhibitInfo::operator<<(QDBusArgument &argument, const InhibitInfo::InhibitorInfo &mystruct)
//{
//    argument.beginStructure();
//    argument << mystruct.name << mystruct.icon;
//    argument.endStructure();
//    return argument;
//}

//const QDBusArgument &InhibitInfo::operator>>(const QDBusArgument &argument, InhibitInfo::InhibitorInfo &mystruct)
//{
//    argument.beginStructure();
//    argument >> mystruct.name >> mystruct.icon ;
//    argument.endStructure();
//    return argument;
//}

LockChecker::LockChecker()
{
}

LockChecker::~LockChecker()
{
}

QStringList LockChecker::getLoginedUsers()
{
    QStringList loginedUser;
    qRegisterMetaType<LoginedUsers>("LoginedUsers");
    qDBusRegisterMetaType<LoginedUsers>();
    QDBusInterface loginInterface(SYSTEMD_SERVICE,
                                  SYSTEMD_PATH,
                                  SYSTEMD_INTERFACE,
                                  QDBusConnection::systemBus());

    if (loginInterface.isValid()) {
        qDebug() << "create interface success";
    }

    QDBusMessage result = loginInterface.call("ListUsers");
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();

    QVector<LoginedUsers> loginedUsers;

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        LoginedUsers user;
        dbusArgs >> user;
        if (user.userName == "lightdm") continue;//过滤登录界面的lightdm用户
        loginedUsers.push_back(user);
    }
    dbusArgs.endArray();

    for (LoginedUsers user : loginedUsers) {

        QDBusInterface userPertyInterface("org.freedesktop.login1",
                                          user.objpath.path(),
                                          "org.freedesktop.DBus.Properties",
                                          QDBusConnection::systemBus());

        QDBusReply<QVariant> reply = userPertyInterface.call("Get", "org.freedesktop.login1.User", "State");
        if (reply.isValid()) {
            QString status = reply.value().toString();
            if ("closing" != status) {
                loginedUser.append(user.userName);
            }
        }
    }
    return loginedUser;
}

QVector<InhibitInfo::InhibitorInfo> LockChecker::listInhibitor(QString type)
{
    qDBusRegisterMetaType<InhibitInfo::InhibitorInfo>();

    QVector<InhibitInfo::InhibitorInfo> resVec;

    QDBusInterface loginInterface("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                  "org.gnome.SessionManager", QDBusConnection::sessionBus());

    if (loginInterface.isValid()) {
        qDebug() << "create interface success";
    }

    QDBusMessage result = loginInterface.call("ListInhibitor", QVariant(type));
    if (!result.errorMessage().isEmpty()) {
        if (getInhibitorLocal(resVec, type)) {
            qDebug() << "get inhibitor information from login1";
        }

        return resVec;
    }

    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        InhibitInfo::InhibitorInfo inhibtor;
        dbusArgs >> inhibtor;
        resVec.push_back(inhibtor);
    }

    dbusArgs.endArray();

    return resVec;
}

QString LockChecker::getName(QFile *a)
{
    QString user = getenv("USER");
    if (a->exists()) {
        a->open(QIODevice::ReadOnly|QIODevice::Text);
        QTextStream fileStream(a);
        int k = 0;
        while (!fileStream.atEnd()) {
            QString line = fileStream.readLine();
            if (k == 0) {
                QString a = line;
                qDebug() << "uid="<<a;
                struct passwd *user1;
                user1 = getpwuid(a.toInt());
                qDebug() << "name=" << user1->pw_name << ",uid=" << user1->pw_uid;
                if (user1->pw_name == NULL) {
                    return user;
                }
                user = user1->pw_name;
            }
            k++;
        }
    }
    return user;
}

bool LockChecker::getInhibitorLocal(QVector<InhibitInfo::InhibitorInfo> &inhibitVec, QString type)
{
    if (type == QString("logout")) {
        inhibitVec = Ukuilockinfo::listInhibitorInfo(Ukuilockinfo::InhibitorType::logout);
    } else if (type == QString("sleep")) {
        inhibitVec = Ukuilockinfo::listInhibitorInfo(Ukuilockinfo::InhibitorType::suspend);
    } else if (type == QString("shutdown")) {
        inhibitVec = Ukuilockinfo::listInhibitorInfo(Ukuilockinfo::InhibitorType::shutdown);
    }

    if (inhibitVec.empty()) {
        return false;
    }

    return true;
}

int LockChecker::getCachedUsers()
{
    QDBusInterface loginInterface("org.freedesktop.Accounts",
                                  "/org/freedesktop/Accounts",
                                  "org.freedesktop.Accounts",
                                  QDBusConnection::systemBus());

    if (loginInterface.isValid()) {
        qDebug() << "create interface success";
    }

    QDBusMessage ret = loginInterface.call("ListCachedUsers");
    QList<QVariant> outArgs = ret.arguments();
    QVariant first = outArgs.at(0);
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();
    QDBusObjectPath path;
    dbusArgs.beginArray();
    int userNum = 0;
    while (!dbusArgs.atEnd()) {
        dbusArgs >> path;
        userNum++;
    }
    dbusArgs.endArray();
    qDebug() << userNum;

    return userNum;
}
