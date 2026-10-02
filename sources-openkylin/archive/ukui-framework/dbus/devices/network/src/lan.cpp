/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "lan.h"
#include "network_common.h"

Lan::Lan(QObject *parent) :
    QObject(parent)
{
    QDBusConnection::sessionBus().connect(KYLIN_NM_DBUS_SERVER,
                                              KYLIN_NM_DBUS_PATH,
                                              KYLIN_NM_DBUS_INTERFACE,
                                              "wiredMainSwitchBtnChanged",
                                             this, SIGNAL(EnableChanged(bool)));
}

Lan::~Lan(){

}

QDBusPendingCallWatcher * Lan::__lan_dbus_call(const QString & method, const QList<QVariant> & args)
{
    QDBusInterface iface(KYLIN_NM_DBUS_SERVER, KYLIN_NM_DBUS_PATH, KYLIN_NM_DBUS_INTERFACE, QDBusConnection::sessionBus());
    QDBusPendingCallWatcher * watch = new QDBusPendingCallWatcher(iface.asyncCallWithArgumentList(method, args), this);
    return watch;
}

bool Lan::SetDeviceState(QString device, bool enable){
    Q_UNUSED(device)

    this->setDelayedReply(true);

    auto msg = this->message();
    auto con = this->connection();

    QList<QVariant> args;
    args.append(enable);

    QDBusPendingCallWatcher * watch = this->__lan_dbus_call("setWiredSwitchEnable", args);
    connect(watch, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *self){
        QDBusMessage reply = self->reply();
        if(reply.type() == QDBusMessage::ReplyMessage) {
            auto r = msg.createReply(reply.arguments());
            con.send(r);
        } else {
            auto r = msg.createErrorReply(reply.errorName(), reply.errorMessage());
            con.send(r);
        }
        watch->deleteLater();
    });

    return true;
}

bool Lan::IsEnable()
{
    QDBusReply<bool> reply = callNetworkMethod("getWiredMainSwitchBtnState");
    if (false == reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::ErrorType::Failed,QString("ukui-framework dbus method call failed."));
        }
    }
    return reply.value();
}

QStringList Lan::GetAllDevices(void)
{
    this->setDelayedReply(true);

    auto msg = this->message();
    auto con = this->connection();
    QList<QVariant> args;

    QDBusPendingCallWatcher * watch = this->__lan_dbus_call("getWiredList", args);
    connect(watch, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *self){
        QDBusMessage reply = self->reply();
        if(reply.type() == QDBusMessage::ReplyMessage) {
            QDBusReply<QVariantMap> retMap = reply;
            auto r = msg.createReply(QVariant(retMap.value().keys()));
            con.send(r);
        } else {
            auto r = msg.createErrorReply(reply.errorName(), reply.errorMessage());
            con.send(r);
        }
    });
    return QStringList();
}

void Lan::Enable(bool enable)
{
    QList<QVariant> args;
    args.append(enable);

    this->__lan_dbus_call("setWiredSwitchEnable", args);
}

template<typename ...Args>
QDBusMessage Lan::callNetworkMethod(const QString &method, Args &&...args)
{
     QDBusInterface iface(KYLIN_NM_DBUS_SERVER, KYLIN_NM_DBUS_PATH, KYLIN_NM_DBUS_INTERFACE, QDBusConnection::sessionBus());

     return iface.call(method, std::forward<Args>(args)...);
}
