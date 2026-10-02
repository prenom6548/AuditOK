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

#include "bluetooth.h"
#include "bluetooth_dbussignal_monitor.h"
#include "bluetooth_define.h"

Bluetooth::Bluetooth(QObject *parent /*= nullptr*/):
    QObject(parent)
{
    qDebug() << "init Bluetooth";
    DBUSSINALMONITOR::instance();

    QDBusConnection::sessionBus().connect(UKUIBLUETHHTH_DBUS_SERVER,
                                          UKUIBLUETHHTH_DBUS_OBJECT,
                                          UKUIBLUETHHTH_DBUS_INTERFACE,
                                          "defaultAdapterPowerChanged",
                                         this, SIGNAL(EnableChanged(bool)));
}

 Bluetooth::~Bluetooth(){
    qDebug() << "fini Bluetooth";

    DBUSSINALMONITOR::destroyInstance();
}


bool Bluetooth::SetDeviceState(QString device, bool enable){
    qInfo() << device << enable;
    
    this->setDelayedReply(true);

    auto msg = this->message();
    auto con = this->connection();

    QList<QVariant> args;
    QMap<QString, QVariant> arg;
    arg["Powered"] = enable;
    args.append(arg);

    QDBusPendingCallWatcher * watch = this->__ukuibluetooth_dbus_call("setDefaultAdapterAttr", args);
    connect(watch, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *self){
        QDBusMessage reply = self->reply();
        if(reply.type() == QDBusMessage::ReplyMessage) {
            auto r = msg.createReply(reply.arguments());
            con.send(r);
        } else {
            auto r = msg.createErrorReply(reply.errorName(), reply.errorMessage());
            con.send(r);
        }
    });

    return true;
}

QStringList Bluetooth::GetAllDevices(void)
{
    this->setDelayedReply(true);

    auto msg = this->message();
    auto con = this->connection();
    QList<QVariant> args;

    QDBusPendingCallWatcher * watch = this->__ukuibluetooth_dbus_call("getAllAdapterAddress", args);
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

    return QStringList();
}

void Bluetooth::Enable(bool enable)
{
    callBlueToothDbus("setDefaultAdapterPower", enable);
}

bool Bluetooth::IsEnable()
{
    QDBusReply<bool> reply = callBlueToothDbus("getDefaultAdapterPower");
    if (reply.isValid()) {
         return reply.value();
    }

    return false;
}

QDBusPendingCallWatcher * Bluetooth::__ukuibluetooth_dbus_call(const QString & method, const QList<QVariant> & args)
{
    QDBusInterface iface(UKUIBLUETHHTH_DBUS_SERVER, UKUIBLUETHHTH_DBUS_OBJECT, UKUIBLUETHHTH_DBUS_INTERFACE, QDBusConnection::systemBus());
    QDBusPendingCallWatcher * watch = new QDBusPendingCallWatcher(iface.asyncCallWithArgumentList(method, args), this);
    return watch;
}

template<typename ...Args>
QDBusMessage Bluetooth::callBlueToothDbus(const QString &method, Args &&...args)
{
    QDBusInterface inputManagerInterface(UKUIBLUETHHTH_DBUS_SERVER,
                                         UKUIBLUETHHTH_DBUS_OBJECT,
                                         UKUIBLUETHHTH_DBUS_INTERFACE);

    return inputManagerInterface.call(method, std::forward<Args>(args)...);
}
