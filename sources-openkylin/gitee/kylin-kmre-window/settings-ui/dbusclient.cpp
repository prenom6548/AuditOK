/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbusclient.h"

#include "syslog.h"
DbusClient::DbusClient(QObject *parent)
    : QObject(parent)
    , mSystemBusInterface(nullptr)
    , mSessionBusInterface(nullptr)
    , mPrefBusInterface(nullptr)
    , mSougouInterfaceDbus(nullptr)
{

}

DbusClient::~DbusClient()
{
    if (mSystemBusInterface) {
        delete mSystemBusInterface;
        mSystemBusInterface = nullptr;
    }

    if (mSessionBusInterface) {
        delete mSessionBusInterface;
        mSessionBusInterface = nullptr;
    }

    if (mPrefBusInterface) {
        delete mPrefBusInterface;
        mPrefBusInterface = nullptr;
    }
}

void DbusClient::initDbusData()
{
    //syslog(LOG_ERR, "DbusClient: create system dbus interface.");
    mSystemBusInterface = new QDBusInterface("cn.kylinos.Kmre",
                                             "/cn/kylinos/Kmre",
                                             "cn.kylinos.Kmre",
                                             QDBusConnection::systemBus());
    if (!mSystemBusInterface->isValid()) {
        syslog(LOG_ERR, "DbusClient: system dbus interface is invalid.");
    }
    QObject::connect(mSystemBusInterface, SIGNAL(Stopped(QString)), this, SIGNAL(dockerStopped(QString)));

    mSessionBusInterface = new QDBusInterface("cn.kylinos.Kmre.Manager",
                                              "/cn/kylinos/Kmre/Manager",
                                              "cn.kylinos.Kmre.Manager",
                                              QDBusConnection::sessionBus());
    if (!mSessionBusInterface->isValid()) {
        syslog(LOG_ERR, "DbusClient: session dbus interface is invalid.");
    }
    
    mPrefBusInterface = new QDBusInterface("cn.kylinos.Kmre.Pref",
                                           "/cn/kylinos/Kmre/Pref",
                                           "cn.kylinos.Kmre.Pref",
                                              QDBusConnection::systemBus());
    if (!mPrefBusInterface->isValid()) {
        syslog(LOG_ERR, "DbusClient: pref system dbus interface is invalid.");
    }
}

void DbusClient::getSystemProp(int type, const QString &field)
{
    QString value;
    QDBusMessage response = mSessionBusInterface->call("getSystemProp", type, field);
    if (response.type() == QDBusMessage::ReplyMessage) {
        value = response.arguments().takeFirst().toString();
        emit systemprop(value);
    }
}
