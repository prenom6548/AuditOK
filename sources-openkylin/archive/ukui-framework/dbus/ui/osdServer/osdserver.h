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

 
#ifndef OSDSERVER_H
#define OSDSERVER_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusContext>

//#include "osd_string.h"
//#include "qmlmanager.h"


class OsdServer : public QObject, protected QDBusContext
{

    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.UI.OsdServer")

public:
    explicit OsdServer(QObject* parent = nullptr);

    ~ OsdServer();

signals:
    void shotIconSignals(const QString& sourceName);
    void shotBarSignals(const QString& sourceName, double currentData, double maxData);

public slots:
    // 自定义进度条
    Q_INVOKABLE void ShowBarWindow(const QString& sourceName, int currentData, int maxData);
    
    // 系统状态OSD显示方法
    Q_INVOKABLE void ShowVolumeBar(int currentData, int maxData);
    Q_INVOKABLE void ShowVolumeBarWithMute(int currentData, int maxData, bool mute);
    Q_INVOKABLE void ShowBrightnessBar(int currentData, int maxData);

    // 自定义OSD显示方法
    Q_INVOKABLE void ShowCustomOSD(const QString& sourceName);

    // 系统状态OSD显示方法
    void ShowFlightModeOSD(bool enabled);
    void ShowTouchpadStatusOSD(bool enabled);
    void ShowMicrophoneStatusOSD(bool enabled);
    void ShowCameraStatusOSD(bool enabled);
    void ShowAudioStatusOSD(bool muted);
    void ShowPowerModeOSD(int mode); 
    void ShowWifiModeOSD(bool connected);

    void ShowFlightModeOn();
    void ShowFlightModeOff();

    void ShowTouchpadStatusOn();
    void ShowTouchpadStatusOff();

    void ShowMicrophoneStatusOn();
    void ShowMicrophoneStatusOff();

    void ShowCameraStatusOn();
    void ShowCameraStatusOff();
 
    void ShowAudioStatusOn();
    void ShowAudioStatusOff();

    void ShowPowerModeEco(); 
    void ShowPowerModeAuto(); 
    void ShowPowerModePerformance(); 
  
    void ShowWifiModeOn();
    void ShowWifiModeOff();
};

#endif // OSDSERVER_H
