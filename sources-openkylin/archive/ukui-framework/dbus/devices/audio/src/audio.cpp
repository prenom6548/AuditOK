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

#include "audio.h"
#include "audio_define.h"

Audio::Audio(QObject *parent):
    QObject(parent)
{
    qInfo() << "Initializing Audio";
    initData();
}

Audio::~Audio()
{

}

void Audio::initData()
{
    QDBusReply<int> reply = dbusMethodCall(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, AUDIO_INTERFACE,
                                           "getVolume",
                                           NodeType::OUTPUT, -1);

    if (reply.isValid()) {
        qInfo() << __func__ << " is volume control dbus client.";
        m_isVolumeControlDBusClient = true;
    } else {
        qInfo() << __func__ << " is media dbus client.";
        m_isVolumeControlDBusClient = false;
    }
}

void Audio::initSignals()
{
    //    QDBusConnection::sessionBus().connect(UKUI_MEDIA_SERVICE,
    //                                          UKUI_MEDIA_CONTROL_PATH,
    //                                          QString(),
    //                                          "updateVolume",
    //                                          this,
    //                                          SIGNAL(sinkVolumeChanged(int)));

    //    QDBusConnection::sessionBus().connect(UKUI_MEDIA_SERVICE,
    //                                          UKUI_MEDIA_CONTROL_PATH,
    //                                          QString(),
    //                                          "updateMute",
    //                                          this,
    //                                          SIGNAL(sinkMuteChanged(bool)));

    //    QDBusConnection::sessionBus().connect(UKUI_MEDIA_SERVICE,
    //                                          UKUI_MEDIA_CONTROL_PATH,
    //                                          QString(),
    //                                          "updateDevice",
    //                                          this,
    //                                          SIGNAL(sinkDeviceChanged(QString)));

    //    QDBusConnection::sessionBus().connect(UKUI_MEDIA_SERVICE,
    //                                          UKUI_MEDIA_CONTROL_PATH,
    //                                          QString(),
    //                                          "updateSourceVolume",
    //                                          this,
    //                                          SIGNAL(sourceVolumeChanged(int)));

    //    QDBusConnection::sessionBus().connect(UKUI_MEDIA_SERVICE,
    //                                          UKUI_MEDIA_CONTROL_PATH,
    //                                          QString(),
    //                                          "updateSourceMute",
    //                                          this,
    //                                          SIGNAL(sourceMuteChanged(bool)));

    //    QDBusConnection::sessionBus().connect(UKUI_MEDIA_SERVICE,
    //                                          UKUI_MEDIA_CONTROL_PATH,
    //                                          QString(),
    //                                          "updateSourceDevice",
    //                                          this,
    //                                          SIGNAL(sourceDeviceChanged(QString)));
}

int Audio::getSinkVolume()
{
    QDBusReply<int> reply;
    if (m_isVolumeControlDBusClient) {
        reply = dbusMethodCall(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, AUDIO_INTERFACE,
                               "getVolume",
                               NodeType::OUTPUT, -1);
    } else {
        reply = dbusMethodCall(UKUI_MEDIA_SERVICE, UKUI_MEDIA_PATH, UKUI_MEDIA_INTERFACE,
                               "getDefaultOutputVolume");
    }

    if (!reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(reply.error().type(), reply.error().message());
        }
        return -1;
    }

    return reply.value();
}

bool Audio::getSinkMute()
{
    QDBusReply<bool> reply;
    if (m_isVolumeControlDBusClient) {
        reply = dbusMethodCall(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, AUDIO_INTERFACE,
                               "getMute",
                               NodeType::OUTPUT, -1);
    } else {
        reply = dbusMethodCall(UKUI_MEDIA_SERVICE, UKUI_MEDIA_PATH, UKUI_MEDIA_INTERFACE,
                               "getDefaultOutputMuteState");
    }

    if (!reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(reply.error().type(), reply.error().message());
        }
        return false;
    }

    return reply.value();
}

QString Audio::getSinkDevice()
{
    if (m_isVolumeControlDBusClient) {
        QDBusMessage reply = dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                            AUDIO_OBJECT_PATH,
                                            AUDIO_INTERFACE,
                                            "getDefaultDevice",
                                            NodeType::OUTPUT, -1);
        if (reply.arguments().isEmpty()) {
            return "NULL";
        }

        const QDBusArgument& dbusArgs = reply.arguments().at(0).value<QDBusArgument>();
        ClientDefaultDeviceInfo info;
        dbusArgs >> info;
        return info.cardName;
    } else {
        QDBusReply<QString> reply = dbusMethodCall(UKUI_MEDIA_SERVICE, UKUI_MEDIA_PATH, UKUI_MEDIA_INTERFACE,
                                                   "getDefaultOutputDevice");

        if (!reply.isValid()) {
            if (calledFromDBus()) {
            sendErrorReply(reply.error().type(), reply.error().message());
            }
            return "NULL";
        }

        return reply.value();
    }

    return "NULL";
}

int Audio::getSourceVolume()
{
    QDBusReply<int> reply;
    if (m_isVolumeControlDBusClient) {
        reply = dbusMethodCall(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, AUDIO_INTERFACE,
                               "getVolume",
                               NodeType::INPUT, -1);
    } else {
        reply = dbusMethodCall(UKUI_MEDIA_SERVICE, UKUI_MEDIA_PATH, UKUI_MEDIA_INTERFACE,
                               "getDefaultInputVolume");
    }

    if (!reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(reply.error().type(), reply.error().message());
        }
        return -1;
    }

    return reply.value();
}

bool Audio::getSourceMute()
{
    QDBusReply<bool> reply;
    if (m_isVolumeControlDBusClient) {
        reply = dbusMethodCall(AUDIO_OBJECT_SERVICE, AUDIO_OBJECT_PATH, AUDIO_INTERFACE,
                               "getMute",
                               NodeType::INPUT, -1);
    } else {
        reply = dbusMethodCall(UKUI_MEDIA_SERVICE, UKUI_MEDIA_PATH, UKUI_MEDIA_INTERFACE,
                               "getDefaultInputMuteState");
    }

    if (!reply.isValid()) {
        if (calledFromDBus()) {
        sendErrorReply(reply.error().type(), reply.error().message());
        }
        return false;
    }

    return reply.value();
}

QString Audio::getSourceDevice()
{
    if (m_isVolumeControlDBusClient) {
        QDBusMessage reply = dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                            AUDIO_OBJECT_PATH,
                                            AUDIO_INTERFACE,
                                            "getDefaultDevice",
                                            NodeType::INPUT, -1);
        if (reply.arguments().isEmpty()) {
            return "NULL";
        }

        const QDBusArgument& dbusArgs = reply.arguments().at(0).value<QDBusArgument>();
        ClientDefaultDeviceInfo info;
        dbusArgs >> info;
        return info.cardName;
    } else {
        QDBusReply<QString> reply = dbusMethodCall(UKUI_MEDIA_SERVICE, UKUI_MEDIA_PATH, UKUI_MEDIA_INTERFACE,
                                                   "getDefaultInputDevice");

        if (!reply.isValid()) {
            if (calledFromDBus()) {
            sendErrorReply(reply.error().type(), reply.error().message());
            }
            return "NULL";
        }

        return reply.value();
    }

    return "NULL";
}

bool Audio::setSinkVolume(int volume)
{
    if (m_isVolumeControlDBusClient) {
        dbusMethodCall(AUDIO_OBJECT_SERVICE,
                       AUDIO_OBJECT_PATH,
                       AUDIO_INTERFACE,
                       "setVolume",
                       NodeType::OUTPUT, -1, volume);

        return true;
    } else {
        QDBusReply<bool> reply = dbusMethodCall(UKUI_MEDIA_SERVICE, UKUI_MEDIA_PATH, UKUI_MEDIA_INTERFACE,
                                                "setDefaultOutputVolume", volume);

        if (!reply.isValid()) {
            if (calledFromDBus()) {
            sendErrorReply(reply.error().type(), reply.error().message());
            }
            return false;
        }

        return reply.value();
    }

    return false;
}

bool Audio::setSinkMute(bool mute)
{
    if (m_isVolumeControlDBusClient) {
        dbusMethodCall(AUDIO_OBJECT_SERVICE,
                       AUDIO_OBJECT_PATH,
                       AUDIO_INTERFACE,
                       "setMute",
                       NodeType::OUTPUT, -1, mute);

        return true;
    } else {
        QDBusReply<bool> reply = dbusMethodCall(UKUI_MEDIA_SERVICE, UKUI_MEDIA_PATH, UKUI_MEDIA_INTERFACE,
                                                "setDefaultOutputMuteState", mute);

        if (!reply.isValid()) {
            if (calledFromDBus()) {
            sendErrorReply(reply.error().type(), reply.error().message());
            }
            return false;
        }

        return reply.value();
    }

    return false;
}

bool Audio::setSinkDevice(QString cardName, QString portName)
{
    if (m_isVolumeControlDBusClient) {
        dbusMethodCall(AUDIO_OBJECT_SERVICE,
                       AUDIO_OBJECT_PATH,
                       AUDIO_INTERFACE,
                       "setDefaultDevice",
                       NodeType::OUTPUT, -1, portName, cardName);
        return true;

    } else {
        QDBusReply<bool> reply = dbusMethodCall(UKUI_MEDIA_SERVICE, UKUI_MEDIA_PATH, UKUI_MEDIA_INTERFACE,
                                                "setDefaultOutputDevice", cardName);

        if (!reply.isValid()) {
            if (calledFromDBus()) {
            sendErrorReply(reply.error().type(), reply.error().message());
            }
            return false;
        }

        return reply.value();
    }

    return false;
}

bool Audio::setSourceVolume(int volume)
{
    if (m_isVolumeControlDBusClient) {
        dbusMethodCall(AUDIO_OBJECT_SERVICE,
                       AUDIO_OBJECT_PATH,
                       AUDIO_INTERFACE,
                       "setVolume",
                       NodeType::INPUT, -1, volume);
        return true;
    } else {
        QDBusReply<bool> reply = dbusMethodCall(UKUI_MEDIA_SERVICE, UKUI_MEDIA_PATH, UKUI_MEDIA_INTERFACE,
                                                "setDefaultInputVolume", volume);

        if (!reply.isValid()) {
            if (calledFromDBus()) {
            sendErrorReply(reply.error().type(), reply.error().message());
            }
            return false;
        }

        return reply.value();
    }

    return false;
}

bool Audio::setSourceMute(bool mute)
{
    if (m_isVolumeControlDBusClient) {
        dbusMethodCall(AUDIO_OBJECT_SERVICE,
                       AUDIO_OBJECT_PATH,
                       AUDIO_INTERFACE,
                       "setMute",
                       NodeType::INPUT, -1, mute);
        return true;
    } else {
        QDBusReply<bool> reply = dbusMethodCall(UKUI_MEDIA_SERVICE, UKUI_MEDIA_PATH, UKUI_MEDIA_INTERFACE,
                                                "setDefaultInputMuteState", mute);

        if (!reply.isValid()) {
            if (calledFromDBus()) {
            sendErrorReply(reply.error().type(), reply.error().message());
            }
            return false;
        }

        return reply.value();
    }

    return false;
}

bool Audio::setSourceDevice(QString cardName, QString portName)
{
    if (m_isVolumeControlDBusClient) {
        dbusMethodCall(AUDIO_OBJECT_SERVICE,
                       AUDIO_OBJECT_PATH,
                       AUDIO_INTERFACE,
                       "setDefaultDevice",
                       NodeType::INPUT, -1, portName, cardName);

        return true;
    } else {
        QDBusReply<bool> reply = dbusMethodCall(UKUI_MEDIA_SERVICE, UKUI_MEDIA_PATH, UKUI_MEDIA_INTERFACE,
                                                "setDefaultInputDevice", cardName);

        if (!reply.isValid()) {
            if (calledFromDBus()) {
            sendErrorReply(reply.error().type(), reply.error().message());
            }
            return false;
        }

        return reply.value();
    }

    return false;
}

template <typename... Args>
QDBusMessage Audio::dbusMethodCall(const QString& destination, const QString& path, const QString& interface, const QString& method, Args&&... args)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        QDBusError err = bus.lastError();
        qWarning() << QString("[DBusCall] Session bus disconnected! dest=%1, method=%2, error=%3")
                      .arg(destination, method, err.message());
        return QDBusMessage::createError(err);
    }

    QDBusInterface mediaInterface(destination, path, interface, bus);
    if (!mediaInterface.isValid()) {
        QDBusError err = mediaInterface.lastError();
        qWarning() << QString("[DBusCall] Invalid interface! dest=%1, path=%2, iface=%3, error=%4")
                      .arg(destination, path, interface, err.message());
        return QDBusMessage::createError(err);
    }

    QDBusMessage reply = mediaInterface.call(method, std::forward<Args>(args)...);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << QString("[DBusCall] Method failed! dest=%1, method=%2, errName=%3, errMsg=%4")
                      .arg(destination, method, reply.errorName(), reply.errorMessage());
        if (calledFromDBus()) {
        sendErrorReply(reply.errorName(), reply.errorMessage());
        }
    }

    return reply;
}
