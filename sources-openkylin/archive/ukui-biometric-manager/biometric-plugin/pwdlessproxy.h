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
#ifndef PWDLESSPROXY_H
#define PWDLESSPROXY_H

#include <QDBusAbstractInterface>
#include <QStringList>
#include <QList>

#define PWDLESS_DBUS_SERVICE "com.kylin.pwdless.daemon"
#define PWDLESS_DBUS_PATH "/"
#define PWDLESS_DBUS_INTERFACE "com.kylin.pwdless.daemon"

/**
 * @brief DBus代理类，负责调用对应的DBus接口
 */
class PwdlessProxy : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit PwdlessProxy(QObject *parent = nullptr);

    int otpBind(QString user, int flag, QString &qrcode);
    int otpRemove(QString user, QString &msg);
    int otpVerify(QString user, QString token, QString &msg);

    int ukeyBind(QString user, QString pin, QString &msg);
    int ukeyStatus();
    int ukeyUnbind(QString user, QString &msg);
    int ukeyVerify(QString user, QString pin, QString &msg);
    QStringList userAuthMethods(QString user);
    QString mfaStatus();
    int getOtpEmergMsg(QString user, QString &msg);
    bool isExists() ;

public Q_SLOTS:

Q_SIGNALS:
    int auhtFinished();
    QString ukeyInserted(QString user);
    QString ukeyRemoved(QString user);
    void mfaStatusChanged(bool status,QString method);
};

#endif // BIOMETRICPROXY_H
