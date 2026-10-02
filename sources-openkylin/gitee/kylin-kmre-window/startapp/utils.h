/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#ifndef _UTILS
#define _UTILS

#include <pwd.h>
#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <sys/utsname.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <vector>

#include <QString>
#include <QObject>
#include <QStandardPaths>

class Utils
{
public:
    static void prepareEnvironment();
    static void startContainer();
    static void startContainerSilently();
    //static void awakeAndroidDesk();
    static bool getOptionsFromArgument(bool &fullScreen, int &density, int &width, int &height);
    static void startApplication(const QString &pkg);
    static bool isAndroidDeskStart();
    static const QString& getUserName();
    static const QString& getUid();
    static QString readFile(const QString &path);
    static int checkSingle();
    //static void killProcess(char *strPid, char *processName);
    static void startAppDaemon();
    static bool isPathCharDevice(const std::string& path);
    static bool isKernelModuleLoaded(const std::string& moduleName);
    static bool isPathFileType(const std::string &path, mode_t fileType);
    //static bool isWayland();
    static QString getContainerName();

private:
    static bool launchApplication(const QString &pkgName, bool fullScreen, int width, int height, int density);
};

#endif // _UTILS
