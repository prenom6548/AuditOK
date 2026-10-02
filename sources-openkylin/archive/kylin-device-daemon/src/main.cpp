/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include <QApplication>
#include <QDebug>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QTranslator>
#include <QTextCodec>
#include <ukui-log4qt.h>
#include "UnionVariable.h"
#include "mainwindow.h"
#include "MainController.h"
#include "fdapplication.h"

int main(int argc, char *argv[])
{
    initUkuiLog4qt("kylin-device-daemon");
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QTextCodec *codec = QTextCodec::codecForName("utf8"); //Linux
    QTextCodec::setCodecForLocale(codec);

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QString id = QString("kylin-device-daemon_%1_%2").arg(getuid()).arg(QLatin1String(getenv("DISPLAY")));
    FDApplication a(id, argc, argv);
    a.setWindowIcon(QIcon::fromTheme("system-file-manager"));

    // Process does not exit implicitly
    a.setQuitOnLastWindowClosed(false);
    if (a.isRunning()) {
        qInfo() << "kylin-device-daemon is running, now exit!";
        return 1;
    }

    // load translation file
    QString locale = QLocale::system().name();
    QTranslator translator;
    if(!translator.load("/usr/share/kylin-device-daemon/kylin-device-daemon_" + locale + ".qm")) {
        qWarning() << "/usr/share/kylin-device-daemon/" + locale << "load failed";
    }
    a.installTranslator(&translator);

    if (QApplication::desktop()->width() >= 2560) {
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
                QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
                QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
        #endif
    }

    MainController *ctrl = MainController::self();
    // single instance is running
    if (ctrl->init() != 0) {
        return 0;
    }

    QObject::connect(&a, &FDApplication::notifyWnd, ctrl, &MainController::notifyWnd);
    a.exec();

    delete ctrl;
    return 0;
}
