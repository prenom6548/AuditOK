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

#include <QDebug>
#include <QDBusError>
#include <QDBusConnection>
#include <QCoreApplication>

#include "test.h"

AudioTest::AudioTest(QObject *obj, QObject *parent):
    QObject(parent),
    m_Obj(obj)
{
    qInfo() << "Initializing AudioTest";
    initSignals();
}

void AudioTest::initSignals()
{
    QObject::connect(m_Obj,SIGNAL(sinkVolumeChanged(int)),this,SLOT(updateSinkVolume(int)));
    QObject::connect(m_Obj,SIGNAL(sinkMuteChanged(bool)),this,SLOT(updateSinkMute(bool)));
    QObject::connect(m_Obj,SIGNAL(sinkDeviceChanged(QString)),this,SLOT(updateSinkDevice(QString)));
    QObject::connect(m_Obj,SIGNAL(sourceVolumeChanged(int)),this,SLOT(updateSourceVolume(int)));
    QObject::connect(m_Obj,SIGNAL(sourceMuteChanged(bool)),this,SLOT(updateSourceMute(bool)));
    QObject::connect(m_Obj,SIGNAL(sourceDeviceChanged(QString)),this,SLOT(updateSourceDevice(QString)));
}

int AudioTest::paVolumeToValue(int volume)
{
    return (volume / PA_VOLUME_NORMAL * UKMEDIA_VOLUME_NORMAL) + 0.5;
}

void AudioTest::updateSinkVolume(int volume)
{
    qInfo() << __func__ << paVolumeToValue(volume);
}

void AudioTest::updateSinkMute(bool mute)
{
    qInfo() << __func__ << mute;
}

void AudioTest::updateSinkDevice(QString device)
{
    qInfo() << __func__ << device;
}

void AudioTest::updateSourceVolume(int volume)
{
    qInfo() << __func__ << paVolumeToValue(volume);
}

void AudioTest::updateSourceMute(bool mute)
{
    qInfo() << __func__ << mute;
}

void AudioTest::updateSourceDevice(QString device)
{
    qInfo() << __func__ << device;
}

AudioTest::~AudioTest()
{
    qInfo() << "~AudioTest";
}


/*
 * 测试情况：
 * getDefaultOutputVolume():通过
 * setDefaultOutputVolume(int volume):通过
 * getDefaultOutputMute():通过
 * setDefaultOutputMute(bool mute):通过
 * getDefaultInputVolume():通过
 * setDefaultInputVolume(int volume):通过
 * getDefaultInputMute():通过
 * setDefaultInputMute(bool mute):通过
 *
*/
int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);
    Audio audio;
    QDBusConnection sessionBug = QDBusConnection::sessionBus();
    if (sessionBug.registerService("org.ukui.mediaTest")) {
        sessionBug.registerObject("/Audio", &audio,
                                 QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals
                                 );
    }

    AudioTest test(&audio);
    app.exec();

    return 0;
}
