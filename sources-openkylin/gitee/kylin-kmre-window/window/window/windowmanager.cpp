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

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QMimeData>
#include <QApplication>
#include <future>
#include <chrono>

#include "app_control_manager.h"
#include "windowmanager.h"
#include "kmrewindow.h"
#include "displaybackend.h"
#include "eventmanager.h"
#include "appsettings.h"
#include "kmreenv.h"
#include "preferences.h"
#include "clipboard.h"
#include "screencapture.h"
#include "window_dbus.h"
#include "signalevent.h"
#include "screensharing.h"
#include <syslog.h>

#define ORG_UKUI_STYLE      "org.ukui.style"
#define SERVICE_INTERFACE "cn.kylinos.Kmre.Window"
#define SERVICE_PATH "/cn/kylinos/Kmre/Window"

KmreWindowManager::KmreWindowManager()
    : QObject(nullptr)
    , mDbusClient(kmre::DbusClient::getInstance())
    , mExitTimer(nullptr)
    , mInhibitValue(0)
{
    QDBusConnection::systemBus().connect("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre",
        "Stopped", this, SLOT(onKmreDockerEnvStopped(QString)));
}

KmreWindowManager::~KmreWindowManager()
{

}

void KmreWindowManager::initialize()
{
    initSystemSettings();

    ScreenCaptureDbus::getInstance();
    AppSettings::getInstance().initAndroidConfigFile();
    AppControlManager::getInstance();
    ScreenSharing::getInstance();
    DisplayBackend::getInstance();
    EventManager::getInstance();
    Clipboard::getInstance();

    registerWindowDbusService();

    syslog(LOG_DEBUG, "[%s] Kmre window boot finished.", __func__);
}

bool KmreWindowManager::isAppAlreadyLaunched(const QString &pkgName)
{
    lgm lk(mWinMapMtx);
    return mWindowMap.contains(pkgName);
}

int KmreWindowManager::getRunningAppNum() 
{
    lgm lk(mWinMapMtx);
    return mWindowMap.size();
}

QString KmreWindowManager::getRunningAppList()
{
    QString appStr;
    lgm lk(mWinMapMtx);
    QStringList appList = mWindowMap.keys();
    for (auto &app : appList) {
        appStr += app;
        appStr += ",";
    }

    syslog(LOG_DEBUG, "[%s] Running apps: %s", __func__, appStr.toStdString().c_str());
    return appStr;
}

bool KmreWindowManager::createWindow(QString pkgName, QString appName, int displayId, int width, int height, int appResumed)
{
    if (!AppSettings::getInstance().isAppSupportShow(pkgName)) {
        syslog(LOG_ERR, "[%s] App '%s' can not be show!", __func__, pkgName.toStdString().c_str());
        return false;
    }

    {
        lgm lk(mWinMapMtx);
        if (!mWindowMap.contains(pkgName)) {
            mWindowMap[pkgName] = nullptr;// 
        }
        else {
            syslog(LOG_DEBUG, "[%s] App '%s' is already launched.", __func__, pkgName.toStdString().c_str());
            if (mWindowMap[pkgName]) {
                mWindowMap[pkgName]->updateDisplay(appName, displayId, width, height);
                mWindowMap[pkgName]->showMainWindow();
                //mWindowMap[pkgName]->onAppResumed();
            }
            else {
                syslog(LOG_DEBUG, "[%s] app '%s' window have not been created, waiting now...", __func__, pkgName.toStdString().c_str());
                // should never reached here !
            }
            return true;
        }
    }

    KmreWindow *window = new KmreWindow(displayId, width, height, appName, pkgName);
    if (window->initialize() < 0) {
        delete window;
        window = nullptr;
    }

    lgm lk(mWinMapMtx);
    if (window) {
        connect(this, SIGNAL(lockScreen(bool)), window, SLOT(onLockScreen(bool)));

        mWindowMap[pkgName] = window;// 
        syslog(LOG_DEBUG, "[%s] Create window app '%s' succeed.", __func__, pkgName.toStdString().c_str());
        return true;
    }
    else {
        mWindowMap.remove(pkgName);
        syslog(LOG_ERR, "[%s] Create window app '%s' failed!", __func__, pkgName.toStdString().c_str());
        return false;
    }

}

bool KmreWindowManager::activeWindow(QString pkgName)
{
    lgm lk(mWinMapMtx);
    if (mWindowMap.contains(pkgName) && mWindowMap[pkgName]) {
        if (!mWindowMap[pkgName]->isActiveWindow()) {
            mWindowMap[pkgName]->showMainWindow();
            return true;
        }
    }

    syslog(LOG_ERR, "[%s] Can not find app '%s' window", __func__, pkgName.toStdString().c_str());
    return false;
}

bool KmreWindowManager::resumeWindow(QString pkgName)
{
    lgm lk(mWinMapMtx);
    if (mWindowMap.contains(pkgName) && mWindowMap[pkgName]) {
        mWindowMap[pkgName]->onAppResumed();
        return true;
    }

    syslog(LOG_ERR, "[%s] Can not find app '%s' window", __func__, pkgName.toStdString().c_str());
    return false;
}

void KmreWindowManager::onKmreDockerEnvStopped(QString containerName)
{
    syslog(LOG_INFO, "[%s] containerName = '%s'", __func__, containerName.toStdString().c_str());
    if (containerName == kmre::utils::makeContainerName()) {
        if (mExitTimer) {
            mExitTimer->stop();
        }
        syslog(LOG_INFO, "[%s] Kmre env stopped. Exit now!", __func__);
        KmreEnv::kmreEnvStopped();
        quitSafely();
    }
}

void KmreWindowManager::waitForExit(int msecond)
{
    if (!mExitTimer) {
        mExitTimer = new QTimer(this);
        mExitTimer->setSingleShot(true);
        connect(mExitTimer, &QTimer::timeout, this, [=] {
            onKmreDockerEnvStopped(kmre::utils::makeContainerName());
        });
        mExitTimer->start(msecond);
    }
}

// called by startapp
void KmreWindowManager::activeApp(const QString &pkgName)
{
    lgm lk(mWinMapMtx);
    if (mWindowMap.contains(pkgName) && mWindowMap[pkgName]) {
        mWindowMap[pkgName]->showMainWindow();
        syslog(LOG_INFO, "[%s] Active app succeed.", __func__);
    }
}

// called by startapp
void KmreWindowManager::launchApp(const QString &pkgName, int width, int height)
{
    if (pkgName.isEmpty() || (width <= 0) || (height <= 0)) {
        syslog(LOG_ERR, "[%s] Invalid app information!", __func__);
        return;
    }

    syslog(LOG_INFO, "[%s] Start app '%s', width = %d, height = %d", 
        __func__, pkgName.toStdString().c_str(), width, height);

    AppSettings::getInstance().updateSettings();
    
    if (createWindow(pkgName, "", -1, width, height, 0)) {
        if (mWindowMap[pkgName]) {
            mWindowMap[pkgName]->startBootTimer();
        }
    }
}

void KmreWindowManager::onAppLaunchSucceed(const QString &appName, const QString &pkgName, bool result, 
                                        int displayId, int width, int height, int density, bool fullscreen, 
                                        bool exists, int appResumed)
{
    Q_UNUSED(density);
    Q_UNUSED(fullscreen);

    if (pkgName.isEmpty() || (displayId < 0) || (width <= 0) || (height <= 0)) {
        syslog(LOG_ERR, "[%s] Invalid app information!", __func__);
        return;
    }

    if (!result) {
        syslog(LOG_ERR, "[%s] Android app '%s' launch failed!", __func__, pkgName.toStdString().c_str());
        return;
    }

    syslog(LOG_INFO, "[%s] Launch app '%s', displayId = %d, width = %d, height = %d, exists = %d, appResumed = %d", 
        __func__, pkgName.toStdString().c_str(), displayId, width, height, exists, appResumed);

    if (createWindow(pkgName, appName, displayId, width, height, appResumed)) {
        if (mWindowMap[pkgName]) {
            mWindowMap[pkgName]->stopBootTimer();
            //mWindowMap[pkgName]->forceUpdating();
            mWindowMap[pkgName]->appLaunchCompleted();
        }
    }
}

void KmreWindowManager::onAppCloseSucceed(const QString &pkgName)
{
    KmreWindow* closedApp = nullptr;
    {
        lgm lk(mWinMapMtx);
        if (mWindowMap.contains(pkgName) && mWindowMap[pkgName]) {// app crashed in android!
            closedApp = mWindowMap[pkgName];
            //mWindowMap.remove(pkgName);//don't remove app window here
        }
    }
    
    if (closedApp) {
        syslog(LOG_ERR, "[%s] App '%s' closed in android!", __func__, pkgName.toStdString().c_str());
        //delete closedApp;
        //closedApp->closeWindow(true);
        closedApp->showCrashedHintDialog();
    }
}

void KmreWindowManager::closeApp(const QString &pkgName)
{
    KmreWindow* window = nullptr;
    lgm lk(mWinMapMtx);
    if (mWindowMap.contains(pkgName)) {
        window = mWindowMap[pkgName];
        mWindowMap.remove(pkgName);
        syslog(LOG_DEBUG, "[%s] Removed app '%s' window.", __func__, pkgName.toStdString().c_str());
    }
}

void KmreWindowManager::appUninstalled(const QString &pkgName)
{
    KmreWindow* window = nullptr;
    {
        lgm lk(mWinMapMtx);
        if (mWindowMap.contains(pkgName)) {
            window = mWindowMap[pkgName];
        }
    }

    if (window) {
        syslog(LOG_DEBUG, "[%s] App(%s) uninstalled, close it now.", __func__, pkgName.toStdString().c_str());
        window->closeWindow(true);
    }
}

void KmreWindowManager::destroyAllAppWindows()
{
    syslog(LOG_INFO, "[%s] Destroy all app windows...", __func__);
    lgm lk(mWinMapMtx);
    for (auto window : mWindowMap) {
        delete window;
    }
    mWindowMap.clear();
}

QString KmreWindowManager::getAppMultipliers()
{
    return AppSettings::getInstance().getAppMultipliersList();
}

QWidget* KmreWindowManager::getCurrentFocusedWindow()
{
    lgm lk(mWinMapMtx);
    for (auto &window : mWindowMap) {
        if (window && window->isWindowFocused()) {
            return window;
        }
    }

    return nullptr;
}


void KmreWindowManager::onEnableInputMethod(int displayId, const QString &pkgName, bool enable, int x, int y)
{
    lgm lk(mWinMapMtx);
    if (mWindowMap.contains(pkgName) && mWindowMap[pkgName]) {
        mWindowMap[pkgName]->onInputMethodCommands(displayId, enable, x, y);
    }
}

void KmreWindowManager::onSystemPanelChanged(const QString &key)
{
    if (key == "panelsize") {
        lgm lk(mWinMapMtx);
        for (auto window : mWindowMap) {
            if (window) {
                window->resizeWindow();
            }
        }
    }
}

void KmreWindowManager::onSystemUkuiStyleChanged(const QString &key)
{
    if (key == "styleName") { 
        syslog(LOG_DEBUG, "[%s] System theme changed.", __func__);
        mSystemTheme = eTheme_Light;// light theme default
        QString styleName = mSystemUkuiStyleGSettings->get("style-name").toString();
        if (styleName == "ukui-dark" || styleName == "ukui-black"){
            mSystemTheme = eTheme_Dark;// dark theme
        }
        syslog(LOG_DEBUG, "[%s] System theme changed, set theme: %s", __func__, mSystemTheme ? "Dark" : "Light");
        AppControlManager::getInstance()->controlApp(0, "android", 19, (int)mSystemTheme);
    }
    else if (key == "systemFont" || key == "systemFontSize") {// system font changed
        lgm lk(mWinMapMtx);
        for (auto window : mWindowMap) {
            if (window) {
                QFont font = window->font();
                for (auto widget : qApp->allWidgets()) {
                    widget->setFont(font);
                }
            }
        }
    }
}

void KmreWindowManager::handleNotification(const QString &appName, const QString &pkgName, const QString &text, 
                                        int winId, bool hide, bool call, const QString &title)
{
    if (KmreConfig::Preferences::getInstance()->notification) {
        lgm lk(mWinMapMtx);
        if (mWindowMap.contains(pkgName) && mWindowMap[pkgName]) {
            mWindowMap[pkgName]->handleNotification(text, hide, call, title);
        }
    }
}

void KmreWindowManager::focusWindowDisplay(int displayId)
{
    EventManager::getInstance()->setCurrentFocusedDisplay(displayId);
    AppControlManager::getInstance()->setFocusWindow(displayId);
}

void KmreWindowManager::onSwitchMultipler(const QString &pkgName, int displayId, bool enable)
{
    lgm lk(mWinMapMtx);
    for (auto &window : mWindowMap) {
        if (window) {
            syslog(LOG_INFO, "[%s] App '%s' switch multipler: [%d][%d]", 
                __func__, pkgName.toStdString().c_str(), displayId, enable);
            if (pkgName.startsWith(window->getPackageName())) {
                window->switchMultipler(pkgName, displayId, enable);
                return;
            }
        }
    }
}

void KmreWindowManager::initSystemSettings()
{
    // system language
    mSystemLanguage = eLauguage_CN;// chinese default
    QString locale = QLocale::system().name();
    if (locale == "en_US") {
        mSystemLanguage = eLauguage_EN;
    }
    // if the system language setting changed, the setting will be valid after system reboot or logout, 
    // so do not need to connect this setting signal.
    syslog(LOG_INFO, "[%s] System language: %s", __func__, mSystemLanguage ? "English" : "中文");
    mDbusClient->SetDefaultPropOfContainer(
        kmre::utils::getUserName(), getuid(), "sys.kmre.language", QString::number(mSystemLanguage));

    // system ukui style
    mSystemTheme = eTheme_Light;// light theme default
    if (QGSettings::isSchemaInstalled(ORG_UKUI_STYLE)) {
        mSystemUkuiStyleGSettings = std::make_unique<QGSettings>(ORG_UKUI_STYLE);
        if (mSystemUkuiStyleGSettings->keys().contains("styleName")) {
            QString styleName = mSystemUkuiStyleGSettings->get("style-name").toString();
            if(styleName == "ukui-dark" || styleName == "ukui-black"){
                mSystemTheme = eTheme_Dark;// dark theme
            }
        }
        connect(mSystemUkuiStyleGSettings.get(), &QGSettings::changed, this, &KmreWindowManager::onSystemUkuiStyleChanged);
    }
    syslog(LOG_INFO, "[%s] System theme: %s", __func__, mSystemTheme ? "Dark" : "Light");
    mDbusClient->SetDefaultPropOfContainer(
        kmre::utils::getUserName(), getuid(), "sys.kmre.theme", QString::number(mSystemTheme));

    mGnomeDbus = std::make_unique<QDBusInterface>("org.gnome.SessionManager", 
                                                "/org/gnome/SessionManager", 
                                                "org.gnome.SessionManager",
                                                QDBusConnection::sessionBus());
}

bool KmreWindowManager::inhibitSystemLockScreen(bool inhibit) 
{
    if (mGnomeDbus->isValid()) {
        if (inhibit) {
            QDBusMessage reply = mGnomeDbus->call(QDBus::Block, "Inhibit", "kmre", (quint32)0, "kmre media is playing", (quint32)8);
            mInhibitValue = reply.arguments().takeFirst().toUInt();
        }
        else {
            mGnomeDbus->call("Uninhibit", mInhibitValue);
        }
        return true;
    }
    return false;
}

bool KmreWindowManager::getSystemPanelHeight(int &height) 
{
    if (mSystemPanelGSettings) {
        if (mSystemPanelGSettings->keys().contains("panelsize")) {
            height = mSystemPanelGSettings->get("panelsize").toInt();
            return true;
        }
    }
    return false;
}

void KmreWindowManager::answerCall(bool accept)
{
    syslog(LOG_DEBUG, "[%s] accept = %d", __func__, accept);
    AppControlManager::getInstance()->answerCall(accept);
}

void KmreWindowManager::registerWindowDbusService()
{
    qRegisterMetaType<AndroidMeta>("AndroidMeta");
    qDBusRegisterMetaType<QList<QByteArray>>();
    qDBusRegisterMetaType<AndroidMeta>();
    qDBusRegisterMetaType<AndroidMetaList>();

    WindowAdaptor *adapter = new WindowAdaptor(this);
    Q_UNUSED(adapter)
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(SERVICE_INTERFACE) || !connection.registerObject(SERVICE_PATH, this)) {
        syslog(LOG_ERR, "[%s] Failed to register window dbus service! Err msg: '%s' ", 
            __func__, connection.lastError().message().toStdString().c_str());
    }
}

void KmreWindowManager::setSettingsForAndroid()
{
    AppSettings::getInstance().initAndroidDataPath();
}

void KmreWindowManager::quitSafely()
{   
    syslog(LOG_INFO, "[%s] Exit kmre window safely now...", __func__);

    inhibitSystemLockScreen(false);
    destroyAllAppWindows();
    
    EventManager::destroy();
    ScreenSharing::destroy();
    DisplayBackend::destroy();
    AppControlManager::destroy();

    exit(0);
}

bool KmreWindowManager::event(QEvent *e)
{
    QEvent::Type type = e->type();
    if (type == SignalEvent::eLeaveEvent) {
        SignalEvent *event = dynamic_cast<SignalEvent*>(e);
        if (event) {
            lgm lk(mWinMapMtx);
            for (const auto& win : mWindowMap) {
                if (win && (win->isOwner(event->getWinId()))) {
                    win->limitMouse();
                    break;
                }
            }
        }
        e->accept();
        return true;
    }

    return QObject::event(e);
}
