/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef DEFINETYPES_H
#define DEFINETYPES_H

typedef enum _LOCK_CMD_ID_e
{
    LOCK_CMD_ID_GET_USERINFO_LIST = 0,
    LOCK_CMD_ID_USERINFO_CHANGED,
    LOCK_CMD_ID_SET_USER,
    LOCK_CMD_ID_SWITCH_TO_USER,
    LOCK_CMD_ID_GET_DEFAULT_AUTH_USER,
    LOCK_CMD_ID_GET_CURRENT_USER,
    LOCK_CMD_ID_CURRENT_USER_CHANGED,
    LOCK_CMD_ID_GET_SESSIONS_LIST,
    LOCK_CMD_ID_SESSIONS_CHANGED,
    LOCK_CMD_ID_SET_SESSION,
    LOCK_CMD_ID_GET_CURRENT_SESSION,
    LOCK_CMD_ID_CURRENT_SESSION_CHANGED,
    LOCK_CMD_ID_START_SESSION,
    LOCK_CMD_ID_GET_INIT_DATA,
    LOCK_CMD_ID_LOGIN1_REQ_LOCK = 100,
    LOCK_CMD_ID_LOGIN1_REQ_UNLOCK,
    LOCK_CMD_ID_LOGIN1_PREPAREFORSLEEP,
    LOCK_CMD_ID_LOGIN1_SESSION_ACTIVE_CHANGED,
    LOCK_CMD_ID_LOGIN1_IS_SESSION_ACTIVE,
    LOCK_CMD_ID_LOGIN1_BLOCKINHIBITED_CHANGED,
    LOCK_CMD_ID_LOGIN1_SET_POWER_MANAGER,
    LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANSWITCHUSER,
    LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANHIBERNATE,
    LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANPOWEROFF,
    LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANREBOOT,
    LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANSUSPEND,
    LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANLOGOUT,
    LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANLOCKSCREEN,
    LOCK_CMD_ID_SYSTEM_UPGRADE_CHECK,
    LOCK_CMD_ID_LOGIN1_BLOCKLOGOUTINHIBITED_CHANGED,
    LOCK_CMD_ID_SYSTEM_UPGRADE_CHECK_HIDE,
    LOCK_CMD_ID_GSETTINGS_GET_LOCKSCREEN_CONF = 200,
    LOCK_CMD_ID_GSETTINGS_SET_LOCKSCREEN_CONF,
    LOCK_CMD_ID_GSETTINGS_LOCKSCREEN_CONF_CHANGED,
    LOCK_CMD_ID_GSETTINGS_GET_SCREENSAVER_CONF,
    LOCK_CMD_ID_GSETTINGS_SET_SCREENSAVER_CONF,
    LOCK_CMD_ID_GSETTINGS_SCREENSAVER_CONF_CHANGED,
    LOCK_CMD_ID_GSETTINGS_GET_POWERMANAGER_CONF,
    LOCK_CMD_ID_GSETTINGS_SET_POWERMANAGER_CONF,
    LOCK_CMD_ID_GSETTINGS_POWERMANAGER_CONF_CHANGED,
    LOCK_CMD_ID_GSETTINGS_GET_MATEBG_CONF,
    LOCK_CMD_ID_GSETTINGS_SET_MATEBG_CONF,
    LOCK_CMD_ID_GSETTINGS_MATEBG_CONF_CHANGED,
    LOCK_CMD_ID_GSETTINGS_GET_UKCCPLUGINS_CONF,
    LOCK_CMD_ID_GSETTINGS_SET_UKCCPLUGINS_CONF,
    LOCK_CMD_ID_GSETTINGS_UKCCPLUGINS_CONF_CHANGED,
    LOCK_CMD_ID_GSETTINGS_GET_THEMESTYLE_CONF,
    LOCK_CMD_ID_GSETTINGS_SET_THEMESTYLE_CONF,
    LOCK_CMD_ID_GSETTINGS_THEMESTYLE_CONF_CHANGED,
    LOCK_CMD_ID_GSETTINGS_GET_SESSION_CONF,
    LOCK_CMD_ID_GSETTINGS_SET_SESSION_CONF,
    LOCK_CMD_ID_GSETTINGS_SESSION_CONF_CHANGED,
    LOCK_CMD_ID_GSETTINGS_GET_KEYBOARD_CONF,
    LOCK_CMD_ID_GSETTINGS_SET_KEYBOARD_CONF,
    LOCK_CMD_ID_GSETTINGS_KEYBOARD_CONF_CHANGED,
    LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIAKEYS_CONF,
    LOCK_CMD_ID_GSETTINGS_SET_USD_MEDIAKEYS_CONF,
    LOCK_CMD_ID_GSETTINGS_USD_MEDIAKEYS_CONF_CHANGED,
    LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIA_STATE_KEYS_CONF,
    LOCK_CMD_ID_GSETTINGS_SET_USD_MEDIA_STATE_KEYS_CONF,
    LOCK_CMD_ID_GSETTINGS_USD_MEDIA_STATE_KEYS_CONF_CHANGED,
    LOCK_CMD_ID_GSETTINGS_GET_DEFAULT_PHOTOPATH,
    LOCK_CMD_ID_GSETTINGS_SET_DEFAULT_PHOTOPATH,
    LOCK_CMD_ID_GSETTINGS_GET_CUSTOM_PHOTOPATH,
    LOCK_CMD_ID_GSETTINGS_SET_CUSTOM_PHOTOPATH,
    LOCK_CMD_ID_GET_AGREEMENT = 300,
    LOCK_CMD_ID_LOCK_STATE_CHANGED,
    LOCK_CMD_ID_GET_PUBLIC_KEY,
    LOCK_CMD_ID_SEND_USER_PASSWORD,
    LOCK_CMD_ID_LIDSTATE_CHANGED,
    LOCK_CMD_ID_PAMAUTH_IS_INAUTHTICATION = 400,
    LOCK_CMD_ID_PAMAUTH_IS_AUTHENTICATED,
    LOCK_CMD_ID_PAMAUTH_GET_AUTHUSER,
    LOCK_CMD_ID_PAMAUTH_AUTHENTICATE,
    LOCK_CMD_ID_PAMAUTH_RESPOND,
    LOCK_CMD_ID_PAMAUTH_AUTHENTICATE_CANCEL,
    LOCK_CMD_ID_PAMAUTH_SHOWMESSAGE,
    LOCK_CMD_ID_PAMAUTH_SHOWPROMPT,
    LOCK_CMD_ID_PAMAUTH_AUTH_COMPLETED,
    LOCK_CMD_ID_PAMAUTH_SHOWPBINARYROMPT,
    LOCK_CMD_ID_PAMAUTH_GET_AUTOLOGINUSERHINT,
    LOCK_CMD_ID_USD_MEDIAKEYS = 500,
    LOCK_CMD_ID_UPOWER_BATTERY_STATUS,
    LOCK_CMD_ID_UPOWER_IS_BATTERY,
    LOCK_CMD_ID_UPOWER_BATTERY,
    LOCK_CMD_ID_SESSION_GET_SLEEP_LOCKCHECK,
    LOCK_CMD_ID_SESSION_GET_SHUTDOWN_LOCKCHECK,
    LOCK_CMD_ID_LOCK_SCREEN_GET_THEMES,
    LOCK_CMD_ID_KWIN_BLOCK_SHORTCUT,
    LOCK_CMD_ID_UPOWER_SERVICE_READY,
    LOCK_CMD_ID_SESSION_GET_LOGOUT_LOCKCHECK,
    LOCK_CMD_ID_SESSION_GET_OPEN_WINDOWS_LOCKCHECK,
    LOCK_CMD_ID_SESSION_SHOW_SHUTDOWN_ANIMATION,
    LOCK_CMD_ID_SESSION_SET_IN_CALL_MANAGER,
    LOCK_CMD_ID_SESSION_WINDOWLIST_CHANGED,
    LOCK_CMD_ID_SESSION_LEGACY_COMPONENTS_CHANGED,
    LOCK_CMD_ID_SESSION_GET_LEGACY_COMPONENTS,
    LOCK_CMD_ID_BIOAUTH_GET_AVAILABLE_DEVICES = 600,
    LOCK_CMD_ID_BIOAUTH_GET_DISABLED_DEVICES,
    LOCK_CMD_ID_BIOAUTH_GET_STATE,
    LOCK_CMD_ID_BIOAUTH_GET_CURDEVICE,
    LOCK_CMD_ID_BIOAUTH_FIND_DEVICE_BY_ID,
    LOCK_CMD_ID_BIOAUTH_FIND_DEVICE_BY_NAME,
    LOCK_CMD_ID_BIOAUTH_STARTAUTH,
    LOCK_CMD_ID_BIOAUTH_STOPAUTH,
    LOCK_CMD_ID_BIOAUTH_GET_DEFAULT_DEVICE,
    LOCK_CMD_ID_BIOAUTH_SERVICE_STATUS_CHANGED,
    LOCK_CMD_ID_BIOAUTH_DEVICE_CHANGED,
    LOCK_CMD_ID_BIOAUTH_SHOW_MESSAGE,
    LOCK_CMD_ID_BIOAUTH_AUTHSTATE_CHANGED,
    LOCK_CMD_ID_BIOAUTH_FRAME_DATA,
    LOCK_CMD_ID_BIOAUTH_COMPLETE,
    LOCK_CMD_ID_TABLET_MODE_CHANGED = 700,
    LOCK_CMD_ID_TABLET_MODE,
    LOCK_CMD_ID_NETWORK_CONNECT_STATE_CHANGED = 800,
    LOCK_CMD_ID_NETWORK_CONNECT_STATE,
    LOCK_CMD_ID_GET_WEATHER_DATA,
    LOCK_CMD_ID_GET_SAVER_DATETIME_ENABLE_DATA,
    LOCK_CMD_ID_SET_SAVER_DATETIME_ENABLE_DATA,
    LOCK_CMD_ID_SET_SAVER_DATETIME_ENABLE_DATA_CHANGED,
    LOCK_CMD_ID_GET_LOCK_DATETIME_ENABLE_DATA,
    LOCK_CMD_ID_SET_LOCK_DATETIME_ENABLE_DATA,
    LOCK_CMD_ID_SET_LOCK_DATETIME_ENABLE_DATA_CHANGED,
    LOCK_CMD_ID_COUNT,
    LOCK_CMD_ID_CAN_SWITCH_USER
} LOCK_CMD_ID;

#define GSETTINGS_SCHEMA_SCREENSAVER "org.ukui.screensaver"
#define KEY_IDLE_DELAY "idleDelay"
#define KEY_IDLE_LOCK "idleLock"
#define KEY_IDLE_ACTIVATION_ENABLED "idleActivationEnabled"
#define KEY_IDLE_LOCK_ENABLED "idleLockEnabled"
#define KEY_LOCK_TIMEOUT "lockTimeout"
#define KEY_CLOSE_ACTIVATION_ENABLED "closeActivationEnabled"
#define KEY_SLEEP_ACTIVATION_ENABLED "sleepActivationEnabled"
#define KEY_LOCK_ENABLED "lockEnabled"
#define KEY_BACKGROUND "background"
#define KEY_MODE "mode"
#define KEY_THEMES "themes"
#define KEY_IMAGE_TRANSITION_EFFECT "imageTransitionEffect"
#define KEY_IMAGE_SWITCH_INTERVAL "imageSwitchInterval"

#define GSETTINGS_SCHEMA_SCREENSAVER_DEFAULT "org.ukui.screensaver-default"
#define KEY_SHOW_REST_TIME "showRestTime" // old
#define KEY_SHOW_CUSTOM_REST_TIME "showCustomRestTime"
#define KEY_SHOW_UKUI_REST_TIME "showUkuiRestTime"
#define KEY_CYCLE_TIME "cycleTime"
#define KEY_AUTOMATIC_SWITCHING_ENABLE "automaticSwitchingEnabled"
#define KEY_BACKGROUND_PATH "backgroundPath"
#define KEY_MYTEXT "mytext"
#define KEY_TEXT_IS_CENTER "textIsCenter"
#define KEY_SHOW_MESSAGE_ENABLED "showMessageEnabled"
#define KEY_MESSAGE_NUMBER "messageNumber"
#define KEY_VIDEO_FORMAT "videoFormat"
#define KEY_VIDEO_HEIGHT "videoHeight"
#define KEY_VIDEO_WIDTH "videoWidth"
#define KEY_VIDEO_PATH "videoPath"
#define KEY_VIDEO_SIZE "videoSize"
#define KEY_SUB_MODE "subMode"
#define KEY_MUSIC_FOLLOW_SYSTEM "musicFollowSystem"
#define KEY_WEATHER_POSITION "weatherPosition"
#define KEY_MUSIC_PATH "musicPath"
#define KEY_PHOTO_CYCLE_MODE "photoCycleMode"
#define KEY_PHOTO_CYCLE_INTERVAL "photoCycleInterval"
#define KEY_PHOTO_DEFAULT_PATH "photoDefaultPath"
#define KEY_PHOTO_CUSTOM_PATH "photoCustomPath"
#define KEY_PHOTO_SELECT_PATH "photoSelectPath"
#define KEY_URL_ADDRESSS "urlAddress"
#define KEY_VIDEO_FORMAT "videoFormat"
#define DEFAULT_PHOTO_PATH "/usr/share/backgrounds"
#define LOCAL_WALLPAPER_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)\
%QStringLiteral("/ukui-control-center/wallpaperData/")
#define KEY_SAVER_DATETIME_ENADLE "saverDatetimeEnable"

#define GSETTINGS_SCHEMA_POWER_MANAGER "org.ukui.power-manager"
#define KEY_LOCK_SUSPEND "lockSuspend"
#define KEY_LOCK_HIBERNATE "lockHibernate"
#define KEY_LOCK_BLANKSCREEN "lockBlankScreen"
#define KEY_SLEEP_COMPUTER_AC "sleepComputerAc"
#define KEY_SLEEP_DISPLAY_AC "sleepDisplayAc"
#define KEY_BUTTON_LID_AC "buttonLidAc"
#define KEY_BUTTON_POWER "buttonPower"

#define GSETTINGS_SCHEMA_MATE_BG "org.mate.background"
#define KEY_PICTURE_FILENAME "pictureFilename"
#define KEY_PICTURE_OPTIONS "pictureOptions"
#define KEY_PRIMARY_COLOR "primaryColor"

#define GSETTINGS_SCHEMA_TIME_TYPE "org.ukui.control-center.panel.plugins"
#define KEY_HOUR_SYSTEM "hoursystem"
#define KEY_DATE "date"

#define GSETTINGS_SCHEMA_STYLE "org.ukui.style"
#define KEY_SYSTEM_FONT_SIZE "systemFontSize"
#define GLIB_KEY_SYSTEM_FONT_SIZE "system-font-size"
#define KEY_THEME_COLOR "themeColor"
#define KEY_SYSTEM_FONT "systemFont"
#define KEY_MENU_TRANSPARENCY "menuTransparency"
#define KEY_STYLE_NAME "styleName"

#define GSETTINGS_SCHEMA_SESSION "org.ukui.session"
#define KEY_SESSION_IDLE "idleDelay"
#define KEY_SESSION_LOGOUT_MUSIC "logoutMusic"
#define KEY_SESSION_POWEROFF_MUSIC "poweroffMusic"

#define GSETTINGS_SCHEMA_PERIPHERALS_KEYBOARD "org.ukui.peripherals-keyboard"
#define KEY_CAPSLOCK_STATUS "capslockState"

#define GSETTINGS_SCHEMA_MEDIAKEY_SCHEMA "org.ukui.SettingsDaemon.plugins.media-keys"
#define KEY_AREA_SCREENSHOT "areaScreenshot"
#define KEY_AREA_SCREENSHOT2 "areaScreenshot2"
#define KEY_SCREEN_SHOT "screenshot"
#define KEY_SCREEN_SHOT2 "screenshot2"
#define KEY_WINDOW_SCREENSHOT "windowScreenshot"

#define GSETTINGS_SCHEMA_KYLIN_WEATHER_SCHEMA "org.kylin-weather.settings"
#define KEY_WEATHER "weather"

#define GSETTINGS_SCHEMA_MEDIAKEY_STATE_SCHEMA "org.ukui.SettingsDaemon.plugins.media-keys-state"
#define KEY_RFKILL_STATE "rfkillState"

#define WORKING_DIRECTORY "/usr/share/ukui-screensaver"

#define SUBWND_COUNT_MAX 1024

#define SS_DBUS_SERVICE "org.ukui.ScreenSaver"
#define SS_DBUS_PATH "/"
#define SS_DBUS_INTERFACE "org.ukui.ScreenSaver"

#define SL_SESSION_DBUS_SERVICE "org.ukui.ScreenSaver.session.Screenlock"
#define SL_SESSION_DBUS_PATH "/Screenlock"
#define SS_SESSION_DBUS_SERVICE "org.ukui.ScreenSaver.session.Screensaver"
#define SS_SESSION_DBUS_PATH "/Screensaver"

enum SessionStatus
{
    SESSION_AVAILABLE = 0,
    SESSION_INVISIBLE = 1,
    SESSION_BUSY = 2,
    SESSION_IDLE = 3
};

enum LockStatus
{
    SCREEN_SAVER_STATUS,
    LOCK,
    SAVER_LOCK,
    UNLOCK,
};

#define SM_DBUS_SERVICE "org.gnome.SessionManager"
#define SM_DBUS_PATH "/org/gnome/SessionManager/Presence"
#define SM_DBUS_INTERFACE "org.gnome.SessionManager.Presence"

#define SESSION_DBUS_SERVICE     "org.gnome.SessionManager"
#define SESSION_DBUS_PATH        "/org/gnome/SessionManager"
#define SESSION_DBUS_INTERFACE   "org.gnome.SessionManager"

#define DM_DBUS_SERVICE "org.freedesktop.DisplayManager"
#define DM_DBUS_PATH "/org/freedesktop/DisplayManager"
#define DM_DBUS_INTERFACE "org.freedesktop.DisplayManager"
#define DM_SEAT_INTERFACE "org.freedesktop.DisplayManager.Seat"
#define DM_SESSION_INTERFACE "org.freedesktop.DisplayManager.Session"

#define SSWND_DBUS_SERVICE "org.ukui.ScreenSaverWnd"
#define SSWND_DBUS_PATH "/"
#define SSWND_DBUS_INTERFACE "org.ukui.ScreenSaverWnd"

#define DEFAULT_BACKGROUND_PATH "/usr/share/backgrounds/1-openkylin.jpg"

#define G_FONT_SIZE (11.0)
#define DEFAULT_FONT_SIZE (10.0)

#define XSCREENSAVER_DIRNAME "/usr/lib/xscreensaver"

enum LOGINOPT_TYPE
{
    LOGINOPT_TYPE_PASSWORD = 0,  // 密码
    LOGINOPT_TYPE_FACE,          // 人脸
    LOGINOPT_TYPE_FINGERPRINT,   // 指纹
    LOGINOPT_TYPE_IRIS,          // 虹膜
    LOGINOPT_TYPE_VOICEPRINT,    // 声纹
    LOGINOPT_TYPE_FINGERVEIN,    // 指静脉
    LOGINOPT_TYPE_GENERAL_UKEY,  // 普通的ukey
    LOGINOPT_TYPE_ADVANCED_UKEY, // 高阶的ukey
    LOGINOPT_TYPE_QRCODE,        // 二维码
    LOGINOPT_TYPE_CUSTOM,        // 第三方
    LOGINOPT_TYPE_OTHERS,        // 其他
    LOGINOPT_TYPE_COUNT
};

#define BIOMETRIC_PAM_DOUBLE "BIOMETRIC_PAM_DOUBLE"
#define BIOMETRIC_PAM "BIOMETRIC_PAM"
#define BIOMETRIC_PAM_QRCODE "BIOMETRIC_PAM_QRCODE"
#define BIOMETRIC_IGNORE "BIOMETRIC_IGNORE"
#define BIOMETRIC_SUCCESS "BIOMETRIC_SUCCESS"

namespace PamAuth
{

enum PromptType
{
    PromptTypeQuestion,
    PromptTypeSecret
};
enum MessageType
{
    MessageTypeInfo,
    MessageTypeError
};

} // namespace PamAuth

// usd快捷键字符串定义（dbus接口参数）
#define MUTE_KEY "MUTE_KEY"
#define VOLUME_DOWN_KEY "VOLUME_DOWN_KEY"
#define VOLUME_UP_KEY "VOLUME_UP_KEY"
#define BRIGHT_UP_KEY "BRIGHT_UP_KEY"
#define BRIGHT_DOWN_KEY "BRIGHT_DOWN_KEY"
#define TOUCHPAD_ON_KEY "TOUCHPAD_ON_KEY"
#define TOUCHPAD_OFF_KEY "TOUCHPAD_OFF_KEY"
#define TOUCHPAD_KEY "TOUCHPAD_KEY"
#define AREA_SCREENSHOT_KEY "AREA_SCREENSHOT_KEY"
#define SCREENSHOT_KEY "SCREENSHOT_KEY"
#define WINDOW_SCREENSHOT_KEY "WINDOW_SCREENSHOT_KEY"

#define UNIAUTH_DBUS_SERVICE "org.ukui.UniauthBackend"
#define UNIAUTH_DBUS_PATH "/org/ukui/UniauthBackend"
#define UNIAUTH_DBUS_INTERFACE "org.ukui.UniauthBackend"
#define UNIAUTH_GREETERCONFIG_DBUS_PATH "/org/ukui/GreeterConfig"

#define FD_DBUS_SERVICE "org.freedesktop.DBus"
#define FD_DBUS_PATH "/org/freedesktop/DBus"
#define FD_DBUS_INTERFACE "org.freedesktop.DBus"
#define FD_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"

#define ACCOUNTS_DBUS_SERVICE "org.freedesktop.Accounts"
#define ACCOUNTS_DBUS_PATH "/org/freedesktop/Accounts"
#define ACCOUNTS_DBUS_INTERFACE "org.freedesktop.Accounts"

#define ACCOUNTS_USER_DBUS_INTERFACE "org.freedesktop.Accounts.User"
#define ACCOUNTS_USER_LDM_SERVICE_INTERFACE "org.freedesktop.DisplayManager.AccountsService"

enum Action
{
    PowerSwitchUser,
    PowerHibernate,
    PowerSuspend,
    PowerMonitorOff,
    PowerLogout,
    PowerUpgradeThenRboot,
    PowerReboot,
    PowerUpgradeThenShutdown,
    PowerShutdown,
    TestAction,
};

enum BinaryPromptType
{
    BinaryPromptAuthType,      //认证类型
    BinaryPromptNumber,        //数字类型消息，暂不使用
    BinaryPromptMessage,       //消息类型
    BinaryPromptImage,         //图片类型，暂不使用
    BinaryPromptQrcodeStr,     //二维码字符串
    BinaryPromptPromptMessage, //绑定密码框中的消息
    BinaryPromptErrorMessage,   //绑定密码框下方的错误消息
    BinaryPromptEmergMsg       //OTP紧急口令
};

enum SaverStatesForAbnormal
{
    SaverUNDEFINE = -1,
    SaverISSTARTUP,
    SaverAUTHCOMPLETE,
};

#endif // DEFINETYPES_H
