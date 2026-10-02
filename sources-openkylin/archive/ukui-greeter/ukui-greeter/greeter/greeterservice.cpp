/*
 * Copyright (C) 2023 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "greeterservice.h"
#include <QDebug>

#include <pwd.h>

#define GREETERDAEMON_DBUS_SERVICE      "org.ukui.GreeterDaemon"
#define GREETERDAEMON_DBUS_PATH         "/org/ukui/GreeterDaemon"
#define GREETERDAEMON_DBUS_INTERFACE    "org.ukui.GreeterDaemon"

/* For the type WillLoginUserInfo */
QDBusArgument &operator<<(QDBusArgument &argument, const WillLoginUserInfo &willLoginUserInfo)
{
    argument.beginStructure();
    argument << willLoginUserInfo.strUserName << willLoginUserInfo.isOneKeyLogin;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, WillLoginUserInfo &willLoginUserInfo)
{
    argument.beginStructure();
    argument >> willLoginUserInfo.strUserName >> willLoginUserInfo.isOneKeyLogin;
    argument.endStructure();
    return argument;
}

QDebug operator <<(QDebug stream, const WillLoginUserInfo &willLoginUserInfo)
{
    stream << "WillLogUser ["
           << willLoginUserInfo.strUserName
           << willLoginUserInfo.isOneKeyLogin
           << "]";
    return stream;
}

GreeterService* GreeterService::m_instance = nullptr;
GreeterService::GreeterService(QObject *parent)
    : QDBusAbstractInterface(GREETERDAEMON_DBUS_SERVICE,
                             GREETERDAEMON_DBUS_PATH,
                             GREETERDAEMON_DBUS_INTERFACE,
                             QDBusConnection::systemBus(),
                             parent)
{
    qRegisterMetaType<WillLoginUserInfo>("WillLoginUserInfo");
    qDBusRegisterMetaType<WillLoginUserInfo>();

    setTimeout(2147483647);
}

GreeterService* GreeterService::instance(QObject *parent)
{
    if (!m_instance) {
        m_instance = new GreeterService(parent);
    }
    return m_instance;
}

void GreeterService::SwitchToUser(QString strUserName)
{
    QDBusReply<int> reply = call(QStringLiteral("SwitchToUser"), strUserName);
    if(!reply.isValid())
    {
        qWarning() << "SwitchToUser error:" << reply.error();
        return ;
    }
    return ;
}

void GreeterService::SwitchToGreeterUser(QString strUserName)
{
    QDBusReply<int> reply = call(QStringLiteral("SwitchToGreeterUser"), strUserName);
    if(!reply.isValid())
    {
        qWarning() << "SwitchToGreeterUser error:" << reply.error();
        return ;
    }
    return ;
}

WillLoginUserInfo GreeterService::getWillSwitchUser()
{
    WillLoginUserInfo willLoginUserInfo;
    QDBusMessage result = call(QStringLiteral("getWillSwitchUser"));
    if(result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getWillSwitchUser error:" << result.errorMessage();
        return willLoginUserInfo;
    }
    if (result.arguments().size() > 0) {
        QDBusArgument argInfo = result.arguments().at(0).value<QDBusArgument>();
        argInfo >> willLoginUserInfo;
    }
    qDebug()<<"getUserInfo:"<<willLoginUserInfo;
    return willLoginUserInfo;
}

bool GreeterService::isUserNameValid(QString strUserName)
{
    struct passwd* userInfo = nullptr;
    userInfo = getpwnam(strUserName.toLatin1().data());
    if (userInfo) {
        return true;
    }
    return false;
}
