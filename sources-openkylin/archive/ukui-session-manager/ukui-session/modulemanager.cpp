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
#include "modulemanager.h"
#include "ukuimodule.h"
#include "idlewatcher.h"
#include "ukuisessiondebug.h"

#include <QCoreApplication>
#include "xdgautostart.h"
#include "xdgdesktopfile.h"
#include "xdgdirs.h"
#include <QFileInfo>
#include <QStringList>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>
#include <QGSettings/QGSettings>
// #include <QSoundEffect>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDir>
#include <QDBusReply>
/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#define SESSION_REQUIRED_COMPONENTS "org.ukui.session.required-components"
#define SESSION_REQUIRED_COMPONENTS_PATH "/org/ukui/desktop/session/required-components/"

#define SS_DBUS_SERVICE "org.ukui.ScreenSaver"
#define SS_DBUS_PATH "/"
#define SS_DBUS_INTERFACE "org.ukui.ScreenSaver"

#define STATE_DBUS_SERVICE "com.kylin.statusmanager.interface"
#define STATE_DBUS_PATH "/"
#define STATE_DBUS_INTERFACE "com.kylin.statusmanager.interface"

std::map<const QString, int> ModuleManager::m_startupMap = {};

void ModuleManager::statusChanged(bool enabled)
{
    if (enabled) {
        qDebug() << "change to table mode ,start tablet desktop";
        startProcess(mTableDesk, false);
    }
}

bool ModuleManager::checkIsTabletmode()
{
    bool isTablemode = false;
    QDBusInterface *dbusService = new QDBusInterface(
        STATE_DBUS_SERVICE, STATE_DBUS_PATH, STATE_DBUS_INTERFACE, QDBusConnection::sessionBus(), this);
    if (dbusService && dbusService->isValid()) {
        QDBusReply<bool> reply = dbusService->call("isTabletModeEnabled");
        if (reply.isValid()) {
            qDebug() << "getCurrentTabletMode:" << reply.value();
            isTablemode = reply.value();
        }
        // 监听切换成平板桌面信号
        connect(dbusService, SIGNAL(isTabletModeEnabledChanged(bool)), this, SLOT(statusChanged(bool)));
    } else {
        qDebug() << "com.kylin.statusmanager.interface not valid";
    }
    return isTablemode;
}

void ModuleManager::playBootMusic(bool arg)
{
    if (arg)
        return;
    // set default value of whether boot-music is opened
    bool play_music = true;
    if (QGSettings::isSchemaInstalled("org.ukui.session")) {
        QGSettings *gset = new QGSettings("org.ukui.session", "/org/ukui/desktop/session/", this);
        if (gset == NULL) {
            qDebug() << "QGSettings init error";
            free(gset);
            return;
        }
        QString xdg_session_type = qgetenv("XDG_SESSION_TYPE");
        if (arg) {
            play_music = gset->get("startup-music").toBool();
            if (play_music) {
                if (xdg_session_type == "wayland") {
                    QProcess::startDetached("paplay --volume=23456 /usr/share/ukui/ukui-session-manager/startup.wav");
                } else {
                    QProcess::startDetached("aplay  /usr/share/ukui/ukui-session-manager/startup.wav");
                }
            }
        } else {
            play_music = gset->get("weakup-music").toBool();
            if (play_music) {
                if (xdg_session_type == "wayland") {
                    QProcess::startDetached("paplay /usr/share/ukui/ukui-session-manager/weakup.wav");
                } else {
                    QProcess::startDetached("aplay /usr/share/ukui/ukui-session-manager/weakup.wav");
                }
            }
        }
    }
}

// GCOV_EXCL_START: The single test process cannot reach simultaneously.
// 睡眠转休眠判断
bool isSuspendToHibernate()
{
    QString path = "/sys/power/wakeup_reason";
    if (!QFile::exists(path)) {
        return false;
    }
    QFile wakeup_reason_file(path);
    wakeup_reason_file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString wakeupReason = wakeup_reason_file.readAll().simplified();
    wakeup_reason_file.close();
    if (QString::compare(wakeupReason, "0xaa") == 0) {
        qDebug() << "Suspend to Hibernate, wakeup_reason is " << wakeupReason;
        return true;
    }
    return false;
}
// GCOV_EXCL_STOP
ModuleManager *ModuleManager::m_singleInstance = nullptr;
ModuleManager::ModuleManager(QObject *parent) : QObject(parent) {}

ModuleManager *ModuleManager::instance(QObject *parent)
{
    if (m_singleInstance == nullptr)
        m_singleInstance = new ModuleManager(parent);
    return m_singleInstance;
}

void ModuleManager::init(const QString &arg)
{
    QDBusConnection::systemBus().connect(
        QString("org.freedesktop.login1"),
        QString("/org/freedesktop/login1"),
        QString("org.freedesktop.login1.Manager"),
        QString("PrepareForSleep"),
        this,
        SLOT(weakup(bool)));

    const QByteArray gsettingsId("org.ukui.session.required-components");
    if (QGSettings::isSchemaInstalled(gsettingsId)) {
        rcGs = new QGSettings("org.ukui.session.required-components");
        isSpecialMode = rcGs->get("isSpecialMode").toBool();
    } else {
        qWarning() << "org.ukui.session.required-components can not be found";
    }

    QFileInfo iniFile("/etc/ukui/ukui-session/smconfig.ini");
    // GCOV_EXCL_START:belongs to the program protection logic, when the system environment problems will be executed.
    if (iniFile.isFile()) {
        QSettings settings("/etc/ukui/ukui-session/smconfig.ini", QSettings::IniFormat);
        settings.setIniCodec("utf-8");

        QString isSMVer = QString("Config/IsSM");
        bool isSMVar = settings.value(isSMVer).toBool();
        QString appNumStr = QString("Config/AppNum");
        int appNum = settings.value(appNumStr).toInt();

        QStringList appList;
        if (appNum >= 1) {
            for (int i = 1; i <= appNum; i++) {
                QString smAppVer = "Config/SMApp" + QString::number(i);
                qDebug() << "smAppVer=" << smAppVer;
                QString smAppVar = settings.value(smAppVer).toString();
                appList << smAppVar;
            }
        }
        qDebug() << "isSMVar=" << isSMVar << ",applist=" << appList;

        if (isSMVar) {
            isSpecialMode = true;
            rcGs->set("isSpecialMode", "true");
            rcGs->set("special-mode-apps", appList);
        } else {
            isSpecialMode = false;
            rcGs->set("isSpecialMode", "false");
        }
    }
    // GCOV_EXCL_STOP
    if (isSpecialMode) {
        constructSpecialList();
    } else {
        constructStartupList();
    }

    ChkScreenLockStartup();
    setUKUILIte(arg);
    initConn();
}

void ModuleManager::initConn()
{
    connect(WindowManager::self(), &WindowManager::windowAdded, this, &ModuleManager::windowAdded);
    connect(WindowManager::self(), &WindowManager::windowRemoved, this, &ModuleManager::windowRemoved);
}

bool ModuleManager::quickStart()
{
    mLockScreen.load("/etc/xdg/autostart/ukui-screensaver.desktop");
    QStringList extArgs;
    extArgs << "--lock-startup";
    startProcess(mLockScreen, true, extArgs);
    isScreenLockStartup = true;
    return true;
}

void ModuleManager::monitorQuickStart()
{
    if (isScreenLockStartup) {
        startModuleTimer(tLockScreen, 3);
        connect(this, &ModuleManager::screensaverfinished, [=]() { startup(); });
    }
}

void ModuleManager::constructSpecialList()
{
    if (isSpecialMode) {
        QStringList appList = rcGs->get("special-mode-apps").toStringList();
        if (appList.isEmpty()) {
            qDebug() << "gsettings key : special-mode-apps is empty";
            return;
        }
        XdgDesktopFile file;
        for (int i = 0; i < appList.size(); i++) {
            file.load(appList.at(i));
            mspecialModeApp << file;
        }
    }
}

QSet<QString> ModuleManager::getUKUILiteApps()
{
    QSet<QString> ret;
    QFileInfo configFile(UKUI_LITE_CONFIG);
    if (configFile.isFile()) {
        QSettings settings(UKUI_LITE_CONFIG, QSettings::IniFormat);
        int appNum = settings.beginReadArray("UKUI-Lite-Apps");

        for (int i = 0; i < appNum; ++i) {
            settings.setArrayIndex(i);
            ret << settings.value("AppName").toString();
        }
    }
    return ret;
}

void ModuleManager::weakup(bool arg)
{
    if (arg) {
        qDebug() << "准备执行睡眠休眠";
    } else {
        qDebug() << "从睡眠休眠中唤醒";
        if (!isSuspendToHibernate()) {
            qDebug() << "播放唤醒音乐";
            playBootMusic(false);
        }
    }
}

ModuleManager::~ModuleManager()
{
    ModulesMapIterator i(mNameMap);
    while (i.hasNext()) {
        i.next();

        auto p = i.value();
        disconnect(p, SIGNAL(finished(int, QProcess::ExitStatus)), nullptr, nullptr);

        delete p;
        mNameMap[i.key()] = nullptr;
    }
}

void ModuleManager::constructStartupList()
{
    const QByteArray id(SESSION_REQUIRED_COMPONENTS);
    QString window_manager;
    QString panel;
    QString file_manager;
    QString wm_notfound;
    if (QGSettings::isSchemaInstalled(id)) {
        const QGSettings *gs = new QGSettings(SESSION_REQUIRED_COMPONENTS, SESSION_REQUIRED_COMPONENTS_PATH, this);
        if (gs == NULL) {
            qDebug() << "QGSettings init error";
            return;
        }
        window_manager = gs->get("windowmanager").toString() + ".desktop";
        panel = gs->get("panel").toString() + ".desktop";
        file_manager = gs->get("filemanager").toString() + ".desktop";
        wm_notfound = gs->get("windowmanager").toString();
    } else {
        // gsetting安装失败，或无法获取，设置默认值
        qDebug() << "从gsettings 中或取值失败，设置默认值";
        window_manager = "ukwm.desktop";
        panel = "ukui-panel.desktop";
        file_manager = "peony-qt-desktop.desktop";
    }

    QString xdg_session_type = qgetenv("XDG_SESSION_TYPE");
    if (xdg_session_type == "wayland") {
        isWayland = true;
    }

    QStringList desktop_paths;
    desktop_paths << "/usr/share/applications";
    desktop_paths << "/etc/xdg/autostart";
    bool panel_found = false;
    bool fm_found = false;
    bool wm_found = false;
    bool table_found = false;

    // const auto files = XdgAutoStart::desktopFileList(desktop_paths, false);
    for (const QString &dirName : const_cast<const QStringList &>(desktop_paths)) {
        QDir dir(dirName);
        if (!dir.exists())
            continue;
        const QFileInfoList files
            = dir.entryInfoList(QStringList(QLatin1String("*.desktop")), QDir::Files | QDir::Readable);
        for (const QFileInfo &fi : files) {
            if (fi.fileName() == panel) {
                mPanel.load(fi.absoluteFilePath());
                panel_found = true;
                qDebug() << "panel has been found";
            } else if (fi.fileName() == file_manager) {
                mFileManager.load(fi.absoluteFilePath());
                fm_found = true;
                qDebug() << "filemanager has been found";
            } else if (fi.fileName() == window_manager) {
                mWindowManager.load(fi.absoluteFilePath());
                wm_found = true;
                qDebug() << "windowmanager has been found";
            } else if (fi.fileName() == "ukui-tablet-desktop.desktop") {
                mTableDesk.load(fi.absoluteFilePath());
                table_found = true;
                qDebug() << "tableDesk has been found";
            }

            if (fm_found && panel_found && wm_found && table_found)
                break;
        }
    }

    if (!panel_found || !fm_found || !wm_found)
        isDirectInstall = true;
    if (isDirectInstall) {
        wmFound = wm_found;
    }

    if (wm_found == false) {
        QFileInfo check_ukwm("/usr/share/applications/ukwm.desktop");
        QFileInfo check_ukuikwin("/usr/share/applications/ukui-kwin.desktop");
        if (check_ukwm.exists()) {
            window_manager = "ukwm.desktop";
            mWindowManager.load("/usr/share/applications/ukwm.desktop");
        } else if (check_ukuikwin.exists()) {
            window_manager = "ukui-kwin.desktop";
            mWindowManager.load("/usr/share/applications/ukui-kwin.desktop");
        }
        wm_found = true;
    }

    // 配置文件所给的窗口管理器找不到.desktop文件时，将所给QString设为可执行命令，创建一个desktop文件赋给mWindowManager
    //    if (wm_found == false) {
    //        mWindowManager = XdgDesktopFile(XdgDesktopFile::ApplicationType,"window-manager", wm_notfound);
    //        qDebug() << "windowmanager has been created";
    //    }

    QString desktop_phase = "X-UKUI-Autostart-Phase";
    QString desktop_type = "Type";
    // 设置excludeHidden为true，判断所有desktop文件的Hidden值，若为true，则将其从自启列表中去掉
    const XdgDesktopFileList all_file_list = XdgAutoStart::desktopFileList(true);
    for (XdgDesktopFileList::const_iterator i = all_file_list.constBegin(); i != all_file_list.constEnd(); ++i) {
        QString filePath = i->fileName();
        QString filename = QFileInfo(i->fileName()).fileName();
        if (filename == panel || filename == file_manager || filename == window_manager
            || filename == "ukui-tablet-desktop.desktop"
            || (filename == "ukui-screensaver.desktop" && mLockScreen.isValid())) {
            continue;
        }
        const XdgDesktopFile file = *i;
        if (i->contains(desktop_phase)) {
            QStringList s1 = file.value(desktop_phase).toString().split(QLatin1Char(';'));
            if (s1.contains("Initialization")) {
                mInitialization << file;
            } else if (s1.contains("Desktop")) {
                mDesktop << file;
            } else if (s1.contains("Application")) {
                // 用户设置的自启应用要单独判断，有可能会存在问题，但暂时想不到更好的判断方法
                if (filePath.contains(".config/autostart")) {
                    m_userAutoStartApp << file;
                } else {
                    mApplication << file;
                }
            } else if (s1.contains("Notification")) {
                mNotification << file;
            }
        } else if (i->contains(desktop_type)) {
            QStringList s2 = file.value(desktop_type).toString().split(QLatin1Char(';'));
            if (s2.contains("Application")) {
                if (filePath.contains(".config/autostart")) {
                    m_userAutoStartApp << file;
                } else {
                    mApplication << file;
                }
            }
        }
    }

    QStringList force_app_paths;
    force_app_paths << "/usr/share/ukui/applications";
    const XdgDesktopFileList force_file_list = XdgAutoStart::desktopFileList(force_app_paths, true);
    for (XdgDesktopFileList::const_iterator i = force_file_list.constBegin(); i != force_file_list.constEnd(); ++i) {
        qDebug() << (*i).fileName();
        mForceApplication << *i;
    }
}

/* Startup Phare:
 *  Initialization
 *  WindowManager
 *  Panel
 *  FileManager
 *  Desktop
 *  Application
 *
 */

bool ModuleManager::startModuleTimer(QTimer *timer, int i)
{
    if (!timer || i < 0) {
        qWarning() << "timer is invalid!!";
        return false;
    }
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeup()));
    timer->start(i * 1000);
    return true;
}

void ModuleManager::startupfinished(const QString &appName, const QString &string)
{
    qDebug() << "moudle :" + appName + " startup finished, and it want to say " + string;
    if (appName == "ukui-settings-daemon") {
        tusd->stop();
        emit usdfinished();
        return;
    }
    if (appName == "ukui-kwin") {
        twm->stop();
        isWMStarted = true;
        emit wmfinished();
        return;
    }
    if (appName == "ukui-panel") {
        tpanel->stop();
        isPanelStarted = true;
        emit panelfinished();
        return;
    }
    if (appName == "peony-qt-desktop") {
        tdesktop->stop();
        isDesktopStarted = true;
        emit desktopfinished();
        return;
    }
    if (appName == "ukui-screensaver-dialog") {
        tLockScreen->stop();
        isLockScreenStarted = true;
        emit screensaverfinished();
        return;
    }
}

void ModuleManager::timeup()
{
    QTimer *time_out = qobject_cast<QTimer *>(sender());
    if (time_out == tusd) {
        qDebug() << "usd超时";
        emit usdfinished();
        return;
    }
    if (time_out == twm) {
        qDebug() << "wm超时";
        isWMStarted = true;
        emit wmfinished();
        return;
    }
    if (time_out == tpanel) {
        qDebug() << "panel超时";
        isPanelStarted = true;
        emit panelfinished();
        return;
    }
    if (time_out == tdesktop) {
        qDebug() << "peony-qt-desktop超时";
        isDesktopStarted = true;
        emit desktopfinished();
        return;
    }
    if (time_out == tLockScreen) {
        qDebug() << "ukui-screensaver超时";
        isLockScreenStarted = true;
        emit screensaverfinished();
        return;
    }
}

void ModuleManager::startNotifPhase()
{
    qDebug() << "是否为秒级登陆 isScreenLockStartup = " << isScreenLockStartup;
    for (XdgDesktopFileList::const_iterator i = mNotification.constBegin(); i != mNotification.constEnd(); ++i) {
        startProcess(*i, false);
    }
}

void ModuleManager::startCompsite()
{
    qDebug() << "Enter:: startCompsite";
    if (!isPanelStarted || !isDesktopStarted || !isWMStarted || (mLockScreen.isValid() && !isLockScreenStarted))
        return;
    if (isCompsiteStarted)
        return;
    isCompsiteStarted = true;
    if (isWayland) {
        if (isScreenLockStartup) {
            // 规避登录界面输入框卡顿问题
            QTimer::singleShot(2500, this, SLOT(timerUpdate()));
        } else {
            QTimer::singleShot(500, this, SLOT(timerUpdate()));
        }
    } else {
        // 非秒级登录，延时不用加
        if (isScreenLockStartup) {
            // 规避登录界面输入框卡顿问题
            QTimer::singleShot(2000, this, SLOT(timerUpdate()));
            // timerUpdate();
        } else {
            timerUpdate();
        }
    }

    /*    //按kwin要求，kwin现在启动后自动开混成
        // start composite
        QDBusInterface dbus("org.ukui.KWin", "/Compositor", "org.ukui.kwin.Compositing", QDBusConnection::sessionBus());

        if (!dbus.isValid()) {
            qWarning() << "dbusCall: QDBusInterface is invalid ; kwin do not exit!";
            timerUpdate();
        } else {
            qDebug() << "Start composite";
            dbus.call("resume");

            timerUpdate();
        }*/
}

void ModuleManager::startup()
{
    if ((mLockScreen.isValid() && !isLockScreenStarted) || isStartuped) {
        qInfo() << "Wait for lock screen quickstart";
        return;
    }
    isStartuped = true;
    // 特殊模式启动
    if (isSpecialMode) {
        for (XdgDesktopFileList::const_iterator i = mspecialModeApp.constBegin(); i != mspecialModeApp.constEnd();
             ++i) {
            startProcess(*i, false);
        }
        return;
    }

    // 直接安装进入的流程
    const QFile file_installer("/etc/xdg/autostart/kylin-os-installer.desktop");
    if (file_installer.exists() && isDirectInstall) {
        if (wmFound) {
            isPanelStarted = true;
            isDesktopStarted = true;
            connect(this, &ModuleManager::wmfinished, [=]() { startCompsite(); });

            startProcess(mWindowManager, true);
            startModuleTimer(twm, 18);
        } else {
            timerUpdate();
        }

        return;
    }

    if (isWayland) {
        startProcess("hwaudioservice", true);
    }

    if (!isWayland) {
        connect(this, &ModuleManager::panelfinished, [=]() { startCompsite(); });
        connect(this, &ModuleManager::wmfinished, [=]() { startCompsite(); });
        connect(this, &ModuleManager::desktopfinished, [=]() { startCompsite(); });
        //        dostartwm();
    } else {
        isWMStarted = true;
        connect(this, &ModuleManager::panelfinished, [=]() { startCompsite(); });
        connect(this, &ModuleManager::desktopfinished, [=]() { startCompsite(); });
        // // 非秒级登录，延时不用加
        // if (isScreenLockStartup) {
        //     // 规避登录界面输入框卡顿问题
        //     QTimer::singleShot(2500, this, SLOT(timerUpdate()));
        // } else {
        //     QTimer::singleShot(500, this, SLOT(timerUpdate()));
        // }
    }

    qDebug() << "Start Initialization app: ";
    for (XdgDesktopFileList::const_iterator i = mInitialization.constBegin(); i != mInitialization.constEnd(); ++i) {
        startProcess(*i, true);
    }
    startModuleTimer(twm, 3);

    startProcess(mPanel, true);
    startModuleTimer(tpanel, 5);

    startProcess(mFileManager, true);
    startModuleTimer(tdesktop, 5);

    if (checkIsTabletmode()) {
        startProcess(mTableDesk, false);
    }

    if (!isWayland) {
        qDebug() << "Start desktop: ";
        for (XdgDesktopFileList::const_iterator i = mDesktop.constBegin(); i != mDesktop.constEnd(); ++i) {
            startProcess(*i, true);
        }
    }
}

void ModuleManager::dostartwm()
{
    qDebug() << "Start window manager: " << mWindowManager.name();
    if (mWindowManager.name() == "UKUI-KWin") {
        startProcess(mWindowManager, true);
        startModuleTimer(twm, 18);
    } else {
        startProcess(mWindowManager, true);
        isWMStarted = true;
    }
}

void ModuleManager::timerUpdate()
{
    QTimer::singleShot(500, this, [&]() { emit finished(); });

    if (isWayland) {
        qDebug() << "Start desktop: ";
        for (XdgDesktopFileList::const_iterator i = mDesktop.constBegin(); i != mDesktop.constEnd(); ++i) {
            startProcess(*i, true);
        }
    }

    qDebug() << "Start application: ";
    QFile file_nm("/etc/xdg/autostart/kylin-nm.desktop");
    QFile file_sogou("/usr/bin/sogouImeService");
    int indexKylinnm = -1;
    int indexSogou = -1;
    for (XdgDesktopFileList::const_iterator i = mApplication.constBegin(); i != mApplication.constEnd(); ++i) {
        qDebug() << i->fileName();
        if (i->fileName() == "/etc/xdg/autostart/nm-applet.desktop" && file_nm.exists()) {
            qDebug() << "the kylin-nm exist so the nm-applet will not start";
            indexKylinnm = i - mApplication.constBegin();
            continue;
        }
        if (i->fileName() == "/etc/xdg/autostart/fcitx-qimpanel-autostart.desktop" && file_sogou.exists()) {
            qDebug() << "the sogouImeService exist so the fcitx-ui-qimpanel will not start";
            indexSogou = i - mApplication.constBegin();
            continue;
        }
    }
    mApplication.removeAt(indexKylinnm);
    mApplication.removeAt(indexSogou);
    sequenceStart(mApplication);

    qDebug() << "Start force application: ";
    for (XdgDesktopFileList::const_iterator i = mForceApplication.constBegin(); i != mForceApplication.constEnd();
         ++i) {
        startProcess(*i, true);
    }

    if (!isScreenLockStartup) {
        startNotifPhase();
    }

    // 启动用户自定义的开机自启应用
    for (XdgDesktopFileList::const_iterator i = m_userAutoStartApp.constBegin(); i != m_userAutoStartApp.constEnd();
         ++i) {
        qDebug() << "start user's autostart application " << i->fileName();
        openAppWithAppManager(*i);
    }

    if (QGSettings::isSchemaInstalled("org.ukui.session")) {
        QGSettings *gset = new QGSettings("org.ukui.session", "/org/ukui/desktop/session/", this);
        bool restoreSession = gset->get("restore-session").toBool();
        if (restoreSession) {
            // 此处设置800ms延迟的原因是，当程序运行到这里时，已经启动了很多应用，根据测试，所有这些应用启动完并且完成与xmspserver的通信一般就需要800ms，如果这里不等待800ms，
            // 然后直接调用xsmpserver的restoreSession接口，则xsmpserver的主循环会被用来启动那些待恢复的应用,从而无暇顾及其他应用的信号请求，造成阻塞，影响整个启动流程
            // 之前觉得把xsmpserver分离出去可以解决这个问题，现在看来还是不行，因为ukuismserver还是要跟ukui-session交互并且判断是否该应用是否已经已经启动过，真正要解决这个
            // 问题，还是要把启动阶段划分好，在restoreSession阶段保证就不会有别的应用干扰。
            QTimer::singleShot(800, []() {
                // 调用ukuismserver恢复会话的接口
                qDebug(UKUI_SESSION) << "began restore session";
                QDBusInterface ukuismserverFace(
                    "org.ukui.ukuismserver", "/UKUISMServer", "org.ukui.ukuismserver", QDBusConnection::sessionBus());

                if (!ukuismserverFace.isValid()) {
                    qDebug() << "ukuisminterface not valid";
                    return;
                }

                ukuismserverFace.call(QDBus::CallMode::NoBlock, "restoreSession");
            });
        }
    }

    // 关闭ksplash
    endUkuiSplash();
}

void ModuleManager::startProcess(
    const XdgDesktopFile &file,
    bool required,
    const QStringList &arguments /* = QStringList()*/,
    bool keepArgs /* = false*/)
{
    QStringList args = file.expandExecString(arguments);
    if (args.isEmpty()) {
        qWarning() << "Wrong desktop file: " << file.fileName();
        return;
    }

    QString name = QFileInfo(file.fileName()).fileName();
    if (!mNameMap.contains(name)) {
        if (mUKUILiteApps.count() > 0) {
            if (!mUKUILiteApps.contains(name)) {
                return;
            } else {
                qDebug() << "ready start UKUILite apps: " << name;
            }
        }

        UkuiModule *proc = new UkuiModule(file, this);
        connect(proc, &UkuiModule::moduleStateChanged, this, &ModuleManager::moduleStateChanged);
        proc->startUKUIModule(arguments, keepArgs);

        mNameMap[name] = proc;

        if (required || autoRestart(file)) {
            connect(
                proc,
                SIGNAL(finished(int, QProcess::ExitStatus)),
                this,
                SLOT(restartModules(int, QProcess::ExitStatus)));
        }
    }
}

void ModuleManager::startProcess(const QString &name, bool required)
{
    QString desktop_name = name + ".desktop";
    QStringList desktop_paths;
    desktop_paths << "/usr/share/applications";

    const auto files = XdgAutoStart::desktopFileList(desktop_paths, false);
    for (const XdgDesktopFile &file : files) {
        if (QFileInfo(file.fileName()).fileName() == desktop_name) {
            startProcess(file, required);
            return;
        }
    }
}

void ModuleManager::stopProcess(const QString &name)
{
    if (mNameMap.contains(name)) {
        mNameMap[name]->terminate();
    }
}

bool ModuleManager::startProcess(const QString &name, const QStringList &args)
{
    if (isProgramStarted(std::move(name))) {
        return false;
    }

    // 这个接口供外部应用通过session打开应用使用，需要使用那个进程非正常死亡再拉起机制吗？
    QProcess *proc = new QProcess(this);
    proc->start(name, args);
    proc->closeReadChannel(QProcess::StandardOutput);
    proc->closeReadChannel(QProcess::StandardError);
    // m_startupMap[std::forward<const QString>(name)] = 1;
    m_startupMap[name] = 1;
    return true;
}

bool ModuleManager::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    //    if (eventType != "xcb_generic_event_t") // We only want to handle XCB events
    //        return false;

    return false;
}

void ModuleManager::insertStartupList(QString &&str)
{
    if (!str.isEmpty()) {
        m_startupMap[std::move(str)] = 1;
    }
}

bool ModuleManager::isProgramStarted(const QString &&str)
{
    // 需要对str做处理，取出可执行程序名，str一般是一个完整的二进制路径，有时候又只有一个可执行程序名，所以既需要判断二进制路径也要判断二进制名
    QString appName = str.mid(str.lastIndexOf(QDir::separator()) + 1);
    auto itFullPath = m_startupMap.find(str);
    auto itName = m_startupMap.find(appName);
    qDebug() << "judge if start before " << appName << " " << str;

    if (itFullPath != m_startupMap.end() || itName != m_startupMap.end()) {
        qDebug() << str << "has start up before";
        return true;
    }

    return false;
}

bool ModuleManager::autoRestart(const XdgDesktopFile &file)
{
    QString auto_restart = "X-UKUI-AutoRestart";
    return file.value(auto_restart).toBool();
}

void ModuleManager::sequenceStart(XdgDesktopFileList &fileList)
{
    // 如果没有要启动的进程，直接返回
    if (fileList.isEmpty()) {
        return;
    }

    if (isScreenLockStartup) {
        // 快速启动时，间隔100ms拉起
        if (!sequenceTimer) {
            sequenceTimer = new QTimer(this);
            sequenceTimer->setInterval(100);
            connect(sequenceTimer, &QTimer::timeout, this, [this]() {
                if (sequenceFileList && !sequenceFileList->isEmpty()) {
                    XdgDesktopFile file = selectOneAppToStart(*sequenceFileList);
                    startProcess(file, false);
                }
                if (!sequenceFileList || sequenceFileList->isEmpty()) {
                    sequenceTimer->stop();
                }
            });
        }
        sequenceFileList = &fileList;
        // 先启动一个
        XdgDesktopFile file = selectOneAppToStart(*sequenceFileList);
        startProcess(file, false);
        // 启动定时器，后续每100ms启动一个
        sequenceTimer->start();
    } else {
        // 普通情况，一次性全部拉起
        while (!fileList.isEmpty()) {
            XdgDesktopFile file = selectOneAppToStart(fileList);
            startProcess(file, false);
        }
    }
}

XdgDesktopFile ModuleManager::selectOneAppToStart(XdgDesktopFileList &fileList)
{
    // 第一次进入循环，m_appStarted为空，所以不会执行
    // 在启动了第一个应用后，m_appStarted有值，开始执行下面的逻辑
    while (!m_appStarted.isEmpty()) {
        // 用于判断是否存在依赖启动app的标志位
        bool hasAppStartAfter = false;
        XdgDesktopFile fileStarted = m_appStarted.first();
        QString fileStartedName = QFileInfo(fileStarted.fileName()).fileName();
        // 找出启动列表中statrtAfter为已启动app的desktopfile
        for (int i = 0; i < fileList.size(); ++i) {
            XdgDesktopFile file = fileList.at(i);
            QString startAfter = file.value("X-UKUI-autostart-after").toString();
            startAfter = startAfter + ".desktop";
            if (startAfter == fileStartedName) {
                qCDebug(UKUI_SESSION()) << file.name() << " start after " << fileStartedName;
                // 这里启动的app加入到已启动app列表中，因为我们还需要搜寻依赖此app启动的app
                // 将其从原始启动列表中删除，并且设置标志位为true
                // break掉，否则，for循环会出问题
                m_appStarted.prepend(file);
                fileList.removeAt(i);
                hasAppStartAfter = true;
                return file;
            }
        }
        // 如果经历上面的搜索过程，标志位仍为false，说明并没有app依赖该app的启动，所以将其移除
        if (!hasAppStartAfter) {
            m_appStarted.removeFirst();
        }
    }

    // 找出一个不依赖其他app的app,将其启动，并加入到已启动列表中
    for (int i = 0; i < fileList.size(); ++i) {
        XdgDesktopFile file = fileList.at(i);
        if (file.value("X-UKUI-autostart-after").toString().isEmpty()) {
            m_appStarted.prepend(file);
            fileList.removeAt(i);
            return file;
        }
    }

    // 不判断任何条件启动一个应用，这段处理是为了防止找不到应用依赖的应用而导致的死循环
#if 0
    for (int i = 0; i < fileList.size(); ++i) {
        XdgDesktopFile file = fileList.at(i);
        m_appStarted.prepend(file);
        fileList.removeAt(i);
        return file;
    }
#endif

    if (!fileList.isEmpty()) {
        XdgDesktopFile file = fileList.takeFirst();
        m_appStarted.prepend(file);
        return file;
    }

    qCDebug(UKUI_SESSION()) << "retun empty xdg file";
    return XdgDesktopFile();
}

void ModuleManager::startUkuiSplash()
{
    QProcess *splash = new QProcess(this);
    qCDebug(UKUI_SESSION()) << "start ukui-ksplash";
    splash->start("ukui-ksplash");
}

void ModuleManager::endUkuiSplash()
{
    QProcess *process = new QProcess(this);
    QString command = "pgrep ukui-ksplash";
    process->start(command);
    process->waitForFinished();
    if (process->exitCode() != 0) {
        qCDebug(UKUI_SESSION()) << "ukui-ksplash not running";
        return;
    }

    qCDebug(UKUI_SESSION()) << "enter close UkuiSplash";
    QDBusInterface face("org.ukui.KSplash", "/KSplash", "org.ukui.KSplash", QDBusConnection::systemBus());

    if (!face.isValid()) {
        qCDebug(UKUI_SESSION()) << "splash interface not valid";
        return;
    }

    QString state = "ksplashFinished";
    QDBusMessage result = face.call("setStage", QVariant(state));
    if (!result.errorMessage().isEmpty()) {
        qCDebug(UKUI_SESSION()) << "call splash interface return error " << result.errorMessage();
        return;
    }

    qCDebug(UKUI_SESSION()) << "finish close UkuiKsplash";
}

bool ModuleManager::openAppWithAppManager(const XdgDesktopFile &file)
{
    QDBusInterface face(
        "com.kylin.ProcessManager",
        "/com/kylin/ProcessManager/AppLauncher",
        "com.kylin.ProcessManager.AppLauncher",
        QDBusConnection::sessionBus());
    if (!face.isValid()) {
        qCDebug(UKUI_SESSION()) << "AppManager interface not valid";
        return false;
    }

    QString name = file.fileName();
    const QVariant desktopFilePath(name);
    QDBusReply<bool> reply = face.call("LaunchApp", desktopFilePath);
    if (reply.isValid()) {
        return reply.value();
    }

    return true;
}

void ModuleManager::restartModules(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
    if (isShutingDown) {
        qDebug() << "Logout phase, don't Restart";
        return;
    }

    UkuiModule *proc = qobject_cast<UkuiModule *>(sender());
    if (nullptr == proc) {
        qWarning() << "Got an invalid (null) module to restart, Ignoring it";
        return;
    }

    // GCOV_EXCL_START:belongs to the program protection logic, when the system environment problems will be executed.
    // 需要做出修改
    if (proc->restartNum > 10) {
        mNameMap.remove(proc->fileName);
        disconnect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), nullptr, nullptr);
        if (proc->fileName == "ukui-screensaver.desktop") {
            qDebug() << "ukui-screensaver-backend times out,start ukui-screensaver-abnormal.sh";
            //            startProcess("/usr/bin/ukui-screensaver-abnormal.sh", QStringList() << "");
            Q_EMIT saverBackendAbnormaled();
        }
        proc->deleteLater();
        return;
    }

    if (!proc->isTerminating()) {
        // 根据退出码来判断程序是否属于异常退出。
        QString procName = proc->getProcName();
        if (proc->exitCode() == 0) {
            qDebug() << "Process" << procName << "(" << proc << ") exited correctly. "
                     << "With the exitcode = " << proc->exitCode() << ",exitStatus = " << exitStatus;
        } else {
            qDebug() << "Process" << procName << "(" << proc << ") has to be restarted. "
                     << "With the exitcode = " << proc->exitCode() << ",exitStatus = " << exitStatus;
            proc->startUKUIModule();
            proc->restartNum++;
            return;
        }
    }
    mNameMap.remove(proc->fileName);
    proc->deleteLater();
    // GCOV_EXCL_STOP
}

bool ModuleManager::getIsShutingDown() const
{
    return isShutingDown;
}

void ModuleManager::setUKUILIte(QString UkuiLite)
{
    QString mode = "normal";
    do {
        QFile liteConfig(UKUI_LITE_CONFIG);
        if (!QGSettings::isSchemaInstalled(GLOBAL_MANAGER_SCHEMAS)) {
            qWarning() << "can;t find ukui-lite schema: " << GLOBAL_MANAGER_SCHEMAS;
            break;
        }

        QGSettings settings(GLOBAL_MANAGER_SCHEMAS);

        if (!(settings.keys().contains(LITE_FUNCITON_KEY, Qt::CaseInsensitive)
              && settings.keys().contains(LITE_ANIMATION_KEY, Qt::CaseInsensitive))) {
            qWarning() << "can;t find ukui-lite key: ukui-lite";
            break;
        }

        if (!UkuiLite.compare(UKUI_LITE, Qt::CaseInsensitive) && liteConfig.exists()) {
            qWarning() << "set ukui-lite";
            mode = "lite";
            mUKUILiteApps = getUKUILiteApps();
        }

        settings.set(LITE_FUNCITON_KEY, mode);
        settings.set(LITE_ANIMATION_KEY, mode);

    } while (0);
}

void ModuleManager::setIsShutingDown(bool value)
{
    isShutingDown = value;
}

// GCOV_EXCL_START:belongs to the program protection logic, when the system environment problems will be executed.
void ModuleManager::logout(bool doExit)
{
    // /org/freedesktop/login1/session/self 和 /org/freedesktop/login1/session/auto
    // 有什么区别
    if (m_strSessionPath.isEmpty()) {
        QDBusInterface dbus(
            "org.freedesktop.login1",
            "/org/freedesktop/login1",
            "org.freedesktop.login1.Manager",
            QDBusConnection::systemBus());
        if (!dbus.isValid()) {
            qWarning() << "dbusCall: QDBusInterface is invalid org.freedesktop.login1";
            return;
        }

        QDBusReply<QDBusObjectPath> reply = dbus.call("GetSession", "auto");

        if (!reply.isValid()) {
            qDebug() << "GetSession error:" << reply.error().name() << reply.error().message();
            return;
        }

        m_strSessionPath = reply.value().path();
    }

    QDBusInterface face(
        "org.freedesktop.login1", m_strSessionPath, "org.freedesktop.login1.Session", QDBusConnection::systemBus());
    if (!face.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid org.freedesktop.login1";
        return;
    }

    if (doExit) {
        face.call("Terminate");
        exit(0);
    }
}
// GCOV_EXCL_STOP

void ModuleManager::ChkScreenLockStartup()
{
    // 监听锁屏解锁和服务状态变化
    if (isScreenLockStartup) {
        // 如果服务已经存在，初始化接口
        if (FreedesktopHelper::getInstance()->NameHasOwner(SS_DBUS_SERVICE)) {
            Q_EMIT screensaverServiceStatusChanged(isScreenLockStartup);
        }

        connect(
            FreedesktopHelper::getInstance(),
            &FreedesktopHelper::serviceStatusChanged,
            this,
            &ModuleManager::onSessionDbusActiveChanged);
    }
}

void ModuleManager::onLockScrenUnlock()
{
    qDebug() << "lockscreen unlocked!";
    if (isScreenLockStartup && mNotificationFlag) {
        // mNotificationFlag作为标志，确保该阶段只启动一次
        mNotificationFlag = false;
        startNotifPhase();
        // 检查是否有未启动的进程，全部一次性拉起
        while (!mApplication.isEmpty()) {
            XdgDesktopFile file = selectOneAppToStart(mApplication);
            startProcess(file, false);
        }
    }
}

void ModuleManager::onSessionDbusActiveChanged(const QString &strServerName, bool isActive)
{
    if (strServerName != SS_DBUS_SERVICE) {
        return;
    }

    if (isActive) {
        // 服务变为active，初始化接口
        qDebug() << "ukui-screensaver service started";
        Q_EMIT screensaverServiceStatusChanged(isScreenLockStartup);
    } else {
        // 服务变为inactive（退出）
        qDebug() << "ukui-screensaver service exited";

        // 在快速启动模式下，第一次退出时重新拉起
        if (mScreensaverFirstExit && mLockScreen.isValid()) {
            qDebug() << "ukui-screensaver first exit, restarting...";
            mScreensaverFirstExit = false;
            QString name = QFileInfo(mLockScreen.fileName()).fileName();
            if (mNameMap.contains(name)) {
                mNameMap.remove(name);
            }
            // 重新拉起ukui-screensaver
            QStringList extArgs;
            if (mNotificationFlag) {
                extArgs << "--lock-startup";
            }
            startProcess(mLockScreen, true, extArgs);

            qDebug() << "ukui-screensaver restarted";
        }
    }
}

// #include "modulemanager.moc"
