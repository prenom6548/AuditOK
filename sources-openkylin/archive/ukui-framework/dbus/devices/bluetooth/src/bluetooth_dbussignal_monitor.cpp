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

#include "bluetooth_dbussignal_monitor.h"
#include <QDebug>

#include "bluetooth.h"
#include "bluetooth_define.h"

DbusSignalMonitor::DbusSignalMonitor(QObject *parent /*= nullptr*/)
{
    qDebug() << "init DbusSignalMonitor";

    this->init();
}

DbusSignalMonitor::~DbusSignalMonitor()
{
    qDebug() << "fini DbusSignalMonitor";

    this->fini();
}

int DbusSignalMonitor::init(void)
{
    QDBusConnection::systemBus().connect(UKUIBLUETHHTH_DBUS_SERVER, UKUIBLUETHHTH_DBUS_OBJECT,
            UKUIBLUETHHTH_DBUS_INTERFACE, "adapterAddSignal", this,
            SLOT(adapterAddSignal(QMap<QString, QVariant>)));

    QDBusConnection::systemBus().connect(UKUIBLUETHHTH_DBUS_SERVER, UKUIBLUETHHTH_DBUS_OBJECT,
            UKUIBLUETHHTH_DBUS_INTERFACE, "adapterAttrChanged", this,
            SLOT(adapterAttrChanged(QString, QMap<QString, QVariant>)));

    QDBusConnection::systemBus().connect(UKUIBLUETHHTH_DBUS_SERVER, UKUIBLUETHHTH_DBUS_OBJECT,
            UKUIBLUETHHTH_DBUS_INTERFACE, "adapterRemoveSignal", this,
            SLOT(adapterRemoveSignal(QString)));

    return 0;
}

int DbusSignalMonitor::fini(void)
{
    QDBusConnection::systemBus().disconnect(UKUIBLUETHHTH_DBUS_SERVER, UKUIBLUETHHTH_DBUS_OBJECT,
            UKUIBLUETHHTH_DBUS_INTERFACE, "adapterAddSignal", this,
            SLOT(adapterAddSignal(QMap<QString, QVariant>)));
    
    QDBusConnection::systemBus().disconnect(UKUIBLUETHHTH_DBUS_SERVER, UKUIBLUETHHTH_DBUS_OBJECT,
            UKUIBLUETHHTH_DBUS_INTERFACE, "adapterAttrChanged", this,
            SLOT(adapterAttrChanged(QString, QMap<QString, QVariant>)));
    
    QDBusConnection::systemBus().disconnect(UKUIBLUETHHTH_DBUS_SERVER, UKUIBLUETHHTH_DBUS_OBJECT,
            UKUIBLUETHHTH_DBUS_INTERFACE, "adapterRemoveSignal", this,
            SLOT(adapterRemoveSignal(QString)));

    return 0;
}

void DbusSignalMonitor::adapterAddSignal(QMap<QString, QVariant> attrs)
{
    qDebug() << "adapterAddSignal" << attrs;

    QString device;
    bool DefaultAdapter = false;
    bool power = false;

    QString key = "DefaultAdapter";
    if(attrs.contains(key) && attrs[key].type() == QVariant::Bool) {
       DefaultAdapter = attrs[key].toBool();
    }

    key = "Powered";
    if(attrs.contains(key) && attrs[key].type() == QVariant::Bool) {
       power = attrs[key].toBool();
    } else {
        return;
    }

    key = "Addr";
    if(attrs.contains(key) && attrs[key].type() == QVariant::String) {
       device = attrs[key].toString();
    } else {
        return;
    }

    if (DefaultAdapter) {
        m_defaultAdapter = device;
        qDebug() << "send enableChanged" << power;
        emit BLUETOOTHDBUS::getInstance()->EnableChanged(power);
    }

    return;
}

void DbusSignalMonitor::adapterAttrChanged(QString device, QMap<QString, QVariant> attrs)
{
    qDebug() << "adapterAttrChanged" << device << attrs;

    QString key;
    bool DefaultAdapter = false;
    bool power = false;

    if (device == m_defaultAdapter) {
        DefaultAdapter = true;
    }

    key = "DefaultAdapter";
    if(attrs.contains(key) && attrs[key].type() == QVariant::Bool) {
       DefaultAdapter = attrs[key].toBool();
       if (DefaultAdapter) 
            m_defaultAdapter =  device;
        else if ( DefaultAdapter == device) {
            m_defaultAdapter = "";
        }
    }

    key = "Powered";
    if(attrs.contains(key) && attrs[key].type() == QVariant::Bool) {
        power = attrs[key].toBool();
    } else {
        return;
    }

    if (DefaultAdapter) {
        qDebug() << "send enableChanged" << power;
        emit BLUETOOTHDBUS::getInstance()->EnableChanged(power);
        return;
    }

    if (m_defaultAdapter.isEmpty()) {
        QList<QVariant> args;
        args.append(device);
        args.append("");
        QDBusPendingCallWatcher * watch = this->__ukuibluetooth_dbus_call("getAdapterAttr", args);
        connect(watch, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *self){
            QDBusMessage reply = self->reply();
            if(reply.type() == QDBusMessage::ReplyMessage) {
                QMap<QString, QVariant> replyattrs = (QDBusReply<QMap<QString, QVariant>>)reply;
                qDebug() << reply;
                QString key = "DefaultAdapter";
                if(replyattrs.contains(key) && replyattrs[key].type() == QVariant::Bool 
                        && replyattrs[key].toBool()) {
                    qDebug() << "send enableChanged" << power;
                    m_defaultAdapter = device;
                    emit BLUETOOTHDBUS::getInstance()->EnableChanged(power);
                }
                
            } 
        });
    }

    return; 
}

void DbusSignalMonitor::adapterRemoveSignal(QString dev)
{
    qDebug() << "adapterRemoveSignal" << dev;
    if ( m_defaultAdapter == dev) {
        m_defaultAdapter = "";
    }
}

QDBusPendingCallWatcher * DbusSignalMonitor::__ukuibluetooth_dbus_call(const QString & method, const QList<QVariant> & args)
{
    QDBusInterface iface(UKUIBLUETHHTH_DBUS_SERVER, UKUIBLUETHHTH_DBUS_OBJECT, UKUIBLUETHHTH_DBUS_INTERFACE, QDBusConnection::systemBus());
    QDBusPendingCallWatcher * watch = new QDBusPendingCallWatcher(iface.asyncCallWithArgumentList(method, args), this);
    return watch;
}
