/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "pwdlessservice.h"
#include <QDBusReply>
#include <QDebug>

#define PWD_DBUS_SERVICE "com.kylin.pwdless.daemon"
#define PWD_DBUS_PATH "/"
#define PWD_DBUS_INTERFACE "com.kylin.pwdless.daemon"

PwdlessService::PwdlessService(QObject *parent)
    :QDBusAbstractInterface(
        PWD_DBUS_SERVICE, PWD_DBUS_PATH, PWD_DBUS_INTERFACE, QDBusConnection::systemBus(), parent)
{
    setTimeout(2147483647);
}

QStringList PwdlessService::GetBoundFactorMethods(QString user)
{
   QDBusReply<QStringList> reply = call(QStringLiteral("userAuthMethods"), user);
   if (!reply.isValid()) {
       qWarning() << "GetPresetSecurityQuestions error: " << reply.error().message();
   }
   return reply;
}

int PwdlessService::GetDualFactorStatus(QString &method)
{
    int result = 0;
    QDBusMessage reply = call(QStringLiteral("mfaStatus"));
    QList<QVariant> varResult = reply.arguments();
    if (varResult.size() > 0) {
        qDebug() << "mfaStatus result = " << varResult.at(0).toInt();
        result = varResult.at(0).toInt();
    }

    if (varResult.size() > 1) {
        qDebug() << "DualFactor method = " << varResult.at(1).toString();
        method = varResult.at(1).toString();
    }
    return result;
}

int PwdlessService::EnableDualFactor(QString meth2)
{
    QDBusReply<int> reply = call(QStringLiteral("mfaEnable"), meth2);
    if (!reply.isValid()) {
        qWarning() << "EnableDualFactor error: " << reply.error().message();
    }
    return reply;
}

int PwdlessService::DisableDualFactor()
{
    QDBusReply<int> reply = call(QStringLiteral("mfaDisable"));
    if (!reply.isValid()) {
        qWarning() << "DisableDualFactor error: " << reply.error().message() << reply.error().type();
        return reply.error().type();
    }
    return reply;
}

bool PwdlessService::GetSshAuthStatus(QString method)
{
    QDBusReply<int> reply = call(QStringLiteral("sshAuthStatus"), method);
    if (!reply.isValid()) {
        qWarning() << "GetSshAuthStatus error: " << reply.error().message();
    }
    return reply == 0 ? false : true;
}

int PwdlessService::SetSshAuthStatus(QString method, int enable)
{
    QDBusReply<int> reply = call(QStringLiteral("sshAuthEnable"), method, enable);
    if (!reply.isValid()) {
        qWarning() << "SetSshAuthStatus error: " << reply.error().message() << reply.error().type();
        return reply.error().type();
    }
    return reply;

}

QString PwdlessService::GetOtpQrCode(QString user, int flag)
{
    QDBusMessage reply = call(QStringLiteral("otpBind"), user, flag);
    QList<QVariant> varResult = reply.arguments();
    bool result = false;
    if (varResult.size() > 0) {
        qDebug() << "otpBind result = " << varResult.at(0).toInt();
        result = varResult.at(0).toInt() == 0 ? true : false;
    }
    QString codeString;
    if (varResult.size() > 1) {
        qDebug() << "otpBind qrcode = " << varResult.at(1).toString();
        codeString = varResult.at(1).toString();
    }
    return codeString;
}

int PwdlessService::BindOtpVerify(QString user, QString token, QString &errorMsg)
{
    QDBusMessage reply = call(QStringLiteral("otpVerify"), user, token);
    QList<QVariant> varResult = reply.arguments();
    qDebug() << __FUNCTION__ << varResult << varResult.size();
    int result = 0;
    if (varResult.size() > 0) {
        qDebug() << "otpVerify result = " << varResult.at(0).toInt();
        result = varResult.at(0).toInt();
    }

    if (varResult.size() > 1) {
        qDebug() << "otpVerify error msg = " << varResult.at(1).toString();
        errorMsg = varResult.at(1).toString();
    }

    return result;
}

int PwdlessService::BindUkeyVerify(QString user, QString pin, QString &errorMsg)
{
    QDBusMessage reply = call(QStringLiteral("ukeyBind"), user, pin);
    QList<QVariant> varResult = reply.arguments();
    qDebug() << __FUNCTION__ << varResult << varResult.size() << reply.type();
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getOtpEmergMsg error:" << reply.errorMessage();
        return -1;
    }
    int result = 0;
    if (varResult.size() > 0) {
        qDebug() << "ukeybind result = " << varResult.at(0).toInt();
        result = varResult.at(0).toInt();
    }

    if (varResult.size() > 1) {
        qDebug() << "ukeybind error msg = " << varResult.at(1).toString();
        errorMsg = varResult.at(1).toString();
    }

    return result;
}

int PwdlessService::UnBindUkey(QString user, QString &errorMsg)
{
    QDBusMessage reply = call(QStringLiteral("ukeyUnbind"), user);
    QList<QVariant> varResult = reply.arguments();
    qDebug() << __FUNCTION__ << varResult << varResult.size();
    int result = 0;
    if (varResult.size() > 0) {
        qDebug() << "ukeybind result = " << varResult.at(0).toInt();
        result = varResult.at(0).toInt();
    }
    if (varResult.size() > 1) {
        qDebug() << "ukeybind error msg = " << varResult.at(1).toString();
        errorMsg = varResult.at(1).toString();
    }
    return result;
}

int PwdlessService::IsInsertUkey()
{
    QDBusReply<int> reply = call(QStringLiteral("ukeyStatus"));
    if (!reply.isValid()) {
        qWarning() << "IsInsertUkey error: " << reply.error().message() << reply.error().type();
        return reply.error().type();
    }

    return reply;
}

int PwdlessService::getOtpEmergMsg(QString user, QString &msg)
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

