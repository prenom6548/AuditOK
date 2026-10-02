/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *               2010-2016 LXQt team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 **/
#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include <window-manager.h>
#include <QQuickItem>

#include "ukuimodule.h"
#include "freedesktophelper.h"

#include <QDBusInterface>
#include <QString>
#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QEventLoop>
#include <QProcess>
#include <QVector>
#include <QMap>
#include <QTimer>
#include <QMediaPlayer>
#include <QDBusInterface>
#include <map>
#include <QGSettings/QGSettings>

class XdgDesktopFile;
class IdleWatcher;

typedef QMap<QString, UkuiModule *> ModulesMap;
typedef QMapIterator<QString, UkuiModule *> ModulesMapIterator;

using WindowManager = UkuiQuick::WindowManager;

#define UKUI_LITE_CONFIG "/usr/share/ukui/UKUI-Lite/UKUI-Lite.ini"
#define GLOBAL_MANAGER_SCHEMAS "org.ukui.SettingsDaemon.plugins.globalManager"
#define LITE_FUNCITON_KEY "ukuiLiteFunction"
#define LITE_ANIMATION_KEY "ukuiLiteAnimation"
#define UKUI_LITE "UKUI-Lite"

class ModuleManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

private:
    ModuleManager(QObject *parent = nullptr);

public:
    static ModuleManager *instance(QObject *parent = nullptr);
    ~ModuleManager() override;

    void init(const QString &arg);

    void startProcess(const QString &name, bool detach);

    void stopProcess(const QString &name);

    bool startProcess(const QString &name, const QStringList &args);

    void startup();

    void dostartwm();

    void startupfinished(const QString &appName, const QString &string);

    // Qt5 users native event filter
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

    static void insertStartupList(QString &&str);

    static bool isProgramStarted(const QString &&str);

    void ChkScreenLockStartup();

    void startNotifPhase();

    void setIsShutingDown(bool value);

    bool getIsShutingDown() const;

    bool quickStart();

    void monitorQuickStart();

private:
    void initConn();
    bool startModuleTimer(QTimer *timer, int i);
    void playBootMusic(bool arg);
    void startProcess(
        const XdgDesktopFile &file, bool required, const QStringList &arguments = QStringList(), bool keepArgs = false);
    void constructStartupList();
    bool autoRestart(const XdgDesktopFile &file);
    void sequenceStart(XdgDesktopFileList &fileList);
    XdgDesktopFile selectOneAppToStart(XdgDesktopFileList &fileList);
    void startUkuiSplash();
    void endUkuiSplash();
    bool openAppWithAppManager(const XdgDesktopFile &file);

    bool checkIsTabletmode();

    bool isSpecialMode = false;
    void constructSpecialList();
    QGSettings *rcGs;
    XdgDesktopFileList mspecialModeApp;

    QSet<QString> getUKUILiteApps();
    void setUKUILIte(QString UkuiLite);
public slots:
    void startCompsite();
    void logout(bool doExit);
    void timerUpdate();
    void timeup();
    void weakup(bool arg);
    // void stateChanged(QMediaPlayer::State state);
    void onLockScrenUnlock();

private slots:
    void restartModules(int exitCode, QProcess::ExitStatus exitStatus);
    void statusChanged(bool enabled);
    void onSessionDbusActiveChanged(const QString &strServerName, bool isActive);

Q_SIGNALS:
    void moduleStateChanged(QString moduleName, bool state);
    void finished();
    void usdfinished();
    void wmfinished();
    void panelfinished();
    void desktopfinished();
    void screensaverfinished();
    void screensaverAuthenticateCompete();
    void screensaverServiceStatusChanged(bool isLockStartup);
    void saverBackendAbnormaled();
    void windowAdded(QString winId);
    void windowRemoved(QString winId);
private:
    QTimer *tusd = new QTimer();
    QTimer *twm = new QTimer();
    QTimer *tpanel = new QTimer();
    QTimer *tdesktop = new QTimer();
    QTimer *tLockScreen = new QTimer();

    bool isPanelStarted = false;
    bool isDesktopStarted = false;
    bool isWMStarted = false;
    bool isCompsiteStarted = false;
    bool isLockScreenStarted = false;

    bool isScreenLockStartup = false;

    bool isStartuped = false;

    bool isWayland = false;
    bool wmFound = false;

    bool isDirectInstall = false;
    bool isShutingDown = false;
    bool mNotificationFlag = true;
    bool mScreensaverFirstExit = true;  // 标记是否是第一次退出，用于重新拉起

    static std::map<const QString, int> m_startupMap;

    QSet<QString> mUKUILiteApps;
    ModulesMap mNameMap;
    XdgDesktopFileList mInitialization;
    XdgDesktopFile mWindowManager;
    XdgDesktopFile mPanel;
    XdgDesktopFile mFileManager;
    XdgDesktopFile mTableDesk;
    XdgDesktopFile mLockScreen;
    XdgDesktopFileList mDesktop;
    XdgDesktopFileList mApplication;
    XdgDesktopFileList mForceApplication;
    XdgDesktopFileList mNotification;
    XdgDesktopFileList m_appStarted; // 用来记录已经启动的app
    XdgDesktopFileList m_userAutoStartApp;

    QTimer *sequenceTimer = nullptr;
    XdgDesktopFileList *sequenceFileList = nullptr;

    QString m_strSessionPath = "";
    static ModuleManager *m_singleInstance;
};

#endif // MODULEMANAGER_H
