/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef WIDGET_H
#define WIDGET_H

#ifdef signals
#undef signals
#endif

#include <canberra.h>
#include <glib.h>
#include <gio/gio.h>
#include <QWidget>
#include <QObject>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QQueue>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QDesktopWidget>
#include <QStandardPaths>
#include <QFile>
#include <QGSettings>
#include "notifyreceiveinfo.h"
#include "toptransparentwidget.h"
#include "sqlinfodata.h"
#include "platformInfo.h"
#include <QTimer>
#include <KApplicationTrader>
#include <QFileInfo>
#include <KSharedConfig>
#include <KConfigGroup>

#define SYSTEM_DESKTOP_PATH      "/etc/xdg/autostart/"

//多屏链接dbus
#define KYLIN_SETTINGDAEMON_NAME          "org.ukui.SettingsDaemon"
#define KYLIN_SETTINGDAEMON_PATH          "/org/ukui/SettingsDaemon/xrandr"
#define KYLIN_SETTINGDAEMON_INTERFACE     "org.ukui.SettingsDaemon.xrandr"
#define KYLIN_SETTINGDAEMON_METHOD        "getScreenMode"

//窗管全屏模式dbus
#define KYLIN_KWIN_NAME          "org.ukui.KWin"
#define KYLIN_KWIN_PATH          "/KWin"
#define KYLIN_KWIN_INTERFACE     "org.ukui.KWin"
#define KYLIN_KWIN_METHOD        "haveFullScreenWindow"

//免打扰模式
#define UKUI_VOLUME_BRIGHTNESS_GSETTING_ID "org.ukui.quick-operation.panel"
#define KYLIN_DISTURB_GSETTING_VALUE_NOTIFYCLOSE "disturbmodenotifyclose"

//弹窗模式：多弹窗、单弹窗
#define UKUI_NOTIFICATION_DEMO_GSETTING_ID "org.ukui.notification.demo"
#define KYLIN_NOTIFICATION_DEMO_CLOSE_MODE_KEY "popupwindowmode"
#define AUTO_START_DISTURB "autostartdisturb"
#define TIME_START "timestart"
#define TIME_END   "timeend"
#define FULL_SCREEN_DISTURB "fullscreendisturb"
#define ALARM_CLOCK_DISSTURB "alarmclockdisturb"
#define PROJECTIN_SCREEN_DISTURB "projectionscreendisturb"

//弹窗是否禁用
#define CONTROL_CENTER_GSETTING_NOTIFY_MAIN_SWITCH "org.ukui.control-center.notice"
#define CONTROL_GSETTING_NOTIFY_MAIN_KEY "enableNotice"

//应用弹窗是否禁用
#define CONTROL_CENTER_GSETTING_PATH "org.ukui.control-center.noticeorigin"
#define CONTROL_CERTER_DYNAMIC_GSETTING_PATH "/org/ukui/control-center/noticeorigin/"
#define CONTROL_CENTER_GSETTING_NOTIFYCATION "messages"
#define CONTROL_CENTER_GSETTING_SCREEN_NOTIFYCATION "showOnScreenlock"
#define CONTROL_CENTER_GSETTING_VOICE_NOTIFYCATION "voice"
#define CONTROL_CENTER_GSETTING_STYLE_NOTIFYCATION "notificationStyle"
#define NOTIFYCATION_SHOW_STYLE_MUTATIVE "mutative"
#define NOTIFYCATION_SHOW_STYLE_ALWAYS "always"
#define NOTIFYCATION_SHOW_STYLE_NONE "none"

#define CONTROL_PATH_SOUNDS_SCHEMA "org.ukui.sound"
#define CONTROL_EVENT_SOUNDS_KEY "event-sounds"

#define KEYBINDINGS_CUSTOM_SCHEMA "org.ukui.media.sound"
#define KEYBINDINGS_CUSTOM_DIR "/org/ukui/sound/keybindings/"

#define FILENAME_KEY "filename"
#define NAME_KEY "name"

//freedesktop notifications 协议中 hints字段
#define DISPLAY_LOCATION  "display-location"

static const QString NotificationsDBusService = "org.freedesktop.Notifications";
static const QString NotificationsDBusPath = "/org/freedesktop/Notifications";

class NotifyManager : public QObject
{
    Q_OBJECT

public:
    enum ClosedReason
    {
        TIMEOUT = 1,
        CLOSEDBYUSER = 2,
        CLOSEDBYAPP = 3,
        UNDEFINED = 4,
        HANDLEDBYUSER = 20
    };
    enum ScreenMode {
        FirstScreenMode = 0,
        CloneScreenMode,
        ExtendScreenMode,
        SecondScreenMode
    };

    NotifyManager(QObject *parent = nullptr);
    ~NotifyManager();

Q_SIGNALS:
    // Standard Notifications dbus implementation
    void ActionInvoked(uint, const QString &);
    void NotificationClosed(uint, uint);

public Q_SLOTS:
    // Standard Notifications dbus implementation
    void CloseNotification(uint id);
    QStringList GetCapabilities();
    void GetServerInformation(QString &name, QString &vendor, QString &version, QString &specVersion);
    // new notify will be received by this slot
    uint Notify(
        const QString &,
        uint replacesId,
        const QString &,
        const QString &,
        const QString &,
        const QStringList &,
        const QVariantMap,
        int);
    uint NotifyPy(
        const QString &,
        int replacesId,
        const QString &,
        const QString &,
        const QString &,
        const QVariantList &,
        const QVariantMap,
        int);
    uint NotifySimple(
        const QString &appName,
        uint replacesId,
        const QString &appIcon,
        const QString &summary,
        const QString &body,
        int expireTimeout);

public:
    topTransparentWidget *m_pTopWidget = nullptr;

private:
    void registerAsService();
    void nextShowAction();
    void initGsettingValue();
    QList<char *> listExistsPath();

    void appNotifySound(const QVariantMap hints);
    bool getControlCentorAppNotify(QString appName);
    bool getScreenAppNotify(QString appName);
    bool getVoiceAppNotify(QString appName);
    bool notifySwitch(QString appName);

    QString getNotifyShowStyle(QString appName);
    bool getScreenLockStates();
    void showToSidebar(NotifyReceiveInfo *notifyInfo);   //将弹窗里所有消息收纳到侧边栏

    void getAppinfo();

private:
    QTranslator *m_pTranslator;

    QGSettings *m_pNodisturbModeGsetting     = nullptr;
    QGSettings *m_pPopupWidgetModeGsetting   = nullptr;
    QGSettings *m_pControlNotifyMainGsetting = nullptr;
    QGSettings *m_pAppNotifyGseting          = nullptr;

    bool m_bPopupWidgetModeStatus  = false;       //通知模式标志位：true：单弹窗 false:多弹窗
    bool m_bNoDisturbMode          = false;       //免打扰标志位
    bool m_bNotifyMainSwitch       = true;        //控制面板通知屏蔽开关
    bool m_autoStartDisturb        = false;       //设置某个时间段开启勿扰模式

    bool m_fullScreenDisturb       = false;       //全屏模式下勿扰
    bool m_alarmClockDisturb       = false;       //勿扰模式下是否允许闹钟提示
    bool m_projectionScreenDisturb = false;       //镜像屏幕或者投影时勿扰
    bool m_isDisplay               = true;

    QStringList existList; // 保存已存在的应用，防止重复
    QMap <QString, QString> appInfoMap; // 储存Name和desktopEntryName键值对,key为Name， value为desktopEntryName

    QString m_timeStart;
    QString m_timeEnd;

    ca_context *m_pCaContext;
    QString m_model;
    sqlInfoData *m_psqlInfoData;
    uint m_counter = 0;

private slots:
    void popupItemWidgetDismissed(uint id, uint closeReson);
    void popupItemWidgetActionInvoked(uint id, QString actionKey);
    void quitHandle();
};
#endif // WIDGET_H
