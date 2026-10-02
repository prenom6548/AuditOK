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
#include "global_utils.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDebug>

#define KYSDK_SYSINFO_DLL "/usr/lib/kysdk/kysdk-system/libkysysinfo.so"
#define FACEPATH "/usr/share/ukui/faces/"
typedef char *(*PFuncGetHostCloudPlatform)();

bool isGreeterMode()
{
    static int nIsGreeter = -1;
    if (nIsGreeter < 0) {
        QString username = getenv("USER");
        if (username == "lightdm") {
            nIsGreeter = 1;
        } else {
            nIsGreeter = 0;
        }
    }
    return (bool)(nIsGreeter);
}

bool isCurUserSelf(QString strUserName)
{
    QString username = getenv("USER");
    if (username == strUserName) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief 删除给定字符串前后的空格、制表符、换行符，注意该操作会修改原字符串
 *
 * @param str 需要进行strip操作的字符串指针
 */
static void strstripspace(char *str)
{
    if (strlen(str) == 0)
        return;
    char *startPos = str;
    while (*startPos != '\0' && isspace(*startPos))
        startPos++;
    if (*startPos == '\0') {
        str[0] = 0;
        return;
    }

    char *endPos = str + strlen(str) - 1;
    while (endPos != str && isspace(*endPos))
        endPos--;

    memmove(str, startPos, endPos - startPos + 1);
    *(str + (endPos - startPos) + 1) = 0;
}

static char *kdk_system_get_hostCloudPlatform()
{
    char *cloudplat = (char *)malloc(sizeof(char) * 65);
    if (!cloudplat)
        return NULL;
#ifdef __linux__
    cloudplat[0] = 0;
    char buf[256] = { 0 };
    bool res_flag = false;
    if (geteuid() == 0) // root 用户，可以用dmidecode
    {
        FILE *pipeLine = popen("dmidecode -s chassis-manufacturer", "r");
        if (__glibc_likely(pipeLine != NULL)) {
            fgets(buf, 255 * sizeof(char), pipeLine);
            strstripspace(buf);
            if (strcmp(buf, "Huawei Inc.") == 0) // 华为云
            {
                strcpy(cloudplat, "huawei");
                res_flag = true;
            }
            pclose(pipeLine);
        }
        if (!res_flag) {
            pipeLine = popen("dmidecode -s chassis-asset-tag", "r");
            if (__glibc_likely(pipeLine != NULL)) {
                fgets(buf, 255 * sizeof(char), pipeLine);
                strstripspace(buf);
                if (strcmp(buf, "HUAWEICLOUD") == 0) // 华为云
                {
                    strcpy(cloudplat, "huawei");
                }
                pclose(pipeLine);
            }
        }
    } else // 普通用户，只能读取文件
    {
        FILE *fp = fopen("/sys/devices/virtual/dmi/id/chassis_vendor", "rt");
        if (__glibc_likely(fp != NULL)) {
            fgets(buf, 255 * sizeof(char), fp);
            strstripspace(buf);
            if (strcmp(buf, "Huawei Inc.") == 0) // 华为云
            {
                strcpy(cloudplat, "huawei");
                res_flag = true;
            }
            fclose(fp);
        }
        if (!res_flag) {
            fp = fopen("chassis_asset_tag", "r");
            if (__glibc_likely(fp != NULL)) {
                fgets(buf, 255 * sizeof(char), fp);
                strstripspace(buf);
                if (strcmp(buf, "HUAWEICLOUD") == 0) // 华为云
                {
                    strcpy(cloudplat, "huawei");
                }
                fclose(fp);
            }
        }
    }

    if (strlen(cloudplat) == 0)
        strcpy(cloudplat, "none");
#endif
    return cloudplat;
}

QString getHostCloudPlatform()
{
    static QString strPlatform = "";
    if (strPlatform.isEmpty()) {
        void *kysdkSysinfoDll = NULL;
        char *platForm = NULL;
        PFuncGetHostCloudPlatform pFuncGetCloudPlatform = NULL;
        kysdkSysinfoDll = dlopen(KYSDK_SYSINFO_DLL, RTLD_LAZY);
        if (kysdkSysinfoDll) {
            pFuncGetCloudPlatform
                = (PFuncGetHostCloudPlatform)dlsym(kysdkSysinfoDll, "kdk_system_get_hostCloudPlatform");
        }
        if (!pFuncGetCloudPlatform) {
            platForm = kdk_system_get_hostCloudPlatform();
        } else {
            platForm = pFuncGetCloudPlatform();
        }
        if (platForm) {
            strPlatform = platForm;
            free(platForm);
            platForm = NULL;
        }
        if (kysdkSysinfoDll) {
            dlclose(kysdkSysinfoDll);
            kysdkSysinfoDll = NULL;
        }
    }
    return strPlatform;
}

bool isCommunity()
{
    static int sIsCommunity = -1;
    if (sIsCommunity == -1) {
        QString filename = "/etc/os-release";
        QSettings osSettings(filename, QSettings::IniFormat);

        QString versionID = osSettings.value("VERSION_ID").toString();

        if (versionID.compare("22.04", Qt::CaseSensitive)) {
            sIsCommunity = 0;
        } else {
            sIsCommunity = 1;
        }
    }
    return (bool)(sIsCommunity);

}

static inline void strstrip(char *str, char ch)
{
    if (strlen(str) == 0)
        return;
    char *startPos = str;
    while (*startPos != '\0' && *startPos == ch)
        startPos++;
    if (*startPos == '\0')
    {
        str[0] = 0;
        return;
    }

    char *endPos = str + strlen(str) - 1;
    while (endPos != str && *endPos == ch)
        endPos --;

    memmove(str, startPos, endPos - startPos + 1);
    *(str + (endPos - startPos) + 1) = 0;
}



char *kdk_system_get_systemName()
{
    char *sysname = NULL;
#ifdef __linux__
    FILE *fp = fopen("/etc/os-release", "r");
    if (!fp)
        return NULL;

    char buf[1024] = {0};
    while (fgets(buf, 1024, fp))
    {
        if (strncmp(buf, "NAME", strlen("NAME")) == 0)
        {
            sysname = strdup((char *)buf + strlen("NAME") + 1);
            break;
        }
    }


    if (!sysname)
        return NULL;
    strstrip(sysname, '\n');
    strstrip(sysname, '\"');

    fclose(fp);
#endif
    return sysname;
}


bool isOpenkylin()
{
    QString systemName = QString(QLatin1String(kdk_system_get_systemName()));
    if (systemName.compare("openkylin", Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

QString getDefaultFace()
{
    // 遍历头像目录
    QDir facesDir = QDir(FACEPATH);
    foreach (QString filename, facesDir.entryList(QDir::Files)) {
        QString fullface = QString("%1%2").arg(FACEPATH).arg(filename);
        // 社区版不加载商业默认头像
        if ((isCommunity() || isOpenkylin()) && fullface.endsWith("commercial.png")) {
            continue;
        }
        // 商业版不加载社区默认头像
        if ((!isCommunity() && !isOpenkylin()) &&fullface.endsWith("community.png")) {
            continue;
        }
        if (fullface.endsWith("default.png")) {
            continue;
        }

        if (!fullface.contains("-")) {
            continue;
        }
        return fullface;
    }
    return "/usr/share/ukui/faces/default.png";
}

void startFcitx()
{
    QString fcitxCommand = "fcitx5 -d &";
    runCommand(fcitxCommand);

    QString vkdcommand = "/usr/bin/kylin-virtual-keyboard-xwayland &";
    runCommand(vkdcommand);

    QString sogouEngineCommand = "/opt/apps/com.cpis/bin/cpis-engine-service --ini /opt/apps/com.cpis/etc/ise.ini --type dbus &";
    runCommand(sogouEngineCommand);

    QString sogouPanelCommand = "/opt/apps/com.cpis/bin/cpis-panel-service --ini /opt/apps/com.cpis/etc/isp.ini --type dbus &";
    runCommand(sogouPanelCommand);
}

void runCommand(const QString &command) {
    QProcess *process = new QProcess();
    process->start(command);
    QObject::connect(process, &QProcess::readyReadStandardOutput, [process]() {
        qDebug() << "Standard Output:" << process->readAllStandardOutput();
    });

    QObject::connect(process, &QProcess::readyReadStandardError, [process]() {
        qDebug() << "Standard Error:" << process->readAllStandardError();
    });
}
