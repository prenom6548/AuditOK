/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include <QApplication>
#include <QTranslator>
#include <syslog.h>
#include <QDBusReply>
#include <QLocale>
#include <QDir>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QDesktopWidget>
#include <QDBusInterface>
#include <QEventLoop>
#include <signal.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <QProcess>
#include <ukui-log4qt.h>
#include <QtSingleApplication>

#include "utils.h"
#include "commonfunc.h"
#include "lockdialogmodel.h"
#include "lockdialogperformer.h"
#include "fullbackgroundwidget.h"
#include "screensaverwndadaptor.h"
#include "pluginsloader.h"
#include "languagesetting.h"
#include "displayservice.h"

FullBackgroundWidget *window = nullptr;

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 0;
    syslog(LOG_INFO, "[ukui-screensaver-dialog] startup!!");
    initUkuiLog4qt("ukui-screensaver-dialog");

    // 检查并设置 XDG_DATA_DIRS 环境变量
    QByteArray xdgDataDirs = qgetenv("XDG_DATA_DIRS");
    if (xdgDataDirs.isEmpty()) {
        // 如果环境变量不存在，设置为 /usr/share:
        qputenv("XDG_DATA_DIRS", QByteArray("/usr/share"));
    } else {
        QStringList paths = QString(xdgDataDirs).split(':', Qt::SkipEmptyParts);
        bool hasUsrShare = false;
        for (const QString &path : paths) {
            if (path == "/usr/share") {
                hasUsrShare = true;
                break;
            }
        }
        if (!hasUsrShare) {
            // 如果环境变量存在但不包含 /usr/share，则在前面添加
            QString newXdgDataDirs = QString("/usr/share:") + QString(xdgDataDirs);
            qputenv("XDG_DATA_DIRS", newXdgDataDirs.toUtf8());
        }
    }

    // 重启或关机时不被session关掉
    qunsetenv("SESSION_MANAGER");

    qputenv("QT_QPA_PLATFORMTHEME", QByteArray("ukui"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    LockDialogModel lockDialogModel;
    LockDialogPerformer performer(&lockDialogModel);

    QString strDisplay = "";
    if (QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        strDisplay = QLatin1String(getenv("WAYLAND_DISPLAY"));
    } else {
        strDisplay = QLatin1String(getenv("DISPLAY"));
    }
    QString id = QString("ukui-screensaver-dialog" + strDisplay);
    QtSingleApplication app(id, argc, argv);
    // 锁屏后端退出时，锁屏前端也退出
    prctl(PR_SET_PDEATHSIG, SIGKILL);

    performer.initExtraData();
    LockDialogModel::CommandLineArgs cmdArgs;
    if (!lockDialogModel.parseCmdArguments(app.arguments(), cmdArgs)) {
        return 0;
    }

    if (!isGreeterMode()) {
        // 需要判断参数是否是session-tools
        if (!cmdArgs.isSessionTools && !cmdArgs.isLockStartup) {
            // 试用模式不起锁屏
            checkIslivecd();
        }
    }

    if (app.isRunning()) {
        QString strArguments = QApplication::arguments().join(",");
        app.sendMessage(strArguments);
        qInfo() << "ukui screensaver dialog is running!";
        return EXIT_SUCCESS;
    }
    if (!lockDialogModel.isUseWayland()) {
        setCursorCenter();
    }
    // 加载插件实例
    PluginsLoader *pluginsLoader = &PluginsLoader::instance();
    pluginsLoader->start(QThread::HighestPriority);
    pluginsLoader->wait();

    QApplication::setSetuidAllowed(true);

    QObject::connect(&lockDialogModel, &LockDialogModel::requestUnlockSession, [=]() {
        if (window) {
            window->onCloseScreensaver();
        } else {
            exit(0);
        }
    });
    if (!lockDialogModel.sessionActive()) {
        QEventLoop *loopTemp = new QEventLoop(&app);
        QObject::connect(&lockDialogModel, &LockDialogModel::sessionActiveChanged, [loopTemp](bool isActive) {
            qDebug() << "sessionActiveChanged:" << isActive;
            if (isActive && loopTemp->isRunning()) {
                loopTemp->quit();
            }
        });
        QObject::connect(&lockDialogModel, &LockDialogModel::requestLockSession, [loopTemp, &lockDialogModel]() {
            qDebug() << "session requestLock:" << lockDialogModel.sessionActive();
            if (loopTemp->isRunning() && lockDialogModel.sessionActive()) {
                loopTemp->quit();
            }
        });
        loopTemp->exec();
    }

    QObject::connect(
        &app, SIGNAL(messageReceived(const QString &)), &lockDialogModel, SLOT(onRunningMessage(const QString &)));

    qInfo() << "Start " << app.arguments();
    // 加载翻译文件
    if (isGreeterMode()) {
        UserInfoPtr ptrUserInfo = lockDialogModel.findUserByName(lockDialogModel.defaultUserName());
        if (ptrUserInfo) {
            LanguageSetting::instance()->onLanguageChanged(ptrUserInfo->lang());
        } else {
            LanguageSetting::instance()->onLanguageChanged(QLocale::system().name());
        }
    } else {
        LanguageSetting::instance()->onLanguageChanged(QLocale::system().name());
    }

    window = new FullBackgroundWidget(&lockDialogModel, cmdArgs.isLockStartup);

    // 注册DBus
    ScreenSaverWndAdaptor adaptorWnd(window);
    QDBusConnection service = QDBusConnection::sessionBus();
    QString sessionDbus = SSWND_DBUS_SERVICE;
    if (!service.registerService(SSWND_DBUS_SERVICE)) {
        QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
        sessionDbus = QString("%1%2").arg(QString(SSWND_DBUS_SERVICE)).arg(displayNum);
        if (!service.registerService(sessionDbus)) {
            qDebug() << service.lastError().message();
            return 1;
        }
    }
    if (!service.registerObject(
            SSWND_DBUS_PATH,
            SSWND_DBUS_SERVICE,
            &adaptorWnd,
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qDebug() << service.lastError().message();
        return 1;
    }
    qDebug() << service.baseService();

    if (cmdArgs.isBlank) {
        window->init();
        Q_EMIT lockDialogModel.showBlankScreensaver(cmdArgs.nBlankDelay, cmdArgs.isBlankHasLock);
    }

#ifndef USE_INTEL
    syslog(LOG_INFO, "[ukui-screensaver-dialog] window show!!");
    window->show();
    window->activateWindow();
    syslog(LOG_INFO, "[ukui-screensaver-dialog] window show done!!");
#endif
    window->init();
    if (!cmdArgs.strSwitchToUser.isEmpty()) {
        Q_EMIT lockDialogModel.showSwitchToUser(cmdArgs.strSwitchToUser);
    }

    if (cmdArgs.isLock) {
        Q_EMIT lockDialogModel.showLock(false);
    }

    if (cmdArgs.isLockStartup) {
        Q_EMIT lockDialogModel.showLock(true);
    }

    if (cmdArgs.isSessionIdle) {
        Q_EMIT lockDialogModel.showSessionIdle();
    }

    if (cmdArgs.isLockScreensaver) {
        Q_EMIT lockDialogModel.showLockScreensaver();
    }

    if (cmdArgs.isScreenSaver) {
        Q_EMIT lockDialogModel.showScreensaver();
    }

    if (cmdArgs.isSessionTools) {
        Q_EMIT lockDialogModel.showSessionTools();
    }

    if (cmdArgs.isAppBlock != -1) {
        Q_EMIT lockDialogModel.showAppBlockWindow(cmdArgs.isAppBlock);
    }

    if (cmdArgs.isMultiUserBlock != -1) {
        Q_EMIT lockDialogModel.showMultiUsersBlockWindow(cmdArgs.isMultiUserBlock);
    }

    if (cmdArgs.isSwitchUser) {
        Q_EMIT lockDialogModel.showSwitchUserLock();
    }

    if (cmdArgs.isShutdownAnimation != -1) {
        Q_EMIT lockDialogModel.showShutdownAnimation(cmdArgs.isShutdownAnimation);
    }

    KillFocusOfKydroid();
    syslog(LOG_INFO, "[ukui-screensaver-dialog] application event loop begin!!");
    return app.exec();
}
