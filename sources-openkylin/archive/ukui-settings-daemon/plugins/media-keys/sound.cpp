/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "sound.h"
#include <QVariant>

#include "clib-syslog.h"

Q_GLOBAL_STATIC(Sound, s_sound)
Sound::Sound(QObject *parent) : QObject(parent)
{
    initVolumeDbus();
}

Sound::~Sound()
{
}

Sound* Sound::self()
{
    return s_sound;
}

int Sound::getSinkVolume()
{
    QVariant ret = getValueByDbus("getVolume", Sound::Output, -1);
    return ret.isValid() ? ret.toInt() : 0;
}

bool Sound::getSinkMute()
{
    QVariant ret = getValueByDbus("getMute", Sound::Output, -1);
    return ret.isValid() ? ret.toBool() : false;

}

bool Sound::getSourceMute()
{
    QVariant ret = getValueByDbus("getMute", Sound::Input, -1);
    return ret.isValid() ? ret.toBool() : false;

}

void Sound::setSinkVolume(int volume)
{
    setValueByDbus("setVolume", Sound::Output, -1, volume);
}

void Sound::setSinkMute(bool mute)
{
    setValueByDbus("setMute", Sound::Output, -1, mute);
}

void Sound::setSourceMute(bool mute)
{
    setValueByDbus("setMute", Sound::Input, -1, mute);
}

bool Sound::initSinkSuccess()
{
    return isDbusValid();
}

bool Sound::initSourceSuccess()
{
    return isDbusValid();
}

bool Sound::isExistSink()
{
    QVariant ret = getValueByDbus("isValidDevice", Sound::Output);
    return ret.isValid() ? ret.toBool() : false;
}

bool Sound::isExistSource()
{
    QVariant ret = getValueByDbus("isValidDevice", Sound::Input);
    return ret.isValid() ? ret.toBool() : false;
}

void Sound::initVolumeDbus()
{
    m_volumeInterface = new QDBusInterface("org.ukui.volume.control",
                                           "/org/ukui/volume/control",
                                           "org.ukui.volume.control",
                                           QDBusConnection::sessionBus(), this);

    if (!isDbusValid()) {
        SYS_LOG(LOG_WARNING, "Init ukui volume dbus error!");
    }
}

bool Sound::isDbusValid() const
{
    return m_volumeInterface != nullptr && m_volumeInterface->isValid();
}

template<typename...Args>
QVariant Sound::getValueByDbus(const QString &method, Args &&...args)
{
    if (!isDbusValid() || method.isEmpty()) {
        SYS_LOG(LOG_WARNING, "Get value by dbus error! Method is %s", method.toUtf8().data());
        return QVariant(QVariant::Invalid);
    }

    QDBusMessage result = m_volumeInterface->call(method, std::forward<Args>(args)...);
    if (result.type() != QDBusMessage::ReplyMessage) {
        SYS_LOG(LOG_WARNING, "volume dbus %s reply ERROR! Error msg is %s", method.toUtf8().data(), result.errorMessage().toUtf8().data());
        return QVariant(QVariant::Invalid);
    }

    return result.arguments().takeFirst();
}

template<typename...Args>
void Sound::setValueByDbus(const QString &method, Args &&...args)
{
    if (!isDbusValid() || method.isEmpty() || sizeof...(args) == 0) {
        SYS_LOG(LOG_WARNING, "Set value by dbus error! Method is %s", method.toUtf8().data());
        return;
    }

    m_volumeInterface->asyncCall(method, std::forward<Args>(args)...);
}
