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
#include "ukui_media_control_led.h"
#include <QFileInfo>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QProcess>
#include <QDir>

UkuiMediaControlLed::UkuiMediaControlLed()
{
    QDBusConnection::systemBus().connect(QString(),"/","org.ukui.media","systemOutputVolumeIsMute",this,SLOT(outputMuteChanged(QString)));
    QDBusConnection::systemBus().connect(QString(),"/","org.ukui.media","systemInputVolumeIsMute",this,SLOT(inputMuteChanged(QString)));
}

void UkuiMediaControlLed::outputMuteChanged(QString str)
{
    QFile file("/sys/class/leds/platform::mute/brightness");

    if(!file.exists()){
        qDebug() << " THE DIRECTORY OR FILE IS NOT EXIST ";
        return;
    }

    qDebug() << "sink muted" << str;

    if (strstr(str.toLatin1().data(),"mute")) {
        qDebug() << "输出音量静音";
        system("echo 1 > /sys/class/leds/platform::mute/brightness");
    }
    else if (strstr(str.toLatin1().data(),"no")) {
        qDebug() << "输出音量取消静音";
        system("echo 0 > /sys/class/leds/platform::mute/brightness");
    }
}


void UkuiMediaControlLed::inputMuteChanged(QString str)
{
    QFile file("/sys/class/leds/platform::micmute/brightness");

    if(!file.exists()){
        qDebug() << " THE DIRECTORY OR FILE IS NOT EXIST ";
        return;
    }

    qDebug() << "source muted" << str;

    if (strstr(str.toLatin1().data(),"mute")) {
        qDebug() << "输入音量静音";
        system("echo 1 > /sys/class/leds/platform::micmute/brightness");

    }
    else if (strstr(str.toLatin1().data(),"no")) {
        qDebug() << "输入音量取消静音";
        system("echo 0 > /sys/class/leds/platform::micmute/brightness");
    }
}


UkuiMediaControlLed::~UkuiMediaControlLed()
{

}
