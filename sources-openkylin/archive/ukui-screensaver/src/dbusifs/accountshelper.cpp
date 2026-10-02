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
#include "accountshelper.h"
#include "../common/definetypes.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QDebug>
#include "../../src/userinfo.h"

AccountsHelper::AccountsHelper(QObject *parent) : QObject(parent)
{
    initData();
    initConnections();
}

AccountsHelper::~AccountsHelper() {}

QString AccountsHelper::getUserLanguageByName(QString strUserName)
{
    QString strLanguage = "";
    if (!m_accountsInterface)
        return strLanguage;
    QDBusReply<QDBusObjectPath> userPath = m_accountsInterface->call("FindUserByName", strUserName);
    if (!userPath.isValid())
        qWarning() << "Get UserPath error:" << userPath.error();
    else {
        QDBusInterface userIface(
            ACCOUNTS_DBUS_SERVICE, userPath.value().path(), FD_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
        QDBusReply<QDBusVariant> languageReply = userIface.call("Get", ACCOUNTS_USER_DBUS_INTERFACE, "Language");
        if (!languageReply.isValid())
            qWarning() << "Get User's language error" << languageReply.error();
        else {
            strLanguage = languageReply.value().variant().toString();
        }
    }
    return strLanguage;
}

QString AccountsHelper::getUserBackgroundByName(QString strUserName)
{
    QString strBackground = "";
    if (!m_accountsInterface)
        return strBackground;
    QDBusReply<QDBusObjectPath> userPath = m_accountsInterface->call("FindUserByName", strUserName);
    if (!userPath.isValid())
        qWarning() << "Get UserPath error:" << userPath.error();
    else {
        QDBusInterface userIface(
            ACCOUNTS_DBUS_SERVICE, userPath.value().path(), FD_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
        QDBusReply<QDBusVariant> backgroundReply
            = userIface.call("Get", ACCOUNTS_USER_DBUS_INTERFACE, "BackgroundFile");
        if (!backgroundReply.isValid())
            qWarning() << "Get User's background error" << backgroundReply.error();
        else {
            strBackground = backgroundReply.value().variant().toString();
        }
    }
    return strBackground;
}

QString AccountsHelper::getUserSessionByName(QString strUserName)
{
    QString strSession = "";
    if (!m_accountsInterface)
        return strSession;
    QDBusReply<QDBusObjectPath> userPath = m_accountsInterface->call("FindUserByName", strUserName);
    if (!userPath.isValid())
        qWarning() << "Get UserPath error:" << userPath.error();
    else {
        QDBusInterface userIface(
            ACCOUNTS_DBUS_SERVICE, userPath.value().path(), FD_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
        QDBusReply<QDBusVariant> sessionReply = userIface.call("Get", ACCOUNTS_USER_DBUS_INTERFACE, "XSession");
        if (!sessionReply.isValid())
            qWarning() << "Get User's xsession error" << sessionReply.error();
        else {
            strSession = sessionReply.value().variant().toString();
        }
    }
    return strSession;
}

QString AccountsHelper::getAccountBackground(uid_t uid)
{
    QString strBackground = "";
    if (!m_accountsInterface)
        return strBackground;
    QDBusReply<QDBusObjectPath> userPath = m_accountsInterface->call("FindUserById", (qint64)uid);
    if (!userPath.isValid())
        qWarning() << "Get UserPath error:" << userPath.error();
    else {
        QDBusInterface userIface(
            ACCOUNTS_DBUS_SERVICE, userPath.value().path(), FD_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
        QDBusReply<QDBusVariant> backgroundReply
            = userIface.call("Get", ACCOUNTS_USER_DBUS_INTERFACE, "BackgroundFile");
        if (!backgroundReply.isValid())
            qWarning() << "Get User's BackgroundFile error" << backgroundReply.error();
        else {
            strBackground = backgroundReply.value().variant().toString();
        }
    }
    return strBackground;
}

void AccountsHelper::initData()
{
    if (!m_accountsInterface) {
        m_accountsInterface = new QDBusInterface(
            ACCOUNTS_DBUS_SERVICE, ACCOUNTS_DBUS_PATH, ACCOUNTS_DBUS_INTERFACE, QDBusConnection::systemBus(), this);
    }
}

void AccountsHelper::initConnections()
{
    connect(m_accountsInterface, SIGNAL(UserAdded(QDBusObjectPath)), this, SIGNAL(UserAdded(QDBusObjectPath)));
    connect(m_accountsInterface, SIGNAL(UserDeleted(QDBusObjectPath)), this, SIGNAL(UserRemoved(QDBusObjectPath)));
}

QStringList AccountsHelper::getUserList()
{
    QStringList listUsers;
    if (!m_accountsInterface)
        return listUsers;
    QDBusMessage ret = m_accountsInterface->call("ListCachedUsers");
    QList<QVariant> outArgs = ret.arguments();
    QVariant first = outArgs.at(0);
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();
    QDBusObjectPath path;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        dbusArgs >> path;
        listUsers << path.path();
    }
    dbusArgs.endArray();
    return listUsers;
}
