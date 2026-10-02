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

#include "shortcut_navigations.h"

NavigationShortcut::NavigationShortcut(QObject *parent):
    QObject(parent)
{
}

NavigationShortcut::~NavigationShortcut()
{

}

void NavigationShortcut::TouchpadToggleState()
{
    callMediaKeys("externalDoActionWithName","TOUCHPAD_KEY","ukui-framework");
}

void NavigationShortcut::MuteToggleState()
{
    callMediaKeys("externalDoActionWithName","MUTE_KEY","ukui-framework");
}

void NavigationShortcut::VolumeDown()
{
    callMediaKeys("externalDoActionWithName","VOLUME_DOWN_KEY","ukui-framework");
}

void NavigationShortcut::VolumeUp()
{
    callMediaKeys("externalDoActionWithName","VOLUME_UP_KEY","ukui-framework");
}

void NavigationShortcut::MicMuteToggleState()
{
    callMediaKeys("externalDoActionWithName","MIC_MUTE_KEY","ukui-framework");
}

void NavigationShortcut::BrightUp()
{
    callMediaKeys("externalDoActionWithName","BRIGHT_UP_KEY","ukui-framework");
}

void NavigationShortcut::BrightDown()
{
    callMediaKeys("externalDoActionWithName","BRIGHT_DOWN_KEY","ukui-framework");
}

void NavigationShortcut::PowerDown()
{
    callMediaKeys("externalDoActionWithName","POWER_DOWN_KEY","ukui-framework");
}

void NavigationShortcut::PowerOff()
{
    callMediaKeys("externalDoActionWithName","POWER_OFF_KEY","ukui-framework");
}

void NavigationShortcut::OpenFileManager()
{
    callMediaKeys("externalDoActionWithName","EJECT_KEY","ukui-framework");
}

void NavigationShortcut::OpenMedia()
{
    callMediaKeys("externalDoActionWithName","MEDIA_KEY","ukui-framework");
}

void NavigationShortcut::OpenCalculator()
{
    callMediaKeys("externalDoActionWithName","CALCULATOR_KEY","ukui-framework");
}

void NavigationShortcut::OpenEmail()
{
    callMediaKeys("externalDoActionWithName","EMAIL_KEY","ukui-framework");
}

void NavigationShortcut::OpenScreensaver()
{
    callMediaKeys("externalDoActionWithName","SCREENSAVER_KEY","ukui-framework");
}

void NavigationShortcut::OpenWWW()
{
    callMediaKeys("externalDoActionWithName","WWW_KEY","ukui-framework");
}

void NavigationShortcut::MediaPlay()
{
    callMediaKeys("externalDoActionWithName","PLAY_KEY","ukui-framework");
}

void NavigationShortcut::MediaPause()
{
    callMediaKeys("externalDoActionWithName","PAUSE_KEY","ukui-framework");
}

void NavigationShortcut::MediaPlaypause()
{
    callMediaKeys("externalDoActionWithName","PLAYPAUSE_KEY","ukui-framework");
}

void NavigationShortcut::MediaStop()
{
    callMediaKeys("externalDoActionWithName","STOP_KEY","ukui-framework");
}

void NavigationShortcut::MediaPrevious()
{
    callMediaKeys("externalDoActionWithName","PREVIOUS_KEY","ukui-framework");
}

void NavigationShortcut::MediaNext()
{
    callMediaKeys("externalDoActionWithName","NEXT_KEY","ukui-framework");
}

void NavigationShortcut::MediaRewind()
{
    callMediaKeys("externalDoActionWithName","REWIND_KEY","ukui-framework");
}

void NavigationShortcut::MediaForward()
{
    callMediaKeys("externalDoActionWithName","FORWARD_KEY","ukui-framework");
}

void NavigationShortcut::MediaRepeat()
{
    callMediaKeys("externalDoActionWithName","REPEAT_KEY","ukui-framework");
}

void NavigationShortcut::MediaShuffleKey()
{
    callMediaKeys("externalDoActionWithName","RANDOM_KEY","ukui-framework");
}

void NavigationShortcut::OpenSettings()
{
    callMediaKeys("externalDoActionWithName","SETTINGS_KEY","ukui-framework");
}

void NavigationShortcut::FileManagerKey()
{
    callMediaKeys("externalDoActionWithName","FILE_MANAGER_KEY","ukui-framework");
}

void NavigationShortcut::OpenShutdownManagement()
{
    callMediaKeys("externalDoActionWithName","SHUTDOWN_MANAGEMENT_KEY","ukui-framework");
}

void NavigationShortcut::OpenTerminal()
{
    callMediaKeys("externalDoActionWithName","TERMINAL_KEY","ukui-framework");
}

void NavigationShortcut::Screenshot()
{
    callMediaKeys("externalDoActionWithName","SCREENSHOT_KEY","ukui-framework");
}

void NavigationShortcut::WindowScreenshot()
{
    callMediaKeys("externalDoActionWithName","WINDOW_SCREENSHOT_KEY","ukui-framework");
}

void NavigationShortcut::AreaScreenshot()
{
    callMediaKeys("externalDoActionWithName","AREA_SCREENSHOT_KEY","ukui-framework");
}

void NavigationShortcut::OpenWindowSwitch()
{
    callMediaKeys("externalDoActionWithName","WINDOWSWITCH_KEY","ukui-framework");
}

void NavigationShortcut::SystemMonitorKey()
{
    callMediaKeys("externalDoActionWithName","SYSTEM_MONITOR_KEY","ukui-framework");
}

void NavigationShortcut::OpenConnectionEditor()
{
    callMediaKeys("externalDoActionWithName","CONNECTION_EDITOR_KEY","ukui-framework");
}

void NavigationShortcut::OpenGlobalSearch()
{
    callMediaKeys("externalDoActionWithName","GLOBAL_SEARCH_KEY","ukui-framework");
}

void NavigationShortcut::OpenDisplaySwitch()
{
    callMediaKeys("externalDoActionWithName","KDS_KEY","ukui-framework");
}

void NavigationShortcut::ToggleWlanSwitch()
{
    callMediaKeys("externalDoActionWithName","WLAN_KEY","ukui-framework");
}

void NavigationShortcut::CamToggleState()
{
    callMediaKeys("externalDoActionWithName","WEBCAM_KEY","ukui-framework");
}

void NavigationShortcut::OpenUkuiSidebar()
{
    callMediaKeys("externalDoActionWithName","UKUI_SIDEBAR","ukui-framework");
}

void NavigationShortcut::TouchpadOn()
{
    callMediaKeys("externalDoActionWithName","TOUCHPAD_ON_KEY","ukui-framework");
}

void NavigationShortcut::TouchpadOff()
{
    callMediaKeys("externalDoActionWithName","TOUCHPAD_OFF_KEY","ukui-framework");
}

void NavigationShortcut::ToggleFlightModeState()
{
    callMediaKeys("externalDoActionWithName","RFKILL_KEY","ukui-framework");
}

void NavigationShortcut::TurnOnFlightModeState()
{
    callMediaKeys("externalDoActionWithName","FLIGHT_MODE_ON","ukui-framework");
}

void NavigationShortcut::TurnOffFlightModeState()
{
    callMediaKeys("externalDoActionWithName","FLIGHT_MODE_OFF","ukui-framework");
}

void NavigationShortcut::ToggleBluetoothState()
{
    callMediaKeys("externalDoActionWithName","BLUETOOTH_KEY","ukui-framework");
}

void NavigationShortcut::OpenAIassistant()
{
    callMediaKeys("externalDoActionWithName","ASRASSISTANT","ukui-framework");
}

void NavigationShortcut::SwitchPerformanceMode()
{
    callMediaKeys("externalDoActionWithName","PERFORMANCE_KEY","ukui-framework");
}

void NavigationShortcut::OpenKylinManager()
{
    callMediaKeys("externalDoActionWithName","KYLIN_SERVICE_SUPPORT","ukui-framework");
}

void NavigationShortcut::OpenClipboard()
{
    callMediaKeys("externalDoActionWithName","UKUI_CLIPBOARD","ukui-framework");
}

void NavigationShortcut::OpenKylinScreencap()
{
    callMediaKeys("externalDoActionWithName","KYLIN_SCREENCAP","ukui-framework");
}

template <typename...Args>
QDBusMessage NavigationShortcut::callMediaKeys(const QString &method, Args &&...args)
{
    static QDBusInterface *mediaKeysInterface = new QDBusInterface("org.ukui.SettingsDaemon",
                                      "/org/ukui/SettingsDaemon/MediaKeys",
                                      "org.ukui.SettingsDaemon.MediaKeys");


    return mediaKeysInterface->call(QDBus::CallMode::NoBlock, method, std::forward<Args>(args)...);
}
