/**
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 **/
#include "pwdlessproxy.h"
#include <QDebug>
#include <QDir>
#include <QDBusReply>
#include <QDebug>
#include <QDBusInterface>

#define SERVICE "pwdless-daemon.service"

PwdlessProxy::PwdlessProxy(QObject *parent)
    : QDBusAbstractInterface(
        PWDLESS_DBUS_SERVICE, PWDLESS_DBUS_PATH, PWDLESS_DBUS_INTERFACE, QDBusConnection::systemBus(), parent)
{
    setTimeout(2147483647);
}

int PwdlessProxy::otpBind(QString user, int flag, QString &qrcode)
{
    QDBusMessage result = call(QStringLiteral("otpBind"), user, flag);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "otpBind error:" << result.errorMessage();
        return -1;
    }
    int ret = result.arguments().at(0).toInt();
    qrcode = result.arguments().at(1).toString();
    return ret;
}

int PwdlessProxy::otpRemove(QString user, QString &msg)
{
    QDBusMessage result = call(QStringLiteral("otpRemove"), user);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "otpRemove error:" << result.errorMessage();
        return -1;
    }
    int ret = result.arguments().at(0).toInt();
    msg = result.arguments().at(1).toString();
    return ret;
}

int PwdlessProxy::otpVerify(QString user, QString token, QString &msg)
{
    QDBusMessage result = call(QStringLiteral("otpVerify"), user, token);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "otpVerify error:" << result.errorMessage();
        return -1;
    }
    int ret = result.arguments().at(0).toInt();
    msg = result.arguments().at(1).toString();
    return ret;
}

int PwdlessProxy::ukeyBind(QString user, QString pin, QString &msg)
{
    QDBusMessage result = call(QStringLiteral("ukeyBind"), user, pin);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "ukeyBind error:" << result.errorMessage();
        return -1;
    }
    int ret = result.arguments().at(0).toInt();
    msg = result.arguments().at(1).toString();
    return ret;
}

int PwdlessProxy::ukeyStatus()
{
    QDBusMessage result = call(QStringLiteral("ukeyStatus"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "ukeyStatus error:" << result.errorMessage();
        return -1;
    }
    int ret = result.arguments().at(0).toInt();
    return ret;
}

int PwdlessProxy::ukeyUnbind(QString user, QString &msg)
{
    QDBusMessage result = call(QStringLiteral("ukeyUnbind"), user);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "ukeyUnbind error:" << result.errorMessage();
        return -1;
    }
    int ret = result.arguments().at(0).toInt();
    msg = result.arguments().at(1).toString();
    return ret;
}

int PwdlessProxy::ukeyVerify(QString user, QString pin, QString &msg)
{
    QDBusMessage result = call(QStringLiteral("ukeyVerify"), user, pin);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "ukeyVerify error:" << result.errorMessage();
        return -1;
    }
    int ret = result.arguments().at(0).toInt();
    msg = result.arguments().at(1).toString();
    return ret;
}

QStringList PwdlessProxy::userAuthMethods(QString user)
{
    QDBusMessage result = call(QStringLiteral("userAuthMethods"), user);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "userAuthMethods error:" << result.errorMessage();
        return QStringList();
    }

    return result.arguments().at(0).toStringList();
}

QString PwdlessProxy::mfaStatus()
{
    QDBusMessage result = call(QStringLiteral("mfaStatus"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "mfaStatus error:" << result.errorMessage();
        return "";
    }
    int ret = result.arguments().at(0).toInt();
    QString method = result.arguments().at(1).toString();

    if (ret == 0)
        return "";
    else
        return method;
}

int PwdlessProxy::getOtpEmergMsg(QString user, QString &msg)
{
    QDBusMessage result = call(QStringLiteral("getOtpEmergMsg"), user);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getOtpEmergMsg error:" << result.errorMessage();
        return -1;
    }
    int ret = result.arguments().at(0).toInt();
    msg = result.arguments().at(1).toString();

    return ret;
}

bool PwdlessProxy::isExists()
{
    QDBusInterface dbusInterface(
        "org.freedesktop.DBus",  // 服务名
        "/org/freedesktop/DBus", // 对象路径
        "org.freedesktop.DBus",  // 接口名
        QDBusConnection::systemBus());

    // 调用 NameHasOwner 方法
    QDBusReply<bool> reply = dbusInterface.call("NameHasOwner", PWDLESS_DBUS_SERVICE);
    return reply.isValid() && reply.value();
}
