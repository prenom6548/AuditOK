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
#include <string.h>
#include <ctype.h>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDebug>

#define FACEPATH "/usr/share/ukui/faces/"
#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif
#define ASSERT_NOT_NULL(x, ret)                                                                                        \
    if (!x) {                                                                                                          \
        return ret;                                                                                                    \
    }

/**
 * @brief 对给定的字符串进行strip操作，删减字符串前后的指定字符；注意该操作会修改原字符串
 *
 * @param str 需要进行strip的字符串指针
 * @param ch 需要删除的字符
 */
static void strstrip(char *str, char ch)
{
    if (strnlen(str, STRNLEN_MAX) == 0)
        return;
    char *startPos = str;
    while (*startPos != '\0' && *startPos == ch)
        startPos++;
    if (*startPos == '\0') {
        str[0] = 0;
        return;
    }

    char *endPos = str + strnlen(str, STRNLEN_MAX) - 1;
    while (endPos != str && *endPos == ch)
        endPos--;

    memmove(str, startPos, endPos - startPos + 1);
    *(str + (endPos - startPos) + 1) = 0;
}

/**
 * @brief 删除给定字符串前后的空格、制表符、换行符，注意该操作会修改原字符串
 *
 * @param str 需要进行strip操作的字符串指针
 */
static void strstripspace(char *str)
{
    if (strnlen(str, STRNLEN_MAX) == 0)
        return;
    char *startPos = str;
    while (*startPos != '\0' && isspace(*startPos))
        startPos++;
    if (*startPos == '\0') {
        str[0] = 0;
        return;
    }

    char *endPos = str + strnlen(str, STRNLEN_MAX) - 1;
    while (endPos != str && isspace(*endPos))
        endPos--;

    memmove(str, startPos, endPos - startPos + 1);
    *(str + (endPos - startPos) + 1) = 0;
}

static char *get_val_from_file(FILE *fp, const char *key)
{
    if (!fp)
        return NULL;
    char *val = NULL;
    char buf[1024] = { 0 };
    while (fgets(buf, 1024, fp)) {
        if (strncmp(buf, key, strnlen(key, STRNLEN_MAX)) == 0) {
            val = strdup((char *)buf + strnlen(key, STRNLEN_MAX) + 1);
            break;
        }
    }

    return val;
}

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

static char *SystemGetHostCloudPlatform()
{
    char *cloudplat = (char *)malloc(sizeof(char) * 65);
    ASSERT_NOT_NULL(cloudplat, NULL);
#ifdef __linux__
    cloudplat[0] = 0;
    char buf[256] = { 0 };
    bool res_flag = false;
    char *type = NULL;

    if (!access("/usr/local/ctyun/clink/Mirror/Registry/Default", F_OK)) {
        snprintf(cloudplat, 65, "ctyun");
        return cloudplat;
    }

    FILE *fp = fopen("/etc/hw-cloud.conf", "rt");
    if (fp) {
        type = get_val_from_file(fp, "platform");
        fclose(fp);
    }
    if (type) {
        strstripspace(type);
        free(cloudplat);
        return type;
    }

    if (geteuid() == 0) // root 用户，可以用dmidecode
    {
        FILE *pipeLine = popen("dmidecode -s chassis-manufacturer", "r");
        if (__glibc_likely(pipeLine != NULL)) {
            fgets(buf, 255 * sizeof(char), pipeLine);
            strstripspace(buf);
            if (strcmp(buf, "Huawei Inc.") == 0) // 华为云
            {
                snprintf(cloudplat, 65, "huawei");
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
                    snprintf(cloudplat, 65, "huawei");
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
                snprintf(cloudplat, 65, "huawei");
                res_flag = true;
            }
            fclose(fp);
        }
        if (!res_flag) {
            fp = fopen("/sys/devices/virtual/dmi/id/chassis_asset_tag", "r");
            if (__glibc_likely(fp != NULL)) {
                fgets(buf, 255 * sizeof(char), fp);
                strstripspace(buf);
                if (strcmp(buf, "HUAWEICLOUD") == 0) // 华为云
                {
                    snprintf(cloudplat, 65, "huawei");
                }
                fclose(fp);
            }
        }
    }

    if (strnlen(cloudplat, STRNLEN_MAX) == 0) {
        snprintf(cloudplat, 65, "none");
    }

#endif
    return cloudplat;
}

QString getHostCloudPlatform()
{
    static QString strPlatform = "";
    if (strPlatform.isEmpty()) {
        char *platForm = SystemGetHostCloudPlatform();
        if (platForm) {
            strPlatform = platForm;
            free(platForm);
            platForm = NULL;
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

static char *SystemGetSystemName()
{
    char *sysname = NULL;
#ifdef __linux__
    FILE *fp = fopen("/etc/os-release", "r");
    if (!fp)
        return NULL;

    sysname = get_val_from_file(fp, "NAME");
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
    char *systemNamePtr = SystemGetSystemName();
    QString systemName;
    // 使用完后释放内存
    if (systemNamePtr) {
        systemName = QString::fromLatin1(systemNamePtr);
        free(systemNamePtr);
    }
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
        if ((!isCommunity() && !isOpenkylin()) && fullface.endsWith("community.png")) {
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

    QString sogouEngineCommand
        = "/opt/apps/com.cpis/bin/cpis-engine-service --ini /opt/apps/com.cpis/etc/ise.ini --type dbus &";
    runCommand(sogouEngineCommand);

    QString sogouPanelCommand
        = "/opt/apps/com.cpis/bin/cpis-panel-service --ini /opt/apps/com.cpis/etc/isp.ini --type dbus &";
    runCommand(sogouPanelCommand);
}

void runCommand(const QString &command)
{
    QProcess *process = new QProcess();
    process->start(command);
    QObject::connect(process, &QProcess::readyReadStandardOutput, [process]() {
        qDebug() << "Standard Output:" << process->readAllStandardOutput();
    });

    QObject::connect(process, &QProcess::readyReadStandardError, [process]() {
        qDebug() << "Standard Error:" << process->readAllStandardError();
    });
}

bool isProcessRunning(const QString &procName)
{
    QDir dir("/proc");
    QStringList pids = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QString &pid, pids) {
        QFile cmdline("/proc/" + pid + "/cmdline");
        if (cmdline.open(QIODevice::ReadOnly)) {
            if (cmdline.readAll().contains(procName.toLatin1())) {
                return true;
            }
        }
    }
    return false;
}
