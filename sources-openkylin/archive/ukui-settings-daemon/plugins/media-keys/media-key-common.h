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

#ifndef __MEDIAKEYCOMMON_H__
#define __MEDIAKEYCOMMON__

#include <QString>
#include <QKeySequence>
#include "media-type.h"

#define LENGTH(arr) (int)(sizeof (arr) / sizeof ((arr)[0]))
//快捷键模式
typedef enum
{

} ActionMode;

typedef struct
{
    ActionType type;
    QString actionName;
    QList<QKeySequence> defaultKeys;
//    ActionMode modes;
} MediaKeys;

//fn hotkey
const static MediaKeys gs_mediaKeyStatic[]
{
    { TOUCHPAD_KEY,             "touchpad-static",                 QList<QKeySequence>{Qt::Key_TouchpadToggle}},
    { TOUCHPAD_ON_KEY,          "touchpad-on-static",              QList<QKeySequence>{Qt::Key_TouchpadOn}},
    { TOUCHPAD_OFF_KEY,         "touchpad-off-static",             QList<QKeySequence>{Qt::Key_TouchpadOff}},
    { MUTE_KEY,                 "volume-mute-static",              QList<QKeySequence>{Qt::Key_VolumeMute}},
    { VOLUME_DOWN_KEY,          "volume-down-static",              QList<QKeySequence>{Qt::Key_VolumeDown}},
    { VOLUME_UP_KEY,            "volume-up-static",                QList<QKeySequence>{Qt::Key_VolumeUp}},
    { MIC_MUTE_KEY,             "mic-mute-static",                 QList<QKeySequence>{Qt::Key_MicMute}},
    { BRIGHT_UP_KEY,            "brightness-up-static",            QList<QKeySequence>{Qt::Key_MonBrightnessUp}},
    { BRIGHT_DOWN_KEY,          "brightness-down-static",          QList<QKeySequence>{Qt::Key_MonBrightnessDown}},
    { POWER_OFF_KEY,            "power-static",                    QList<QKeySequence>{Qt::Key_PowerOff}},
    { POWER_DOWN_KEY,           "power-down-static",               QList<QKeySequence>{Qt::Key_PowerDown}},
    { EJECT_KEY,                "eject-static",                    QList<QKeySequence>{Qt::Key_Eject}},
    { HOME_KEY,                 "home-static",                     QList<QKeySequence>{Qt::Key_Explorer}},
    { EMAIL_KEY,                "email-static",                    QList<QKeySequence>{Qt::Key_LaunchMail}},
    { CALCULATOR_KEY,           "calculator-static",               QList<QKeySequence>{Qt::Key_Calculator}},
    { WWW_KEY,                  "www-static",                      QList<QKeySequence>{Qt::Key_WWW}},
    { MEDIA_KEY,                "meida-static",                    QList<QKeySequence>{Qt::Key_LaunchMedia}},
    { PLAY_KEY,                 "play-static",                     QList<QKeySequence>{Qt::Key_MediaPlay}},
    { PAUSE_KEY,                "pause-static",                    QList<QKeySequence>{Qt::Key_MediaPause}},
    { PLAYPAUSE_KEY,            "playpause-static",                QList<QKeySequence>{Qt::Key_MediaTogglePlayPause}},
    { STOP_KEY,                 "stop-static",                     QList<QKeySequence>{Qt::Key_MediaStop}},
    { PREVIOUS_KEY,             "previous-static",                 QList<QKeySequence>{Qt::Key_MediaPrevious}},
    { NEXT_KEY,                 "next-static",                     QList<QKeySequence>{Qt::Key_MediaNext}},
    { SETTINGS_KEY,             "ukui-control-center-static",      QList<QKeySequence>{Qt::Key_Tools}},
    { KDS_KEY,                  "kylin-display-switch-static",     QList<QKeySequence>{Qt::Key_Display}},
    { HELP_KEY,                 "help-static",                     QList<QKeySequence>{Qt::Key_Help}},
    { WLAN_KEY,                 "wlan-static",                     QList<QKeySequence>{Qt::Key_WLAN}},
    { RFKILL_KEY,               "rfkill-static",                   QList<QKeySequence>{Qt::Key_RFKill}},
    { BLUETOOTH_KEY,            "bluetooth-static",                QList<QKeySequence>{Qt::Key_Bluetooth}},
    { WEBCAM_KEY,               "webcam-static",                   QList<QKeySequence>{Qt::Key_WebCam}},
    { WINDOWSWITCH_KEY,         "ukui-window-switch-static",       QList<QKeySequence>{Qt::Key_TaskPane}},
    { SCREENSAVER_KEY,          "screensaver-static-static",       QList<QKeySequence>{Qt::Key_ScreenSaver}},
    { GLOBAL_SEARCH_KEY,        "ukui-search-static",              QList<QKeySequence>{Qt::Key_Search}},
    { PERFORMANCE_KEY,          "performance-static",              QList<QKeySequence>{Qt::Key_Battery}},
    //248-252
    { UKUI_SIDEBAR,             "ukui-sidebar-static",             QList<QKeySequence>{Qt::Key_Launch1}},//#214254 联想要求进主线，message是打开即时通讯软件。所以用新键值
    { UKUI_ASSISTANT,           "ukui-assistant-static",           QList<QKeySequence>{Qt::Key_Launch2}},//216575.tools已经应用于控制面板了。
    { KYLIN_AI,                 "kylin-ai-static",                 QList<QKeySequence>{Qt::Key_Launch3}},//需求29753

    { FORWARD_KEY,              "forward-static",                  QList<QKeySequence>{Qt::Key_AudioForward}},  //音频向前
    { REWIND_KEY,               "rewind-static",                   QList<QKeySequence>{Qt::Key_AudioRewind}},  //音频向后
//
};

//自定义组合键功能
const static MediaKeys gs_mediaKeyCustom[]
{
    { SETTINGS_KEY, "ukui-control-center", QList<QKeySequence>{}},
    { SCREENSAVER_KEY, "screensaver", QList<QKeySequence>{}},
    { SCREENSAVER_KEY, "screensaver2", QList<QKeySequence>{}},
    { SHUTDOWN_MANAGEMENT_KEY, "logout", QList<QKeySequence>{}},
    { FILE_MANAGER_KEY, "peony-qt", QList<QKeySequence>{}},
    { FILE_MANAGER_KEY, "peony-qt2", QList<QKeySequence>{}},
    { TERMINAL_KEY, "terminal", QList<QKeySequence>{}},
    { TERMINAL_KEY, "terminal2", QList<QKeySequence>{}},
    { SCREENSHOT_KEY, "screenshot", QList<QKeySequence>{}},
    { SCREENSHOT_KEY, "screenshot2", QList<QKeySequence>{}},
    { WINDOW_SCREENSHOT_KEY, "window-screenshot", QList<QKeySequence>{}},
    { AREA_SCREENSHOT_KEY, "area-screenshot", QList<QKeySequence>{}},
    { AREA_SCREENSHOT_KEY, "area-screenshot2", QList<QKeySequence>{}},
    { UKUI_SIDEBAR, "ukui-sidebar", QList<QKeySequence>{}},
    { WINDOWSWITCH_KEY, "ukui-window-switch", QList<QKeySequence>{}},
    { WINDOWSWITCH_KEY, "ukui-window-switch2", QList<QKeySequence>{}},
    { SYSTEM_MONITOR_KEY, "ukui-system-monitor", QList<QKeySequence>{}},
    { CONNECTION_EDITOR_KEY, "nm-connection-editor", QList<QKeySequence>{}},
    { GLOBAL_SEARCH_KEY, "ukui-search", QList<QKeySequence>{}},
//    { UKUI_EYECARE_CENTER, "eye-protection-center", QList<QKeySequence>{Qt::CTRL + Qt::ALT + Qt::Key_P}},
    { KDS_KEY, "kylin-display-switch", QList<QKeySequence>{}},
    { UKUI_CLIPBOARD, "ukui-cliboard", QList<QKeySequence>{}},
    { KYLIN_SERVICE_SUPPORT, "kylin-service-support", QList<QKeySequence>{}},
    { KYLIN_SCREENCAP, "kylin-screencap", QList<QKeySequence>{}},
};

#endif /* __MEDIAKEYCOMMON_H__ */
