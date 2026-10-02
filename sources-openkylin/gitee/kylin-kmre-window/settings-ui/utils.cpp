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

#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <dlfcn.h>
#include <sys/syslog.h>

#include <QProcessEnvironment>
#include <QFile>
#include <QDebug>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QProcess>
#include <QFont>

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>


namespace kmre {
namespace utils {

void centerToScreen(QWidget* widget)
{
    if (!widget) {
      return;
    }
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = widget->width();
    int y = widget->height();
    widget->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
}

const QString& getUserName()
{
    static QString userName = "";

    if (!userName.isEmpty()) {
        return userName;
    }

    struct passwd  pwd;
    struct passwd* result = 0;
    char buf[1024];

    memset(&buf, 0, sizeof(buf));
    uint32_t uid = getuid();
    (void)getpwuid_r(uid, &pwd, buf, 1024, &result);

    if (result && pwd.pw_name) {
        userName = pwd.pw_name;
    }
    else {
        try {
            userName = std::getenv("USER");
        } 
        catch (...) {
            try {
                userName = std::getenv("USERNAME");
            }
            catch (...) {
                syslog(LOG_ERR, "[%s] Get user name failed!", __func__);
                char name[16] = {0};
                snprintf(name, sizeof(name), "%u", getuid());
                userName = name;
            }
        }
    }

    userName.replace('\\', "_");// 在某些云环境，用户名中会有'\'字符，需将该字符转换为'_'字符
    return userName;
}

const QString& getUid()
{
    int uid = -1;
    static QString userId = "";

    if (!userId.isEmpty()) {
        return userId;
    }

    try {
        uid = getuid();
    }
    catch (...) {
        syslog(LOG_ERR, "[%s] Get user id failed!", __func__);
    }
    
    userId = QString::number(uid);
    return userId;
}

QString readFileContent(const QString &fileName)
{
    QString content = QString("");

    QFile file(fileName);
    if (!file.exists()) {
        return content;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "failed to open " << fileName;
        return content;
    }

    content = QString(file.readAll());
    file.close();

    return content;
}

QString getIconPath(const QString &packageName)
{
    QString homeDir;
    QString homeIconPath;
    QString containerName;
    QString containerIconPath;

    //svg comes from the package named ubuntukylin-theme-android
    /*QString svgPath = QString("/usr/share/icons/ukui-android/scalable/apps/%1.svg").arg(packageName);
    QFile fp(svgPath);
    if (fp.exists()) {
        const QFileInfo info(svgPath);
        if (info.isFile()) {
            return svgPath;
        }
    }*/

    homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    homeIconPath = homeDir + "/.local/share/icons/" + packageName + ".svg";

    QFile homeIconFile(homeIconPath);
    if (homeIconFile.exists()) {
        return homeIconPath;
    }
    else {
        homeIconPath.clear();
        homeIconPath = homeDir + "/.local/share/icons/" + packageName + ".png";
        QFile fp(homeIconPath);
        if (fp.exists()) {
            return homeIconPath;
        }
    }

    containerName = makeContainerName();
    containerIconPath = "/var/lib/kmre/" + containerName + "/data/local/icons/" + packageName + ".png";
    QFile containerIconFile(containerIconPath);
    if (containerIconFile.exists()) {
        return containerIconPath;
    }

    return QString();
}

QString makeContainerName()
{
    QString userName;

    userName = getUserName();

    return makeContainerName(userName, getuid());
}

QString makeContainerName(const QString& userName, int uid)
{
    QString containerName;

    if (userName.isNull() || userName.isEmpty() || userName.length() == 0) {
        return containerName;
    }

    if (uid < 0) {
        return containerName;
    }

    containerName = QString("kmre-%1-%2").arg(QString::number(uid)).arg(userName);

    return containerName;
}

QByteArray getInstalledAppListJsonStr()
{
    QString libPath = "/usr/lib/libkmre.so";
    if (!QFile::exists(libPath)) {
        return QString("[]").toUtf8();
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        return QString("[]").toUtf8();
    }

    char *(*get_installed_applist)();
    get_installed_applist = (char *(*)())dlsym(module_handle, "get_installed_applist");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        return QString("[]").toUtf8();
    }

    char *list_info = NULL;
    list_info = get_installed_applist();
    if (list_info) {
        std::string runningInfo = std::string(list_info);
        QByteArray byteArray(list_info, runningInfo.length());
        dlclose(module_handle);
        return byteArray;
    }

    dlclose(module_handle);
    return QString("[]").toUtf8();
}

std::vector<std::pair<QString, QString>> getInstalledAppList()
{
    std::vector<std::pair<QString, QString>> appList;
    QJsonParseError err;

    QString locale = QLocale::system().name();
    QByteArray array = getInstalledAppListJsonStr();
    QJsonDocument document = QJsonDocument::fromJson(array, &err);
    if (err.error == QJsonParseError::NoError && !document.isNull() && !document.isEmpty() && document.isArray()) {
        QJsonArray jsonArray = document.array();
        //qDebug() << "Get installed app list:";
        foreach (QJsonValue val, jsonArray) {
            QJsonObject subObject = val.toObject();

            QString pkgName = subObject.value("package_name").toString().trimmed();
            QString appName = subObject.value("app_name").toString().trimmed();
            //qDebug() << "package_name: " << pkgName << ", app_name: " << appName;
            if (!pkgName.isEmpty()) {
                appList.emplace_back(std::make_pair(pkgName, appName));
            }
        }
    }

    return std::move(appList);
}

QString execCmd(const QString &cmd, int msec)
{
    QByteArray data;
    QProcess process;
    QStringList options;

    options << "-c" << cmd;
    process.start("/bin/bash", options);
    process.waitForFinished(msec);
    process.waitForReadyRead(msec);
    data = process.readAllStandardOutput();

    return std::move(QString(data));
}

bool checkAndCreateFile(const QString& filePath, int permission)
{
    if (access(filePath.toStdString().c_str(), F_OK) == -1) {
        // create empty file
        umask(0000);
        int fileFd = open(filePath.toStdString().c_str(), O_WRONLY | O_CLOEXEC | O_CREAT, permission);
        if (fileFd < 0) {
            qWarning() << "Failed to create file: " << filePath;
        }
        else {
            close(fileFd);
        }
        return false;
    }

    return true;
}

QByteArray tryReadFile(const QString& filePath, bool lock, int msecond) 
{
    QByteArray data;
    int fileFd = -1;

    fileFd = open(filePath.toStdString().c_str(), O_RDONLY | O_CLOEXEC);
    if (fileFd < 0) {
        qWarning() << "Failed to open file: " << filePath;
        return data;
    }

    if (lock) {
        int timeout = 0;
        while (flock(fileFd, LOCK_EX | LOCK_NB) != 0) {
            if (errno == EWOULDBLOCK) {
                if (timeout >= msecond) {
                    qWarning() << "Lock file: " << filePath << " timeout!";
                    close(fileFd);
                    return data;
                }

                usleep(1000);
                timeout++;
                continue;
            } 
            else {
                qWarning() << "Failed to lock file: " << filePath;
                close(fileFd);
                return data;
            }
        }
    }

    int readByte = 0;
    char buf[1024] = {0};
    while ((readByte = read(fileFd, buf, sizeof(buf))) > 0) {
        data.append(buf, readByte);
    }

    if (lock) {
        flock(fileFd, LOCK_UN);
    }

    close(fileFd);
    return data;
}

bool tryWriteFile(const QString& filePath, const QByteArray& data, bool lock, int msecond) 
{
    int fileFd = -1;

    fileFd = open(filePath.toStdString().c_str(), O_WRONLY | O_TRUNC | O_CREAT | O_CLOEXEC, 0666);
    if (fileFd < 0) {
        qWarning() << "Failed to open or create file: " << filePath;
        return false;
    }

    if (lock) {
        int timeout = 0;
        while (flock(fileFd, LOCK_EX | LOCK_NB) != 0) {
            if (errno == EWOULDBLOCK) {
                if (timeout >= msecond) {
                    qWarning() << "Lock file: " << filePath << " timeout!";
                    close(fileFd);
                    return false;
                }

                usleep(1000);
                timeout++;
                continue;
            } 
            else {
                qWarning() << "Failed to lock file: " << filePath;
                close(fileFd);
                return false;
            }
        }
    }

    int writeCount = data.size();
    char *buf = (char*)(data.data());
    while (writeCount > 0) {
        int writeByte = write(fileFd, buf, writeCount);
        if (writeByte <= 0) {
            qWarning() << "Failed to write data to file: " << filePath;
            break;
        }
        buf += writeByte;
        writeCount -= writeByte;
    }

    if (lock) {
        flock(fileFd, LOCK_UN);
    }

    close(fileFd);
    return true;
}

std::tuple<bool, QString> getElideText(const QString& origString, const uint32_t maxLength, Qt::TextElideMode mode)
{
    bool elided = false;
    QFont ft;
    QFontMetrics fm(ft);
    QString elideText = fm.elidedText(origString, mode, maxLength);
    if (fm.width(origString) > maxLength) {
        elided = true;
    }

    //qDebug() << "origString = " << origString << ", elideText = " << elideText << ", elided = " << elided;
    return {elided, elideText};
}

}
}
