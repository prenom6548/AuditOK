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



#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTranslator>
#include <QLocale>
#include <QStandardPaths>
#include <QLibraryInfo>
#include <QDir>
#include <QMetaType>

#include <sys/inotify.h>
#include <fcntl.h>
#include <syslog.h>

#include <QGSettings>
#include <QDir>
#include <QObject>

#include <singleapplication.h>
#include "gsettings_monitor.h"
#include "device_monitor.h"
#include "mainwindow.h"
#include "common.h"
#include "log.h"
#include "find_and_match_ppds.h"
#include "daemonipcdbus.h"
#include "pop_window_manager.h"
#include "information_collector.h"
#include "windowmanage.hpp"
#include "base_info.h"


static bool WaylandPlatform() {
    const auto platform = QGuiApplication::platformName();
    qInfo() << "current platform: " << platform;
    if (platform.startsWith("wayland", Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

void addTrans()
{
//加载中文
    QString qtTranslationsPath;
    qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);// /usr/share/qt5/translations
    QString kylinPrinterTranslationsPath;
#ifndef TEST_DEBUG
    if (QDir("/usr/share/kylin-printer/translations").exists()) {
        kylinPrinterTranslationsPath = "/usr/share/kylin-printer/translations";
    }
    else {
        kylinPrinterTranslationsPath = QApplication::applicationDirPath() + "/.qm";
    }

#else
    kylinPrinterTranslationsPath = QString(PROJECT_SOURCE_DIR) + "/translations";
#endif
    qDebug () << kylinPrinterTranslationsPath;
    QString locale = QLocale::system().name();
    static QTranslator trans_global, trans_menu, trans_dict, trans_sdk, trans_driver_install;

    if(!trans_global.load("kylin-printer_" + locale + ".qm", kylinPrinterTranslationsPath))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QApplication::installTranslator(&trans_global);

    if(!trans_dict.load("kylin-printer_dict_" + locale + ".qm", kylinPrinterTranslationsPath))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QApplication::installTranslator(&trans_dict);

    if(!trans_menu.load("qt_" + locale + ".qm", qtTranslationsPath))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QApplication::installTranslator(&trans_menu);

    if(!trans_sdk.load(":/translations/gui_" + locale + ".qm"))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        QApplication::installTranslator(&trans_sdk);

    if(!trans_driver_install.load(QLocale(), "driver-acquirer-widget", "_", "/usr/share/driver-acquirer-widget/translations"))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else {
        QApplication::installTranslator(&trans_driver_install);
    }
}

int main(int argc, char *argv[])
{
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "ukui-shell");
    // 统一日志输出
#ifndef TEST_DEBUG
    qInstallMessageHandler(logOutput);
#else
    qInstallMessageHandler(myMessageOutput);
#endif
    InformationCollector::getInstance().addMessage("打印机应用启动!");
    qRegisterMetaType<ProtocolType>("ProtocolType");
    qRegisterMetaType<DeviceInformation>("DeviceInformation");
    qRegisterMetaType<DeviceInformation>("DeviceInformation&");
    qRegisterMetaType<ppdPrinterMap>("ppdPrinterMap");
    qRegisterMetaType<QString>("QString&");

    //高清屏幕自适应,适配990高DPI
    kabase::WindowManage::setScalingProperties();

    kdk::QtSingleApplication app(argc, argv);

    if (app.isRunning()) {
        qDebug() << "is running";
        app.sendMessage("running");
        return 0;
    }

    QStringList cmd = app.arguments();
    cmd.removeFirst();
    if (checkCommand(cmd) == CHECK_FALSE) {
        qDebug() << "UNKNOWN COMMAND:" << cmd;
        return 0;
    }

    if(!GsettingsMonitor::getInstance()->gsettingsCheck())
    {
        app.setProperty("noChangeSystemFontSize",true);
    }

    addTrans();
    BaseInfo::instance();

    app.setApplicationName(BaseInfo::instance().getAppDisplayName());
    app.setApplicationVersion(PROJECT_VERSION);
    app.setWindowIcon(QIcon::fromTheme("kylin-printer"));

    kdk::WindowId wayland_mainwindow_id;
    if (WaylandPlatform()) {
        QObject::connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded, [&wayland_mainwindow_id](const kdk::WindowId &window_id) {
            if (getpid() == (int)kdk::WindowManager::getPid(window_id)) {
                wayland_mainwindow_id = window_id;
            }
        });
    }

    MainWindow::getInstance()->show();

    QTimer::singleShot(200, [&app, &wayland_mainwindow_id]() {

        MainWindow::getInstance()->init();
        /* 最小化拉起 */
        app.setActivationWindow(MainWindow::getInstance());

        QObject::connect(&app, &kdk::QtSingleApplication::messageReceived, [&wayland_mainwindow_id](QString message) {
            auto data = message.toLocal8Bit();
            auto index = data.indexOf('\n');
            if (data == "running") {
                // 最小化拉起
                if (WaylandPlatform()) {
                    kdk::WindowManager::activateWindow(wayland_mainwindow_id);
                }
            }
        });

        QObject::connect(GsettingsMonitor::getInstance(),SIGNAL(sysFontSizeUpdate(QFont)),
                        MainWindow::getInstance(),SLOT(setQSSFontSize(QFont)));
        QObject::connect(MainWindow::getInstance(),SIGNAL(getCurFontSize()),
                        GsettingsMonitor::getInstance(),SLOT(update()));

        GsettingsMonitor::getInstance()->update();

        DaemonIpcDbus::getInstance();


        g_findAndMatchWorkThread->start();
        g_findAndMatchWorkThread->updatePPDsMap();
        // MainWindow::getInstance()->show();
        return ;
    });



#ifdef UNIT_TEST
    return 0;
#else
    return app.exec();
#endif
}
