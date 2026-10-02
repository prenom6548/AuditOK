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
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>
#include <QGSettings>
#include <QTimer>

#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <ukui-log4qt.h>
#include "qtsinglecoreapplication.h"
#include "utils.h"
#include "dbusupperinterface.h"
#include "screensaveradaptor.h"
#include "personalizeddata.h"
#include "msysdbus.h"

int main(int argc, char *argv[])
{
    initUkuiLog4qt("ukui-screensaver-backend");
    // 重启或关机时不被session关掉
    qunsetenv("SESSION_MANAGER");
    QString strDisplay = "";
    if (QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        strDisplay = QLatin1String(getenv("WAYLAND_DISPLAY"));
    } else {
        strDisplay = QLatin1String(getenv("DISPLAY"));
    }
    QString singleId = QString("ukui-screensaver-backend"+strDisplay);
    QtSingleCoreApplication a(singleId, argc, argv);
    if (a.isRunning()) {
        qDebug() << "There is an instance running";
        return 0;
    }
    qInfo()<<"ukui screensaver backend Started!!";
    // 会话管理器退出时，锁屏后端也退出
    prctl(PR_SET_PDEATHSIG, SIGKILL);
    //命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Backend for the ukui ScreenSaver."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    QCommandLineOption lockStartupOption(QStringLiteral("lock-startup"),
                                  QCoreApplication::translate("main", "lock the screen by startup"));
    parser.addOptions({lockStartupOption});
    parser.process(a.arguments());

    KYLINUSERDATAMNG::instance();

    // 检查该程序是否已经有实例在运行
    QDBusInterface *checkInterface =
            new QDBusInterface("org.freedesktop.DBus",
                               "/org/freedesktop/DBus",
                               "org.freedesktop.DBus",
                               QDBusConnection::sessionBus());
    QDBusReply<bool> ret = checkInterface->call("NameHasOwner",
                                               "cn.kylinos.ScreenSaver");
    if(ret.value()) {
        qDebug() << "There is an instance running";
        return EXIT_FAILURE;
    }

    // 注册DBus
    DbusUpperInterface *interface = new DbusUpperInterface();
    ScreenSaverAdaptor adaptor(interface);

    QDBusConnection service = QDBusConnection::sessionBus();
    QString sessionDbus = SS_DBUS_SERVICE;
    if(!service.registerService(sessionDbus)) {
        QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");;
        sessionDbus = QString("%1%2").arg(QString(SS_DBUS_SERVICE)).arg(displayNum);
        if(!service.registerService(sessionDbus)) {
            qDebug() << service.lastError().message();
            return EXIT_FAILURE;
        }
    }
    if(!service.registerObject(SS_DBUS_PATH, SS_DBUS_SERVICE, &adaptor,
                               QDBusConnection::ExportAllSlots |
                               QDBusConnection::ExportAllSignals)) {
        qDebug() << service.lastError().message();
        return EXIT_FAILURE;
    }
    qDebug() << service.baseService();

    QObject::connect(checkInterface, SIGNAL(NameLost(QString)),
                     interface, SLOT(onNameLost(QString)));

    if (parser.isSet(lockStartupOption)) {
        QTimer::singleShot(0, interface, [=](){
            interface->LockStartupMode();
        });
    }

    MSysDbus *m_sysDbus = new MSysDbus(interface,nullptr);

    QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
    QString sysService = "org.ukui.screensaver._" + displayNum;
    if (!QDBusConnection::systemBus().registerService("org.ukui.screensaver")) {
        qDebug()<<"registerService failed";
    }

    QDBusConnection::systemBus().registerObject("/", "org.ukui.screensaver",m_sysDbus, QDBusConnection::ExportAllSlots);

    return a.exec();
}
