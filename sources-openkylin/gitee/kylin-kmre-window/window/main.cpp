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

#include <QApplication>
#include <QDebug>
#include <QLockFile>
#include <QDir>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syslog.h>
#include <sys/wait.h>

#include "utils.h"
#include "kmreapplication.h"
#include "kmreenv.h"
#include "sessionsettings.h"
#include "dbusclient.h"

#define LOG_IDENT "KMRE_kylin-kmre-window"

static bool stopKmreContainer = false;
//该函数必须先于main函数执行，防止其他某些代码早于该段代码执行，从而使QApplication创建失败。
__attribute((constructor)) 
void start_daemon()
{
    openlog(LOG_IDENT, LOG_NDELAY | LOG_NOWAIT | LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Start kylin-kmre-window daemon...");

    pid_t pid = fork();
    if (pid > 0) {
        int status;

        int ret = waitpid(pid, &status, 0);
        if (ret != pid) {
            syslog(LOG_ERR, "Wait pid failed! Errno: %d. Try to stop container now.", errno);
            stopKmreContainer = true;
        }
        else {
            if (WIFEXITED(status)) {
                syslog(LOG_DEBUG, "Kylin-kmre-window exit, return code = %d", WEXITSTATUS(status));
            }
            else {
                syslog(LOG_ERR, "Kylin-kmre-window exit unexpectly! Try to stop container now.");
                stopKmreContainer = true;
            }
        }

        if (!stopKmreContainer) {
            exit(0);
        }
    }
    else if (pid == 0) {
        syslog(LOG_INFO, "Start kylin-kmre-window...");
    }
    else {
        syslog(LOG_ERR, "Fork kylin-kmre-window failed!");
        exit(0);
    }
}


static void setApplicationEnv()
{
    syslog(LOG_INFO, "Set application environment...");

    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        qputenv("QT_QPA_PLATFORM", "wayland");
    }

    if (KmreEnv::isKylinOSPro() || KmreEnv::isOpenKylin()) {
        qputenv("QT_QPA_PLATFORMTHEME", "ukui");
    }

    setenv("AMD_DEBUG", "nodma", 1);
    setenv("R600_DEBUG", "nodma", 1);

    // 适配4K屏
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__) || \
    defined(__i386) || defined(__i386__) || defined(__i686) || defined(__i686__)
    // It seems like that AA_DontCreateNativeWidgetSiblings is needed for drm mode on x86 platform.
    if (SessionSettings::getInstance().windowUsePlatformX11()) {
        QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    }
#endif

    // read and set env from file which set by kylin-kmre-manager
    QFile env_file("/tmp/kylin-kmre-env");
    if (env_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&env_file);
        QString line = stream.readLine();
        while(!line.isNull()) {
            QStringList str_list = line.split("=", QString::SkipEmptyParts);
            if (str_list.size() == 2) {
                QString variable = str_list.at(0);
                QString value = str_list.at(1);
                if (!qputenv(variable.toStdString().c_str(), value.toStdString().c_str())) {
                    syslog(LOG_ERR, "Set env '%s' failed!", variable.toStdString().c_str());
                }
            }
            line = stream.readLine();
        }
        env_file.close();
    }
    else {
        syslog(LOG_WARNING, "Can't open env file!");
    }
}


int main(int argc, char *argv[])
{
    if (stopKmreContainer) {
        syslog(LOG_INFO, "Stop kmre container...");
        QDBusInterface kmreDbus("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus());
        kmreDbus.call("StopContainer", kmre::utils::getUserName(), (int32_t)getuid());

        kmre::utils::unLockKylinKmreWindow();
        return 0;
    }

    if (!kmre::utils::tryLockKylinKmreWindow()) {
        syslog(LOG_INFO, "Process has already run.");
        return 1;
    }
    
    atexit([]() {
        kmre::utils::unLockKylinKmreWindow();
    });

    setApplicationEnv();

    KmreApplication app(argc, argv);
    if (!app.init()) {
        syslog(LOG_ERR, "[%s] Init kmre window failed!. Exit now!", __func__);
        closelog();
        return -1;
    }

    return app.exec();
}
