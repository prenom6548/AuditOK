/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "dbusclient.h"
#include <QList>
#include <QString>
#include <QClipboard>
#include <QSize>
#include <QMap>
#include <QGSettings>
#include "typedef.h"
#include "metatypes.h"
#include "singleton.h"

class KmreWindow;

using namespace kmre;

class KmreWindowManager : public QObject, public SingletonP<KmreWindowManager>
{
    Q_OBJECT

public:
    void initialize();
    void focusWindowDisplay(int displayId);
    void handleNotification(const QString &appName, const QString &pkgName, const QString &text, 
                            int winId, bool hide, bool call, const QString &title);
    int getCurrentSystemLanguage() {return mSystemLanguage;}
    int getCurrentSystemTheme() {return mSystemTheme;}
    int getRunningAppNum();
    QWidget* getCurrentFocusedWindow();
    bool getSystemPanelHeight(int &height);
    bool isAppAlreadyLaunched(const QString &pkgName);
    void setSettingsForAndroid();
    void closeApp(const QString &pkgName);
    bool inhibitSystemLockScreen(bool inhibit);
    void waitForExit(int msecond);
    void appUninstalled(const QString &pkgName);

signals:
    void lockScreen(bool lock);
    // dbus signals
    void containerEnvBooted(bool status, const QString &errInfo);
    void updateAppStatus(const QString &pkgName, int status, int type);
    void syncFiles(int type, AndroidMetaList metas, int totalNum);
    void postAppMultipliers(const QString &apps);
    void postResponseInfo(int id, const QString &pkgName, const QString &category, int ret, const QString &info);

public slots:
    // dbus interface
    void start() {/* do nothing */}
    void launchApp(const QString &pkgName, int width, int height);
    void activeApp(const QString &pkgName);
    QString getRunningAppList();
    QString getAppMultipliers();
    void answerCall(bool accept);
    //--------------------------------
    void onEnableInputMethod(int displayId, const QString &pkgName, bool enable, int x, int y);
    void onSwitchMultipler(const QString &pkgName, int displayId, bool enable);
    void onAppCloseSucceed(const QString &pkgName);
    void onAppLaunchSucceed(const QString &appName, const QString &pkgName, bool result, 
                            int displayId, int width, int height, int density, bool fullscreen, 
                            bool exists, int appResumed);

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;

private slots:
    void onKmreDockerEnvStopped(QString containerName);
    void onSystemPanelChanged(const QString &key);
    void onSystemUkuiStyleChanged(const QString &key);

private:
    void initSystemSettings();
    void initKmreSettings();
    void registerWindowDbusService();
    bool createWindow(QString pkgName, QString appName, int displayId, int width, int height, int appResumed);
    bool activeWindow(QString pkgName);
    bool resumeWindow(QString pkgName);
    void destroyAllAppWindows();
    void quitSafely();

private:
    KmreWindowManager();
    ~KmreWindowManager();
    std::mutex mWinMapMtx;
    QMap<QString, KmreWindow*> mWindowMap;
    kmre::DbusClient *mDbusClient;
    QTimer *mExitTimer;

    quint32 mInhibitValue;//阻止锁屏cookie
    up<QDBusInterface> mGnomeDbus;//锁屏dbus

    up<QGSettings> mSystemPanelGSettings;
    up<QGSettings> mSystemUkuiStyleGSettings;
    enum LANGUAGE{
        eLauguage_CN = 0,// chinese
        eLauguage_EN,// english
    }mSystemLanguage;
    enum THEME{
        eTheme_Light = 0,// light theme
        eTheme_Dark,// dark theme
    }mSystemTheme;

    friend SingletonP<KmreWindowManager>;
};
