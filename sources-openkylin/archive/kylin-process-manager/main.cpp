/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "log-utils.h"
#include "processmanager.h"
#include "processmanagerservice.h"

#include "loghelper.h"
#include "signalhandler.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTranslator>

void setLauncherEnvironment()
{
    const char *gtkUsePortalEnvKey = "GTK_USE_PORTAL";
    const char *glibForceUsePortalEnvKey = "GLIB_FORCE_USE_PORTAL";
    const char *glibUseKylinProcessManagerEnvKey = "GLIB_USE_KYLIN_PROCESS_MANAGER";

    QByteArray gtkUsePortalEnvValue = qgetenv(gtkUsePortalEnvKey);
    QByteArray glibForceUsePortalEnvValue = qgetenv(glibForceUsePortalEnvKey);
    QByteArray glibUseKylinProcessManagerEnvValue = qgetenv(glibUseKylinProcessManagerEnvKey);

    // 初始化glib
    qunsetenv(gtkUsePortalEnvKey);
    qunsetenv(glibForceUsePortalEnvKey);
    qunsetenv(glibUseKylinProcessManagerEnvKey);

    // 需要通过一些接口调用到glib_should_use_portal完成glib的初始化
    GFile *tmp_file = g_file_new_for_uri("file:///nofile");
    g_file_trash(tmp_file, 0, 0);
    g_object_unref(tmp_file);

    qputenv(gtkUsePortalEnvKey, gtkUsePortalEnvValue);
    qputenv(glibForceUsePortalEnvKey, glibForceUsePortalEnvValue);
    qputenv(glibUseKylinProcessManagerEnvKey, glibUseKylinProcessManagerEnvValue);

    qputenv("XDG_CURRENT_DESKTOP", "UKUI");
    // qputenv("QT_QPA_PLATFORMTHEME", "ukui");
}

void initEnvironment()
{
    setLauncherEnvironment();
}

int main(int argc, char *argv[])
{
    //LogUtils::initLogFile("kylin-process-manager");
    //qInstallMessageHandler(LogUtils::messageOutput);

    QElapsedTimer timer;
    timer.start();
    //qDebug() << "==========process manager service starting==========";
    PRINT_NOTICE("==========process manager service starting==========\n");
    initEnvironment();

    QCoreApplication a(argc, argv);

    SignalHandler::setupHandlers();
    SignalHandler signalHandler;
    QObject::connect(&signalHandler, &SignalHandler::sigTermReceived, &a, &QCoreApplication::quit, Qt::QueuedConnection);

    QTranslator translator;
    translator.load(QLocale::system().name(), "/usr/share/kylin-process-manager/");
    a.installTranslator(&translator);

    ProcessManager processManager;
    ProcessManagerService processManagerService(&processManager);

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected()) {
        PRINT_NOTICE("DBus session bus not connected: %s\n", connection.lastError().message().toStdString().c_str());
        return 0;
    }

    if (!connection.registerService("com.kylin.ProcessManager")){
        PRINT_NOTICE("Register process manager dbus service failed\n");
        return 0;
    }

    if (!connection.registerObject("/com/kylin/ProcessManager", &processManager)) {
        PRINT_NOTICE("Register process manager object failed\n");
        return 0;
    }

    if (!connection.registerObject("/com/kylin/ProcessManager/AppWhitelist", processManager.app_whitelist.get())) {
        PRINT_NOTICE("Register whitelist dbus service failed: %s\n", connection.lastError().message().toStdString().c_str());
        return 0;
    }
    
    if (!connection.registerObject("/com/kylin/ProcessManager/AppLauncher", processManager.app_launch_manager.get())) {
        PRINT_NOTICE("Register app launcher dbus service failed: %s\n", connection.lastError().message().toStdString().c_str());
        return 0;
    }
    
    //qDebug() << "==========process manager service started in" << timer.elapsed() << "ms==========";
    PRINT_NOTICE("==========process manager service started in %lld ms==========\n", timer.elapsed());
    return a.exec();
}
