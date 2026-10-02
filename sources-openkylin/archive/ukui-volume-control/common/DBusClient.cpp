/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "DBusClient.h"

#include <QDebug>

//#include "ClientMethod.h"

DBusClient& DBusClient::getInstance()
{
    static DBusClient instance;
    return instance;
}

void DBusClient::initDbusConnect()
{
    if (!QDBusConnection::sessionBus().connect(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, AUDIO_INTERFACE,
                                              "volumeChanged",
                                              this, SLOT(volumeChangedSlots(int, int, const QDBusVariant&)))) {
        qDebug() << "Audio framwork interface error, connect " << "volumeChanged" << "failed!";
        qDebug() << QDBusConnection::sessionBus().lastError().message();
    }

    if (!QDBusConnection::sessionBus().connect(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, AUDIO_INTERFACE,
                                              "muteChanged",
                                              this, SLOT(muteChangedSlots(int, int, bool)))) {
        qDebug() << "Audio framwork interface error, connect " << "muteChanged" << "failed!";
        qDebug() << QDBusConnection::sessionBus().lastError().message();
    }

    if (!QDBusConnection::sessionBus().connect(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, AUDIO_INTERFACE,
                                              "volumeChanged",
                                              this, SLOT(volumeChangedSlots(int, int, const QDBusVariant&)))) {
        qDebug() << "Audio framwork interface error, connect " << "volumeChanged" << "failed!";
        qDebug() << QDBusConnection::sessionBus().lastError().message();
    }

    if (!QDBusConnection::sessionBus().connect(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, AUDIO_INTERFACE,
                                              "deviceChanged",
                                              this, SLOT(deviceChangedSlots(int, const QString&, const QString&)))) {
        qDebug() << "Audio framwork interface error, connect " << "deviceChanged" << "failed!";
        qDebug() << QDBusConnection::sessionBus().lastError().message();
    }

    if (!QDBusConnection::sessionBus().connect(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, AUDIO_INTERFACE,
                                              "deviceAdjust",
                                              this, SLOT(deviceAdjustSlots(int)))) {
        qDebug() << "Audio framwork interface error, connect " << "deviceChanged" << "failed!";
        qDebug() << QDBusConnection::sessionBus().lastError().message();
    }

    if (!QDBusConnection::sessionBus().connect(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, SETTINGS_INTERFACE,
                                              "changed",
                                              this, SLOT(settingsChangedSlots(const QString&, const QDBusVariant&)))) {
        qDebug() << "Audio framwork interface error, connect " << "changed" << "failed!" ;
        qDebug() << QDBusConnection::sessionBus().lastError().message();
    }

    if (!QDBusConnection::sessionBus().connect(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, AUDIO_INTERFACE,
                                              "addStream",
                                              this, SLOT(addStreamSlots(int, int, int, bool, const QString&, const QString&)))) {
        qDebug() << "Audio framwork interface error, connect " << "addStream" << "failed!" ;
        qDebug() << QDBusConnection::sessionBus().lastError().message();
    }

    if (!QDBusConnection::sessionBus().connect(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, AUDIO_INTERFACE,
                                              "removeStream",
                                              this, SLOT(removeStreamSlots(int, int)))) {
        qDebug() << "Audio framwork interface error, connect " << "removeStream" << "failed!" ;
        qDebug() << QDBusConnection::sessionBus().lastError().message();
    }
}

void DBusClient::volumeChangedSlots(int type, int idx, const QDBusVariant& v)
{
    Q_EMIT volumeChangedSignal(type, idx, v);
}

void DBusClient::muteChangedSlots(int type, int idx, bool mute)
{
    Q_EMIT muteChangedSignal(type, idx, mute);
}

void DBusClient::deviceChangedSlots(int type, const QString& portName, const QString& cardName)
{
    Q_EMIT deviceChangedSignal(type, portName, cardName);
}

void DBusClient::deviceAdjustSlots(int type)
{
    Q_EMIT deviceAdjustSignal(type);
}

void DBusClient::settingsChangedSlots(const QString& key, const QDBusVariant& v)
{
    qDebug() << "DBusClient::settingsChangedSlots, key:" << key << " value: " << v.variant();
    Q_EMIT settingsChangedSignal(key, v);
}

void DBusClient::addStreamSlots(int dire, int idx, int value, bool mute, const QString& iconName, const QString& descName)
{
    Q_EMIT addStreamSignal(dire, idx, value, mute, iconName, descName);
}

void DBusClient::removeStreamSlots(int dire, int idx)
{
    Q_EMIT removeStreamSignal(dire, idx);
}
