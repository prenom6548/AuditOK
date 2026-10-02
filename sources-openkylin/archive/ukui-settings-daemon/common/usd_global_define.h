/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2020 KylinSoft Co., Ltd.
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

#ifndef USD_GLOBAL_DEFINE_H
#define USD_GLOBAL_DEFINE_H


#define USD_BTN_PRESS   2
#define USD_BTN_RELEASE 3

/****dbus****/

/*com.kylin.statusmanager.interface*/
#define DBUS_STATUSMANAGER_NAME             "com.kylin.statusmanager.interface"
#define DBUS_STATUSMANAGER_PATH             "/"
#define DBUS_STATUSMANAGER_INTERFACE        "com.kylin.statusmanager.interface"
#define DBUS_STATUSMANAGER_GET_MODE         "get_current_tabletmode"
#define DBUS_STATUSMANAGER_GET_ROTATION     "get_current_rotation"
/*****/

/*org.ukui.SettingsDaemon.xrandr*/
#define SETTINGS_XRANDR_SCHEMAS             "org.ukui.SettingsDaemon.plugins.xrandr"
#define DBUS_XRANDR_NAME                    "org.ukui.SettingsDaemon"
#define DBUS_XRANDR_PATH                    "/org/ukui/SettingsDaemon/xrandr"
#define DBUS_XRANDR_INTERFACE               "org.ukui.SettingsDaemon.xrandr"
#define REMEMBER_KEY                        "remember"
#define DEFAULT_MODE_KEY                    "default-mode"
#define XRANDR_PRI_KEY                      "primary"
#define DBUS_XRANDR_GET_MODE                "getScreenMode"
#define DBUS_XRANDR_SET_MODE                "setScreenMode"
#define DBUS_XRANDR_GET_SCREEN_PARAM        "getScreensParam"
#define DEFAULT_PRIMARY_SCREEN              "default-primary"
#define GSETTINGS_XRANDR_KEY_USE_EDID_HASH  "use-edid-hash"
/*****/

/*org.ukui.SettingsDaemon.gammaManager*/

#define DBUS_GM_NAME                    "org.ukui.SettingsDaemon"
#define DBUS_GM_PATH                    "/org/ukui/SettingsDaemon/GammaManager"
#define DBUS_GM_INTERFACE               "org.ukui.SettingsDaemon.GammaManager"
#define DBUS_GM_SCREENCHANGED           "screenBrightnessChanged"
/*****/

/*com.control.center.qt.systemdbus*/
#define DBUS_CONTROL_CENTER_NAME            "com.control.center.qt.systemdbus"
#define DBUS_CONTROL_CENTER_PATH            "/"
#define DBUS_CONTROL_CENTER_INTERFACE       "com.control.center.interface"
/*****/

/*com.settings.daemon.qt.systemdbus*/
#define SETTINGS_DAEMON_SYSTEMDBUS_NAME            "com.kylin.ukui.SettingsDaemon"
#define SETTINGS_DAEMON_SYSTEMDBUS_PATH            "/"
#define SETTINGS_DAEMON_SYSTEMDBUS_PATH_SECURITY   "/securityConfig"
#define SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE       "com.kylin.ukui.SettingsDaemon.interface"
/**/
//panel 整体配置
#define GSETTINGS_PANEL_SCHEMA                   "org.ukui.panel"
#define GSETTINGS_PANEL_SC_KEY                   "support-config"
#define GSETTINGS_PANEL_LOAD_PLUGIN_KEY          "load-plugin"

#define GSETTINGS_PANEL_LEFT_KEY                 "left"
#define GSETTINGS_PANEL_RIGHT_KEY                "right"

//media-keys gsettings
#define GSETTINGS_USD_MEDIA_KEYS_SCHEMA     "org.ukui.SettingsDaemon.plugins.media-keys"
#define GSETTINGS_USD_MEDIA_KDS             "kylin-display-switch"
//panel quickbar 快速启动栏配置（固定区域配置）
#define GSETTINGS_PANEL_QUICKBAR_SCHEMA                   "org.ukui.panel.quickbar"
//panel showdesktop 展示桌面配置
#define GSETTINGS_PANEL_SHOWDESKTOP_SCHEMA                   "org.ukui.panel.showdesktop"
//panel showdesktop 状态通知
#define GSETTINGS_PANEL_STATUSNOTIFIER_SCHEMA                   "org.ukui.panel.statusnotifier"
//panel viewbar 任务试图
#define GSETTINGS_PANEL_VIEWBAR_SCHEMA                   "org.ukui.panel.viewbar"
//panel calendar 日历配置
#define GSETTINGS_PANEL_CALENDAR_SCHEMA                   "org.ukui.panel.calendar"
//panel startbar 开始菜单进度条
#define GSETTINGS_PANEL_STARTBAR_SCHEMA                   "org.ukui.panel.startbar"
//panel taskbar 激活窗口
#define GSETTINGS_PANEL_TASKBAR_SCHEMA                   "org.ukui.panel.taskbar"

#define GSETTINGS_PANEL_TASKBAR_SCHEMA                   "org.ukui.panel.taskbar"
#define GSETTINGS_PANEL_SETTINGS_SCHEMA                   "org.ukui.panel.settings"
#define GSETTINGS_PANEL_SETTINGS_VISIBLE                  "panelvisible"
#define GSETTINGS_NOTIFICATION_SCHEMA           "org.ukui.notification.daemon"
#define GSETTINGS_MENU_SCHEMA                   "org.ukui.menu.settings"

//dbus 配置
#define GNOME_SESSION_MANAGER               "org.gnome.SessionManager.Presence"
#define SESSION_MANAGER_PATH                "/org/gnome/SessionManager/Presence"



/**/
#define DBUS_GC_BRIGHTNESS_PATH                             "/GlobalBrightness"
#define DBUS_GC_BRIGHTNESS_INTERFACE                        "org.ukui.SettingsDaemon.Brightness"
#define DBUS_GC_BRIGHTNESS_SIGNAL_PRIMARYCHANGED_START      "primaryBrightnessChangedStart"
#define DBUS_GC_BRIGHTNESS_SIGNAL_PRIMARYCHANGED            "primaryBrightnessChanged"
#define DBUS_GC_BRIGHTNESS_SIGNAL_PRIMARYCHANGED_END        "primaryBrightnessChangedEnd"
#define DBUS_GC_BRIGHTNESS_SIGNAL_ENABLECHANGED             "enableChanged"
#define DBUS_GC_BRIGHTNESS_SIGNAL_ENHANCEMENT_CHANGED       "enhancementBrightnessSwitchChanged"
#define GSETTINGS_CUSTOMIZED_SCHEMA                 "org.ukui.SettingsDaemon.plugins.customized"

/**/
#define DBUS_GC_SIGNAL_PATH                         "/GlobaSignal"
#define DBUS_GC_SIGNAL_INTERFACE                    "org.ukui.SettingsDaemon.GlobalSignal"
#define DBUS_GC_SIGNAL_SIGNAL_Active                "Active"
#define DBUS_GC_SIGNAL_SIGNAL_HIDE_OSD              "hideOsd"

/*org.ukui.SettingsDaemon.mediakey*/
#define DBUS_MK_NAME                    "org.ukui.SettingsDaemon"
#define DBUS_MK_PATH                    "/org/ukui/SettingsDaemon/MediaKeys"
#define DBUS_MK_INTERFACE               "org.ukui.SettingsDaemon.MediaKeys"
#define DBUS_MK_DOACTION                "externalDoAction"

/*wlcom media-key，touch calibrate 使用*/
#define KDE_KWIN_SERVICE "org.kde.KWin"
#define KDE_KWIN_OBJECT_PATH  "/org/kde/KWin/InputDevice/"
#define KDE_KWIN_INTERFACE "org.kde.KWin.InputDevice"
#define KDE_KWIN_OBJECT "/org/kde/KWin/InputDevice"
#define KDE_KWIN_DEVICE_MANAGER "org.kde.KWin.InputDeviceManager"

//gsettings

#define ORG_UKUI_DBUS_WLCOM_NAME "com.kylin.Wlcom"
#define ORG_UKUI_DBUS_WLCOM_RENDERER_PATH "/com/kylin/Wlcom/Renderer"
#define ORG_UKUI_DBUS_WLCOM_RENDERER_INTERFACE  "com.kylin.Wlcom.Renderer"
#define ORG_UKUI_DBUS_WLCOM_RENDERER_ISHADRWARE "IsHardwareRendering"

/*com.kylin.statusmanager.interface*/
/****dbus end****/
#define MIN_GAMMA_BRIGHTNESS    0.35
#define MAX_GAMMA_BRIGHTNESS    0.65
/****gsettings****/
//ukui.style
#define UKUI_STYLE_SCHEMA                   "org.ukui.style"
#define SYSTEM_FONT_SIZE                    "system-font-size"
#define SYSTEM_FONT                         "system-font"

//MATE_INTERFACE
#define MATE_INTERFACE_SCHEMA "org.mate.interface"
//GMONE_DESKTOP_INTERFACE
#define GNOME_DESKTOP_INTERFACE "org.gnome.desktop.interface"

//auto-brightness gsettings
#define AUTO_BRIGHTNESS_SCHEMA              "org.ukui.SettingsDaemon.plugins.auto-brightness"
#define AUTO_BRIGHTNESS_KEY                 "auto-brightness"
#define DYNAMIC_BRIGHTNESS_KEY              "dynamic-brightness"
#define HAD_SENSOR_KEY                      "have-sensor"
#define DEBUG_MODE_KEY                      "debug-mode"
#define DEBUG_LUX_KEY                       "debug-lux"
#define DELAYMS_KEY                         "delayms"

#define POWER_MANAGER_SCHEMA                "org.ukui.power-manager"
#define BRIGHTNESS_AC_KEY                   "brightness-ac"
#define BRIGHTNESS_ENHANCEMENT_SWITCH       "brightness-enhancement-switch"
#define G_DEFAULT_MAX_BRIGHTNESS  100
#define G_ENHANCED_MAX_BRIGHTNESS   150
/*Xorg shutkey name*/

//xsettings
#define GSETTINGS_XSETTINGS_SCHEMA          "org.ukui.SettingsDaemon.plugins.xsettings"
#define SCALING_FACTOR_KEY                  "scaling-factor"


//mouse
#define GSETTINGS_MOUSE_SCHEMA              "org.ukui.peripherals-mouse"
#define CURSOR_THEME_KEY                    "cursor-theme"
#define CURSOR_SIZE_KEY                     "cursor-size"


//xsettings
#define GSETTINGS_GLOBALMANAGER_SCHEMA          "org.ukui.SettingsDaemon.plugins.globalManager"
#define HIDE_POWEROFF_UI                        "disable-power-operation"
#define DISABLE_POWEROFF_OPTIONS                "disable-shutdown-option"
#define UKUI_LITE_FUNCTION_KEY                  "ukui-lite-function"
#define UKUI_LITE_ANIMATION_KEY                 "ukui-lite-animation"
#define UKUI_LITE_MODE                          "ukui-lite"

#define C_GNOME_SETTINGS_SCHE  "org.gnome.desktop.interface"

#define UKUI_LITE_FUNCTION                      "function"
#define UKUI_LITE_ANIMATION                     "animation"

#define UKUI_DRM_PATH                       "/sys/class/drm/"
/*
 * 变量
*/
#define MAX_SIZE_MATCH_DIFF         0.05

//session
#define SESSION_BUSY    0
#define SESSION_IDLE    3

#define LEFT_SHIFT 0x32
#define LEFT_CTRL   0x25
#define LEFT_ALT    0x40
#define RIGHT_SHIFT 0x3E
#define RIGHT_ALT 0x6C
#define RIGHT_CTRL 0x69
#define MATE_KEY 0x85

/* EDID */
#define EDID_OFFSET_PNPID 0x08
#define EDID_OFFSET_SERIAL 0x0c
#define EDID_OFFSET_SIZE 0x15
#define EDID_OFFSET_GAMMA 0x17
#define EDID_OFFSET_DATA_BLOCKS 0x36
#define EDID_OFFSET_LAST_BLOCK 0x6c
#define EDID_OFFSET_EXTENSION_BLOCK_COUNT 0x7e
#define EDID_BYTE_DESCRIPTOR_COUNT 4
#define EDID_BYTE_DESCRIPTOR_SIZE 18

#define DESCRIPTOR_DISPLAY_PRODUCT_NAME 0xfc
#define DESCRIPTOR_DISPLAY_PRODUCT_SERIAL_NUMBER 0xff
#define DESCRIPTOR_COLOR_MANAGEMENT_DATA 0xf9
#define DESCRIPTOR_ALPHANUMERIC_DATA_STRING 0xfe
#define DESCRIPTOR_COLOR_POINT 0xfb

#define xEventHandleHadRelase(action)     bool action##HadRelease = false

#define xEventHandleRelease(action)       action##HadRelease = false;

#define xEventHandle(action) \
if (false == action##HadRelease) {  \
    doAction(action);   \
    action##HadRelease = true;  \
}

#define STATE_OFF     0
#define STATE_ON      1
#define STATE_TOGGLE  2

#define OSD_SIZE        84,84
#define OSD_FREME_SIZE  64,64
#define OSD_ICON_SIZE   48,48

/*Xorg shutkey nameEnd*/


#define TIME_LIMIT(time) \
    static QTime startTime = QTime::currentTime();  \
    static int elapsed = -1;    \
    elapsed = startTime.msecsTo(QTime::currentTime());  \
    if(elapsed > 0 && elapsed <= time){  \
        /*USD_LOG(LOG_ERR,"time in TIME_LIMIT");*/    \
        return; \
    }   \
    startTime = QTime::currentTime();

//全局配置
#define GLOBAL_CONFIG_UKUI_PATH "/etc/ukui/"
#define GLOBAL_CONFIG_USD_PATH "/etc/ukui/usd/"
#define GLOBAL_CONFIG_PATH  "/etc/ukui/usd/config/"

//action type :快捷键对应功能
#define ACTION_ENUM_TYPE     TOUCHPAD_KEY,  \
                                MUTE_KEY,   \
                                VOLUME_DOWN_KEY,    \
                                VOLUME_UP_KEY,  \
                                MIC_MUTE_KEY,   \
                                BRIGHT_UP_KEY,  \
                                BRIGHT_DOWN_KEY,    \
                                POWER_DOWN_KEY, \
                                POWER_OFF_KEY,  \
                                EJECT_KEY,  \
                                HOME_KEY,   \
                                MEDIA_KEY,  \
                                CALCULATOR_KEY, \
                                EMAIL_KEY,  \
                                SCREENSAVER_KEY,    \
                                HELP_KEY,   \
                                WWW_KEY,    \
                                PLAY_KEY,   \
                                PAUSE_KEY,  \
                                PLAYPAUSE_KEY,  \
                                STOP_KEY,   \
                                PREVIOUS_KEY,   \
                                NEXT_KEY,   \
                                REWIND_KEY, \
                                FORWARD_KEY,    \
                                REPEAT_KEY, \
                                CURSOR_PROMPT_KEY,  \
                                RANDOM_KEY, \
                                SETTINGS_KEY,   \
                                FILE_MANAGER_KEY,   \
                                SHUTDOWN_MANAGEMENT_KEY,    \
                                TERMINAL_KEY,   \
                                SCREENSHOT_KEY, \
                                WINDOW_SCREENSHOT_KEY,  \
                                AREA_SCREENSHOT_KEY,    \
                                WINDOWSWITCH_KEY,\
                                SYSTEM_MONITOR_KEY, \
                                CONNECTION_EDITOR_KEY,  \
                                GLOBAL_SEARCH_KEY,  \
                                KDS_KEY,    \
                                WLAN_KEY,   \
                                WEBCAM_KEY, \
                                HANDLED_KEYS,   \
                                UKUI_SIDEBAR,   \
                                UKUI_EYECARE_CENTER,    \
                                TOUCHPAD_ON_KEY,    \
                                TOUCHPAD_OFF_KEY,   \
                                RFKILL_KEY, \
                                BLUETOOTH_KEY,  \
                                ASRASSISTANT,   \
                                PERFORMANCE_KEY,    \
                                UKUI_ASSISTANT, /*工具箱 */ \
                                KYLIN_SERVICE_SUPPORT,    /*服务与支持*/    \
                                UKUI_CLIPBOARD, /*剪切板*/  \
                                KYLIN_SCREENCAP,      /*打开录屏*/  \
                                KYLIN_AI,   \
                                EXTEND_MEDIAKEYS
#endif // USD_GLOBAL_DEFINE_H
