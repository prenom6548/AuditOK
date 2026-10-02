/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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
#include <QProcess>
#include "navigations.h"

NavigationUKcc::NavigationUKcc(QObject *parent):
    QObject(parent)
{
     m_isUkccInstall = checkAppExist("/usr/bin/ukui-control-center");
     m_isKyomInstall = checkAppExist("/usr/bin/kylin-os-manager");
     m_isUksmInstall = checkAppExist("/usr/bin/ukui-system-monitor");
     m_isUkclockInstall = checkAppExist("/usr/bin/ukui-clock");
     m_isUkmusicInstall = checkAppExist("/usr/bin/kylin-music");
     m_isUkvideroInstall = checkAppExist("/usr/bin/kylin-video");

    qDebug()<<"init NavigationUKcc";
    m_processPool = new ProcessPool(6);
}

NavigationUKcc::~NavigationUKcc()
{
    qDebug()<<"deinit NavigationUKcc";
}

void NavigationUKcc::OpenUkccUserInfo()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"userinfo");
}

void NavigationUKcc::OpenUkccAudio()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"audio");
}

void NavigationUKcc::OpenUkccPower()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"power");
}

void NavigationUKcc::OpenUkccBluetooth()
{
    Peek_Ukcc();//需要通过rfkill检查蓝牙
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"bluetooth");
}

void NavigationUKcc::OpenUkccAutostart()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"autostart");
}

void NavigationUKcc::OpenUkccLoginOptions()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"Login Options");
}

void NavigationUKcc::OpenUkccTouchpad()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"touchpad");
}

void NavigationUKcc::OpenUkccTouchscreen()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"touchscreen");
}

void NavigationUKcc::OpenUkccNetconnect()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"netconnect");
}

void NavigationUKcc::OpenUkccWlanconnect()
{
    Peek_Ukcc();//需要通过rfkill检查wifi
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"wlanconnect");
}

void NavigationUKcc::OpenUkccTheme()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"theme");
}

void NavigationUKcc::OpenUkccScreensaver()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"screensaver");
}

void NavigationUKcc::OpenUkccUpgrade()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"upgrade");
}

void NavigationUKcc::OpenUkccSecurityCenter()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"securitycenter");
}

void NavigationUKcc::OpenUkccDisplay()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"display");
}

void NavigationUKcc::OpenUkccScreenlock()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"screenlock");
}

void NavigationUKcc::OpenUkccNotice()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"notice");
}

void NavigationUKcc::OpenUkccVino()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"vino");
}

void NavigationUKcc::OpenUkccPrinter()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"printer");
}

void NavigationUKcc::OpenUkccShortcut()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"shortcut");
}

void NavigationUKcc::OpenUkccKeyboard()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"keyboard");
}

void NavigationUKcc::OpenUkccProxy()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"proxy");
}

void NavigationUKcc::OpenUkccMobileHotspot()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"mobilehotspot");
}

void NavigationUKcc::OpenUkccArea()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"area");
}

void NavigationUKcc::OpenUkccBackup()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"backup");
}

void NavigationUKcc::OpenUkccDate()
{
    Peek_Ukcc();
    Start_Process(c_Ukcc, QStringList()<<"-m"<<"date");
}


void NavigationUKcc::OpenKyomDeviceManager()
{
    Peek_Ukcc();
    Start_Process(c_Kyom, QStringList()<< "--jumpTab=DeviceManager");
}


void NavigationUKcc::OpenUksmAbout()
{
    Peek_Uksm();
    Start_Process(c_Uksm, QStringList()<< "-m"<<"about");
}

void NavigationUKcc::OpenUkClock()
{
    Peek_Ukclock();
    Start_Process(c_Ukclock, QStringList());
}

void NavigationUKcc::OpenKyMusic()
{
    Peek_Ukmusic();
    Start_Process(c_Ukmusic, QStringList());
}

void NavigationUKcc::OpenKyVideo()
{
    Peek_Ukvideo();
    Start_Process(c_Ukvideo, QStringList());
}

bool NavigationUKcc::checkAppExist(QString appPath)
{
    QFile file(appPath);

    return file.exists();
}
