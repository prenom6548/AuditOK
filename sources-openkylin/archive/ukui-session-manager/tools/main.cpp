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
#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QDir>
#include <QTranslator>
#include <QCommandLineParser>
#include <QString>
#include <QSoundEffect>
#include <QTimer>
#include <QGSettings/QGSettings>
#include <X11/Xlib.h>

#include <QMessageBox>
#include <QDBusInterface>
#include <QDBusReply>
#include <QSharedPointer>

#include "ukuipower.h"
#include "mainwindow.h"
#include "loginedusers.h"
#include "lockchecker.h"
#include <QPushButton>

#include <KWindowSystem>

#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <ukui-log4qt.h>
#include <QStandardPaths>
#include "plasma-shell-manager.h"

#ifdef signals
#undef signals
#endif

bool messageboxCheck()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(QObject::tr("ukui-session-tools"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
//    msgBox.setModal(false);
    msgBox.setText(QObject::tr("Multiple users are logged in at the same time.Are you sure you want to close this system?"));
    QPushButton *confirmButton = msgBox.addButton(QObject::tr("confirm"), QMessageBox::ActionRole);
    QPushButton *cancelButton = msgBox.addButton(QObject::tr("cancel"), QMessageBox::RejectRole);
    //QPushButton *confirmButton = msgBox.addButton(QObject::tr("confirm"), QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == cancelButton) {
        qDebug() << "cancel!";
        return false;
    } else if (msgBox.clickedButton() == confirmButton) {
        qDebug() << "confirm!";
        return true;
    } else {
        return false;
    }
}

void messagecheck()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
    QString t1 = QObject::tr("System update or package installation in progress,this function is temporarily unavailable.");
    QString t2 = QObject::tr("System backup or restore in progress,this function is temporarily unavailable.");

    QString lockDirPath = QDir::tempPath() + "/lock";

    FILE *file_update = fopen((lockDirPath + "/kylin-update.lock").toLocal8Bit().constData(), "r");
    if (file_update != NULL) {
        msgBox.setText(t1);
        fclose(file_update);
    }

    FILE *file_backup = fopen((lockDirPath + "/kylin-backup.lock").toLocal8Bit().constData(), "r");
    if (file_backup != NULL) {
        msgBox.setText(t2);
        fclose(file_backup);
    }

    QPushButton *cancelButton = msgBox.addButton(QObject::tr("OK"), QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == cancelButton) {
        qDebug() << "OK!";
    }
}

bool playShutdownMusic(UkuiPower &powermanager, int num, QTimer *up_to_time)
{
    bool        play_music = false;
    QGSettings *gs         = new QGSettings("org.ukui.session", "/org/ukui/desktop/session/");
    if (num == 4) {
        play_music = gs->get("logout-music").toBool();
    }
    if (num == 6 || num == 8) {
        play_music = gs->get("poweroff-music").toBool();
    }
    if (num == 0 || num == 1 || num == 2) {
        play_music = false;
    }

    gs->set("win-key-release", false);
    static int action = num;

    if (num == 4 || num == 0) {
        QDBusInterface dbus("org.gnome.SessionManager", "/org/gnome/SessionManager",
                            "org.gnome.SessionManager", QDBusConnection::sessionBus());
        if (!dbus.isValid()) {
            qWarning() << "dbusCall: QDBusInterface is invalid";
            return false;
        }

        QDBusMessage msg;
        if (num == 4) {
            msg = dbus.call("emitStartLogout");
        }

        if (num == 0) {
            msg = dbus.call("emitPrepareForSwitchuser");
        }

        if (!msg.errorName().isEmpty()) {
            qWarning() << "Dbus error: " << msg;
        }
    }

    if (play_music) {
        // up_to_time and soundplayer can not be define out of this if().
        // otherwise run ukui-session-tools --suspend with segmente error.
        // because they will be delate at the end of the playShutdownMusic().
        QObject::connect(up_to_time, &QTimer::timeout, [&]() {
            powermanager.doAction(UkuiPower::Action(action));
            exit(0);
        });

        QString xdg_session_type = qgetenv("XDG_SESSION_TYPE");
        if (num == 6 || num == 8) {
            if (xdg_session_type == "wayland") {
                QProcess::startDetached("paplay /usr/share/ukui/ukui-session-manager/shutdown.wav");
            } else {
                QProcess::startDetached("aplay /usr/share/ukui/ukui-session-manager/shutdown.wav");
            }
            up_to_time->start(5000);
        } else if (num == 4) {
            if (xdg_session_type == "wayland") {
                QProcess::startDetached("paplay /usr/share/ukui/ukui-session-manager/logout.wav");
            } else {
                QProcess::startDetached("aplay /usr/share/ukui/ukui-session-manager/logout.wav");
            }
            up_to_time->start(2000);
        } else {
            qDebug() << "error num";
            return false;
        }
    } else {
        powermanager.doAction(UkuiPower::Action(action));
        exit(0);
    }
    return false;
}

/**
 * @brief 判断当前是否处于锁屏
 */
bool screensaverIsActive()
{
    QString SS_DBUS_SERVICE = "org.ukui.ScreenSaver";
    QString SS_DBUS_PATH = "/";
    QString SS_DBUS_INTERFACE = "org.ukui.ScreenSaver";
    QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
    qDebug() << "dispaly = " << displayNum ;
    QString sessionDbus = QString("%1%2").arg(SS_DBUS_SERVICE).arg(displayNum);
    QDBusInterface *interface = new QDBusInterface(sessionDbus, SS_DBUS_PATH, SS_DBUS_INTERFACE);

    if (!interface->isValid()) {
        qDebug() << "interface not valid";
        delete interface;
        interface = new QDBusInterface(SS_DBUS_SERVICE, SS_DBUS_PATH, SS_DBUS_INTERFACE);
    }

    QDBusMessage response = interface->call("GetLockState");
    //判断method是否被正确返回
    if (response.type() == QDBusMessage::ReplyMessage) {
        //从返回参数获取返回值
        bool isActive = response.arguments()[0].toBool();
        qDebug() << "screensaver isActive:" << isActive;
        return isActive;
    } else {
        qDebug() << "QDBusMessage failed!";
        return false;
    }
}

int main(int argc, char* argv[])
{
    initUkuiLog4qt("ukui-session-tools");

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))

  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))

  QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

#endif

    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    int fd = open(QString(homePath.at(0) + "/.config/ukui-session-tools%1.lock").arg(getenv("DISPLAY")).toUtf8().data(),
              O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        exit(1);
    }

    if (lockf(fd, F_TLOCK, 0)) {
        qDebug() << "Can't lock single file, ukui-session-tools is already running!";
        exit(0);
    }

    qunsetenv("QT_IM_MODULE");
    qunsetenv("SESSION_MANAGER");

    //关机界面按钮定制
     if (QGSettings::isSchemaInstalled("org.ukui.SettingsDaemon.plugins.globalManager")) {
         QGSettings globalSettings("org.ukui.SettingsDaemon.plugins.globalManager");
         if (globalSettings.get("disable-power-operation").toBool()) {
             exit(0);
         }
     }

    QString SS_DBUS_SERVICE = "org.ukui.ScreenSaver";
    QString SS_DBUS_PATH = "/";
    QString SS_DBUS_INTERFACE = "org.ukui.ScreenSaver";
    QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
    QString sessionDbus = QString("%1%2").arg(SS_DBUS_SERVICE).arg(displayNum);
    QSharedPointer<QDBusInterface> screensaverInterface(
        new QDBusInterface(sessionDbus, SS_DBUS_PATH, SS_DBUS_INTERFACE));

    if (!screensaverInterface->isValid()) {
        screensaverInterface
            = QSharedPointer<QDBusInterface>(new QDBusInterface(SS_DBUS_SERVICE, SS_DBUS_PATH, SS_DBUS_INTERFACE));
    }

    bool m_flag = false;
    QDBusReply<bool> reply = screensaverInterface->call("CheckAppVersion");
    if (reply.isValid()) {
        // use the returned value
        qDebug() << "CheckAppVersion is " << reply.value();
        m_flag = reply.value();
    } else {
        qDebug() << "CheckAppVersion function is not existed";
    }

    QApplication a(argc, argv);

    if (m_flag) {
        qDebug() << "screensaverInterface valid";
        QCommandLineParser parser;
        parser.setApplicationDescription(QApplication::tr("UKUI session tools, show the screensave dialog"));

        parser.addHelpOption();
        parser.addVersionOption();

        QCommandLineOption switchuserOption(
            QStringLiteral("switchuser"), QApplication::tr("Switch the user of this computer."));
        parser.addOption(switchuserOption);
        QCommandLineOption hibernateOption(QStringLiteral("hibernate"), QApplication::tr("Hibernate this computer."));
        parser.addOption(hibernateOption);
        QCommandLineOption suspendOption(QStringLiteral("suspend"), QApplication::tr("Suspend this computer."));
        parser.addOption(suspendOption);
        QCommandLineOption logoutOption(QStringLiteral("logout"), QApplication::tr("Logout this computer."));
        parser.addOption(logoutOption);
        QCommandLineOption rebootOption(QStringLiteral("reboot"), QApplication::tr("Restart this computer."));
        parser.addOption(rebootOption);
        QCommandLineOption shutdownOption(QStringLiteral("shutdown"), QApplication::tr("Shutdown this computer."));
        parser.addOption(shutdownOption);

        parser.process(a);

        bool flag1 = true;

        if (parser.isSet(switchuserOption)) {
            flag1 = false;
            screensaverInterface->call("SwitchUser");
        }
        if (parser.isSet(hibernateOption)) {
            flag1 = false;
            screensaverInterface->call("Hibernate");
        }
        if (parser.isSet(suspendOption)) {
            flag1 = false;
            screensaverInterface->call("Suspend");
        }
        if (parser.isSet(logoutOption)) {
            flag1 = false;
            screensaverInterface->call("Logout");
        }
        if (parser.isSet(rebootOption)) {
            flag1 = false;
            screensaverInterface->call("Reboot");
        }
        if (parser.isSet(shutdownOption)) {
            flag1 = false;
            screensaverInterface->call("PowerOff");
        }
        if (flag1) {
            screensaverInterface->call("SessionTools");
        }
        exit(0);
    }

    qDebug() << "screensaverInterface not valed";

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *settings = new QGSettings("org.ukui.style");
        if (qApp->property("noChangeSystemFontSize").isValid() && qApp->property("noChangeSystemFontSize").toBool()) {
            return 0;
        }

        double fontSize = settings->get("system-font-size").toString().toDouble();
        QString fontType = settings->get("systemFont").toString();
        if (fontSize > 0) {
            QFont font = QApplication::font();
            font.setPointSizeF(fontSize);
            font.setStyleName(fontType);
            QApplication::setFont(font);
        }
    }

    // Load ts files
    const QString locale = QLocale::system().name();
    QTranslator   translator;
    qDebug() << "local: " << locale;
    qDebug() << "path: " << QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager");
    if (translator.load(locale, QStringLiteral(UKUI_TRANSLATIONS_DIR) + QStringLiteral("/ukui-session-manager"))) {
        a.installTranslator(&translator);
    } else {
        qDebug() << "Load translations file failed!";
    }

    UkuiPower powermanager(&a);
    bool      flag = true;

    // define in the main() avoid scope error.
    QTimer *up_to_time = new QTimer();
    up_to_time->setSingleShot(true);

    QGSettings *gs = new QGSettings("org.ukui.session", "/org/ukui/desktop/session/");
    gs->set("win-key-release", true);

    //初始化窗口 Load qss file
    MainWindow *w = new MainWindow();
    
    //KWindowSystem::setType(w->winId(), NET::Logout);

    QFile qss(":/powerwin.qss");
    qss.open(QFile::ReadOnly);
    a.setStyleSheet(qss.readAll());
    qss.close();

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QApplication::tr("UKUI session tools, show the shutdown dialog without any arguments."));
    const QString VERINFO = QStringLiteral("2.0");
    a.setApplicationVersion(VERINFO);
    a.setApplicationName(QApplication::tr("ukui-session-tools"));

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption switchuserOption(
        QStringLiteral("switchuser"), QApplication::tr("Switch the user of this computer."));
    parser.addOption(switchuserOption);
    QCommandLineOption hibernateOption(QStringLiteral("hibernate"), QApplication::tr("Hibernate this computer."));
    parser.addOption(hibernateOption);
    QCommandLineOption suspendOption(QStringLiteral("suspend"), QApplication::tr("Suspend this computer."));
    parser.addOption(suspendOption);
    QCommandLineOption logoutOption(QStringLiteral("logout"), QApplication::tr("Logout this computer."));
    parser.addOption(logoutOption);
    QCommandLineOption rebootOption(QStringLiteral("reboot"), QApplication::tr("Restart this computer."));
    parser.addOption(rebootOption);
    QCommandLineOption shutdownOption(QStringLiteral("shutdown"), QApplication::tr("Shutdown this computer."));
    parser.addOption(shutdownOption);

    parser.process(a);

    if (parser.isSet(switchuserOption)) {
        flag = playShutdownMusic(powermanager, 0, up_to_time);
    }
    if (parser.isSet(hibernateOption)) {
        QVector<InhibitInfo::InhibitorInfo> res = LockChecker::listInhibitor("sleep");
        if (res.isEmpty()) {
            flag = playShutdownMusic(powermanager, 1, up_to_time);
        } else {
            w->showInhibitWarning(res, 1);
            w->show();
        }
    }
    if (parser.isSet(suspendOption)) {
        QVector<InhibitInfo::InhibitorInfo> res = LockChecker::listInhibitor("sleep");
        if (res.isEmpty()) {
            flag = playShutdownMusic(powermanager, 2, up_to_time);
        } else {
            w->showInhibitWarning(res, 2);
            w->show();
        }
    }
    if (parser.isSet(logoutOption)) {
        QVector<InhibitInfo::InhibitorInfo> res = LockChecker::listInhibitor("shutdown");
        if (res.isEmpty()) {
            flag = playShutdownMusic(powermanager, 4, up_to_time);
        } else {
            w->showInhibitWarning(res, 4);
            w->show();
        }
    }
    if (parser.isSet(rebootOption)) {
        QVector<InhibitInfo::InhibitorInfo> res = LockChecker::listInhibitor("shutdown");
        if (res.isEmpty()) {
            if (LockChecker::getLoginedUsers().count() > 1) {//提醒多个用户登录的情况
                if (messageboxCheck()) {
                    flag = playShutdownMusic(powermanager, 6, up_to_time);
                } else {
                    gs->set("win-key-release", false);
                    return 0;
                }
            } else {
                flag = playShutdownMusic(powermanager, 6, up_to_time);
            }
        } else {
            w->showInhibitWarning(res, 6);
            w->show();
        }
    }
    if (parser.isSet(shutdownOption)) {
        QVector<InhibitInfo::InhibitorInfo> res = LockChecker::listInhibitor("shutdown");
        if (res.isEmpty()) {
            if (LockChecker::getLoginedUsers().count() > 1) {//提醒多个用户登录的情况
                if (messageboxCheck()) {
                    flag = playShutdownMusic(powermanager, 8, up_to_time);
                } else {
                    gs->set("win-key-release", false);
                    return 0;
                }
            } else {
                flag = playShutdownMusic(powermanager, 8, up_to_time);
            }
        } else {
            w->showInhibitWarning(res, 8);
            w->show();
        }
    }

    if (screensaverIsActive()) {
        gs->set("win-key-release", false);
        exit(0);
    }

    if (flag) {
        w->show();
        QObject::connect(w, &MainWindow::signalTostart, [&]() {
            // 从界面点击 切换用户、睡眠和休眠 按钮，则等待界面隐藏再执行命令
            if (w->defaultnum < 3) {
                w->hide();
                QTimer::singleShot(500, [&]() {
                    playShutdownMusic(powermanager, w->defaultnum, up_to_time);
                });
            } else {
                playShutdownMusic(powermanager, w->defaultnum, up_to_time);
            }
        });
    }
    return a.exec();
}
