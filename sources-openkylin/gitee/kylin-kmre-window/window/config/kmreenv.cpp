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

#include "kmreenv.h"
#include "common.h"
#include "common/utils.h"

#include <QSettings>
#include <QGSettings>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>

#include <atomic>
#include <syslog.h>

namespace KmreEnv {
    const QString CommonHWPlatformStr = "pc";
    const QString KylinOSV10ProStr    = "kylin_v10_pro";
    const QString KylinOSV10Str       = "kylin_v10";
    const QString OpenKylinOSStr      = "openkylin";
    QString OSPlatformString = "";
    QString HWPlatformString = "";
    QString ContainerSocketPath = "";
    QString AndroidPath = "";
    QString AndroidDataPath = "";
    QString ConfigPath = "";
    const QString GlobalConfigPath = "/usr/share/kmre/";
    QString KmreConfigFile = "";
    bool KmreEnvStopped = false;

    void initPaths();
    void checkPlatform();
}

bool KmreEnv::isKmreEnvStopped() 
{
    return KmreEnvStopped;
}

void KmreEnv::kmreEnvStopped() 
{
    KmreEnvStopped = true;
}

bool KmreEnv::isKylinOSPro()
{
    if (OSPlatformString.isEmpty()) {
        checkPlatform();
    }
    return (OSPlatformString != KylinOSV10Str);
}

bool KmreEnv::isOpenKylin()
{
    if (OSPlatformString.isEmpty()) {
        checkPlatform();
    }
    return (OSPlatformString == OpenKylinOSStr);
}

void KmreEnv::checkPlatform()
{
    HWPlatformString = CommonHWPlatformStr;
    OSPlatformString = KylinOSV10Str;

    QFile file("/etc/lsb-release");
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString &content = QString::fromUtf8(file.readAll());
            file.close();

            if (content.contains("Kylin V10 Professional") || 
                content.contains("Kylin V10 SP1") || 
                content.contains("Kylin V10 SP2")) {
                OSPlatformString = KylinOSV10ProStr;
            }

            if (content.contains("openKylin")) {
                OSPlatformString = OpenKylinOSStr;
            }
        }
    }

    syslog(LOG_INFO, "[%s] HW Platform:'%s', OS Platform:'%s'", 
        __func__, HWPlatformString.toStdString().c_str(), OSPlatformString.toStdString().c_str());
}

void KmreEnv::initPaths()
{
    AndroidPath = QString("/var/lib/kmre/%1").arg(kmre::utils::makeContainerName());
    ContainerSocketPath = QString("%1/sockets").arg(AndroidPath);
    QDir socketPath(ContainerSocketPath);
    if (!socketPath.mkpath(ContainerSocketPath)) {
        syslog(LOG_ERR, "[%s] Kmre socket path '%s' doesn't exist and created failed!", 
            __func__, ContainerSocketPath.toStdString().c_str());
    }

    AndroidDataPath = QString("/var/lib/kmre/data/%1").arg(kmre::utils::makeContainerName());
    QDir dataPath(AndroidDataPath);
    if (!dataPath.exists()) {
        syslog(LOG_WARNING, "[%s] Kmre data path '%s' have not created yet!",
            __func__, AndroidDataPath.toStdString().c_str());
    }

    ConfigPath = QString("%1/.config/kmre").arg(QDir::homePath());
    QDir configPath(ConfigPath);
    if (!configPath.mkpath(ConfigPath)) {
        syslog(LOG_ERR, "[%s] Kmre config path '%s' doesn't exist and created failed!", 
            __func__, ConfigPath.toStdString().c_str());
    }

    KmreConfigFile = ConfigPath + "/kmre.ini";
    if (!QFile(KmreConfigFile).exists()) {
        syslog(LOG_WARNING, "[%s] Kmre config file '%s' isn't exist!", 
            __func__, KmreConfigFile.toStdString().c_str());
    }

    syslog(LOG_INFO, "[%s] kmre env paths initialized.", __func__);
}

QString KmreEnv::GetContainerSocketPath()
{
    if (ContainerSocketPath.isEmpty()) {
        initPaths();
    }
    return ContainerSocketPath;
}

QString KmreEnv::GetAndroidPath()
{
    if (AndroidPath.isEmpty()) {
        initPaths();
    }
    return AndroidPath;
}

QString KmreEnv::GetAndroidDataPath()
{
    if (AndroidDataPath.isEmpty()) {
        initPaths();
    }
    return AndroidDataPath;
}

QString KmreEnv::GetConfigPath()
{
    if (ConfigPath.isEmpty()) {
        initPaths();
    }
    return ConfigPath;
}

QString KmreEnv::GetGlobalConfigPath()
{
    return GlobalConfigPath;
}

QString KmreEnv::GetKmreConfigFile()
{
    if (KmreConfigFile.isEmpty()) {
        initPaths();
    }
    return KmreConfigFile;
}

void KmreEnv::envCheck()
{
    KmreEnv::checkPlatform();
    KmreEnv::initPaths();
}

