/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
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

#ifndef __PRE_INSTALL_APP_MANAGER__
#define __PRE_INSTALL_APP_MANAGER__

#include <QString>
#include <QVector>

class PreInstallAppManager
{
public:
    static void preInstallApks();
    static void waitForPreInstallFinished();

private:
    typedef struct {
        QString appName;
        QString version;
        int status;
    }PreInstalledApkStatus;

    typedef enum {
        ePreInstallNull,
        ePreInstallFailed,
        ePreInstallWaiting,
        ePreInstallSucceed
    }PRE_INSTALL_STATUS;

    typedef struct {
        QString pkgName;
        QString version;
        bool needinstallDesktop;
        bool needInstallAtBoot;
    }PreInstallApkConf;
    
    static QString preInstalledFilePath;
    static QStringList preInstallApkList;
    static QVector<PreInstalledApkStatus> getPreInstalledApps(QString filePath);
    static bool isApkPreInstalled(QVector<PreInstalledApkStatus> apps, QString pkgName);
    static PRE_INSTALL_STATUS getApkPreInstallStatus(QString pkgName);
};

#endif //__PRE_INSTALL_APP_MANAGER__