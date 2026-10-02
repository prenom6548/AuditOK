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

 
#include "osdserver.h"

#include "qmlmanager.h"

#include <QDebug>

OsdServer::OsdServer(QObject* parent) : QObject(parent)
{
    QmlManager::instance();
}

OsdServer::~OsdServer()
{

}

void OsdServer::ShowBarWindow(const QString& sourceName, int currentData, int maxData)
{
    QmlManager::instance()->onShowBar(sourceName, currentData, maxData);
}

void OsdServer::ShowVolumeBar(int currentData, int maxData)
{
    ShowVolumeBarWithMute(currentData, maxData, false);
}

void OsdServer::ShowVolumeBarWithMute(int currentData, int maxData, bool mute)
{
    double per = (currentData*1.0 / maxData*1.0) * 100;
    QString mIconName;
    if (per == 0) {
        mIconName = "audio-volume-muted-symbolic";
    } else if (per <= 33) {
        mIconName = "audio-volume-low-symbolic";
    } else if (per <= 66) {
        mIconName = "audio-volume-medium-symbolic";
    } else {
        mIconName = "audio-volume-high-symbolic";
    }
    if (mute) {
        mIconName = "audio-volume-muted-symbolic";
    }
    QmlManager::instance()->onShowBar(mIconName, currentData, maxData);
}

void OsdServer::ShowBrightnessBar(int currentData, int maxData)
{
    double per = (currentData*1.0 / maxData*1.0) * 100;
    QString mIconName;
    if (per == 0) {
        mIconName = "ukui-light-0-symbolic";
    } else if (per <= 25) {
        mIconName = "ukui-light-25-symbolic";
    } else if (per <= 50) {
        mIconName = "ukui-light-50-symbolic";
    } else if (per <= 75) {
        mIconName = "ukui-light-75-symbolic";
    } else {
        mIconName = "ukui-light-100-symbolic";
    }
    QmlManager::instance()->onShowBar(mIconName, currentData, maxData);
}

void OsdServer::ShowCustomOSD(const QString &sourceName)
{
    QmlManager::instance()->onShowIcon(sourceName);
}

void OsdServer::ShowFlightModeOSD(bool enabled)
{
    if (enabled) {
        ShowFlightModeOn();
    } else {
        ShowFlightModeOff();
    }
}

void OsdServer::ShowTouchpadStatusOSD(bool enabled)
{
    if (enabled) {
        ShowTouchpadStatusOn();
    } else {
        ShowTouchpadStatusOff();
    }
}

void OsdServer::ShowMicrophoneStatusOSD(bool enabled)
{
    if (enabled) {
        ShowMicrophoneStatusOn();
    } else {
        ShowMicrophoneStatusOff();
    }
}

void OsdServer::ShowCameraStatusOSD(bool enabled)
{
    if (enabled) {
        ShowCameraStatusOn();
    } else {
        ShowCameraStatusOff();
    }
}

void OsdServer::ShowAudioStatusOSD(bool muted)
{
    if (muted) {
        ShowAudioStatusOn();
    } else {
        ShowAudioStatusOff();
    }
}

void OsdServer::ShowPowerModeOSD(int mode)
{
    if (mode == 0) {
        ShowPowerModeEco();
    } else if (mode == 1) {
        ShowPowerModeAuto();
    } else if (mode == 2) {
        ShowPowerModePerformance();
    } else {

    }
}

void OsdServer::ShowWifiModeOSD(bool connected)
{
    if (connected) {
        ShowWifiModeOn();
    } else {
        ShowWifiModeOff();
    }
}

void OsdServer::ShowFlightModeOn()
{
    ShowCustomOSD(QStringLiteral("ukui-airplane-mode-on-symbolic"));
}

void OsdServer::ShowFlightModeOff()
{
    ShowCustomOSD(QStringLiteral("ukui-airplane-mode-off-symbolic"));
}

void OsdServer::ShowTouchpadStatusOn()
{
    ShowCustomOSD(QStringLiteral("ukui-touchpad-on-symbolic"));
}

void OsdServer::ShowTouchpadStatusOff()
{
    ShowCustomOSD(QStringLiteral("ukui-touchpad-off-symbolic"));
}

void OsdServer::ShowMicrophoneStatusOn()
{
    ShowCustomOSD(QStringLiteral("ukui-microphone-on-symbolic"));
}

void OsdServer::ShowMicrophoneStatusOff()
{
    ShowCustomOSD(QStringLiteral("ukui-microphone-off-symbolic"));
}

void OsdServer::ShowCameraStatusOn()
{
    ShowCustomOSD(QStringLiteral("camera-photo-symbolic"));
}

void OsdServer::ShowCameraStatusOff()
{
    ShowCustomOSD(QStringLiteral("camera-switch-off-symbolic"));
}

void OsdServer::ShowAudioStatusOn()
{
    ShowCustomOSD(QStringLiteral("audio-volume-medium-symbolic"));
}

void OsdServer::ShowAudioStatusOff()
{
    ShowCustomOSD(QStringLiteral("audio-volume-muted-symbolic"));
}

void OsdServer::ShowPowerModeEco()
{
    ShowCustomOSD(QStringLiteral("ukui-eco-symbolic"));
}

void OsdServer::ShowPowerModeAuto()
{
    ShowCustomOSD(QStringLiteral("ukui-auto-symbolic"));
}

void OsdServer::ShowPowerModePerformance()
{
    ShowCustomOSD(QStringLiteral("ukui-performance-symbolic"));
}

void OsdServer::ShowWifiModeOn()
{
    ShowCustomOSD(QStringLiteral("network-wireless-connected-symbolic"));
}

void OsdServer::ShowWifiModeOff()
{
    ShowCustomOSD(QStringLiteral("network-wireless-offline-symbolic"));
}




