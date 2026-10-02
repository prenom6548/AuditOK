/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "dbus-adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class MediaAdaptor
 */

MediaAdaptor::MediaAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

MediaAdaptor::~MediaAdaptor()
{
    // destructor
}

QStringList MediaAdaptor::getAllInputDevices()
{
    // handle method call org.ukui.media.getAllInputDevices
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "getAllInputDevices", Q_RETURN_ARG(QStringList, out0));
    return out0;
}

QStringList MediaAdaptor::getAllOutputDevices()
{
    // handle method call org.ukui.media.getAllOutputDevices
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "getAllOutputDevices", Q_RETURN_ARG(QStringList, out0));
    return out0;
}

QString MediaAdaptor::getDefaultInputDevice()
{
    // handle method call org.ukui.media.getDefaultInputDevice
    QString out0;
    QMetaObject::invokeMethod(parent(), "getDefaultInputDevice", Q_RETURN_ARG(QString, out0));
    return out0;
}

bool MediaAdaptor::getDefaultInputMuteState()
{
    // handle method call org.ukui.media.getDefaultInputMuteState
    bool out0;
    QMetaObject::invokeMethod(parent(), "getDefaultInputMuteState", Q_RETURN_ARG(bool, out0));
    return out0;
}

int MediaAdaptor::getDefaultInputVolume()
{
    // handle method call org.ukui.media.getDefaultInputVolume
    int out0;
    QMetaObject::invokeMethod(parent(), "getDefaultInputVolume", Q_RETURN_ARG(int, out0));
    return out0;
}

QString MediaAdaptor::getDefaultOutputDevice()
{
    // handle method call org.ukui.media.getDefaultOutputDevice
    QString out0;
    QMetaObject::invokeMethod(parent(), "getDefaultOutputDevice", Q_RETURN_ARG(QString, out0));
    return out0;
}

bool MediaAdaptor::getDefaultOutputMuteState()
{
    // handle method call org.ukui.media.getDefaultOutputMuteState
    bool out0;
    QMetaObject::invokeMethod(parent(), "getDefaultOutputMuteState", Q_RETURN_ARG(bool, out0));
    return out0;
}

int MediaAdaptor::getDefaultOutputVolume()
{
    // handle method call org.ukui.media.getDefaultOutputVolume
    int out0;
    QMetaObject::invokeMethod(parent(), "getDefaultOutputVolume", Q_RETURN_ARG(int, out0));
    return out0;
}

bool MediaAdaptor::setDefaultInputDevice(const QString &deviceName)
{
    // handle method call org.ukui.media.setDefaultInputDevice
    bool out0;
    QMetaObject::invokeMethod(parent(), "setDefaultInputDevice", Q_RETURN_ARG(bool, out0), Q_ARG(QString, deviceName));
    return out0;
}

bool MediaAdaptor::setDefaultInputMuteState(bool mute)
{
    // handle method call org.ukui.media.setDefaultInputMuteState
    bool out0;
    QMetaObject::invokeMethod(parent(), "setDefaultInputMuteState", Q_RETURN_ARG(bool, out0), Q_ARG(bool, mute));
    return out0;
}

bool MediaAdaptor::setDefaultInputVolume(int value)
{
    // handle method call org.ukui.media.setDefaultInputVolume
    bool out0;
    QMetaObject::invokeMethod(parent(), "setDefaultInputVolume", Q_RETURN_ARG(bool, out0), Q_ARG(int, value));
    return out0;
}

bool MediaAdaptor::setDefaultOutputDevice(const QString &deviceName)
{
    // handle method call org.ukui.media.setDefaultOutputDevice
    bool out0;
    QMetaObject::invokeMethod(parent(), "setDefaultOutputDevice", Q_RETURN_ARG(bool, out0), Q_ARG(QString, deviceName));
    return out0;
}

bool MediaAdaptor::setDefaultOutputMuteState(bool mute)
{
    // handle method call org.ukui.media.setDefaultOutputMuteState
    bool out0;
    QMetaObject::invokeMethod(parent(), "setDefaultOutputMuteState", Q_RETURN_ARG(bool, out0), Q_ARG(bool, mute));
    return out0;
}

bool MediaAdaptor::setDefaultOutputVolume(int value)
{
    // handle method call org.ukui.media.setDefaultOutputVolume
    bool out0;
    QMetaObject::invokeMethod(parent(), "setDefaultOutputVolume", Q_RETURN_ARG(bool, out0), Q_ARG(int, value));
    return out0;
}

int MediaAdaptor::getBatteryLevel(const QString &deviceName)
{
    // handle method call org.ukui.media.bluetooth.getBatteryLevel
    int out0;
    QMetaObject::invokeMethod(parent(), "getBatteryLevel", Q_RETURN_ARG(int, out0), Q_ARG(QString, deviceName));
    return out0;
}

