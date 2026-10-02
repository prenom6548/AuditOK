/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
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
#include "common.h"
#include "config/kmreenv.h"

#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QDirIterator>
#include <QProcessEnvironment>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCameraInfo>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QSettings>
#include <QFontMetrics>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <sys/syslog.h>

#define DISPLAY_TYPE_FILE "kylin-kmre-window.type"
static const char *WindowLockFile = "kylin-kmre-window.lock";

namespace kmre {
namespace utils {

static int lockFd = -1;

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

bool checkAndCreateFile(const QString& filePath, int permission)
{
    if (access(filePath.toStdString().c_str(), F_OK) == -1) {
        // create empty file
        umask(0000);
        int fileFd = open(filePath.toStdString().c_str(), O_WRONLY | O_CLOEXEC | O_CREAT, permission);
        if (fileFd < 0) {
            syslog(LOG_ERR, "Failed to create file: '%s'", filePath.toStdString().c_str());
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
        syslog(LOG_ERR, "Failed to open file(%s)", filePath.toStdString().c_str());
        return data;
    }

    if (lock) {
        int timeout = 0;
        while (flock(fileFd, LOCK_EX | LOCK_NB) != 0) {
            if (errno == EWOULDBLOCK) {
                if (timeout >= msecond) {
                    syslog(LOG_ERR, "Lock file '%s' timeout!", filePath.toStdString().c_str());
                    close(fileFd);
                    return data;
                }

                usleep(1000);
                timeout++;
                continue;
            } 
            else {
                syslog(LOG_ERR, "Failed to lock file '%s' ! errno = '%d'", filePath.toStdString().c_str(), errno);
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
        syslog(LOG_ERR, "Failed to open or create file(%s)", filePath.toStdString().c_str());
        return false;
    }

    if (lock) {
        int timeout = 0;
        while (flock(fileFd, LOCK_EX | LOCK_NB) != 0) {
            if (errno == EWOULDBLOCK) {
                if (timeout >= msecond) {
                    syslog(LOG_ERR, "Lock file '%s' timeout!", filePath.toStdString().c_str());
                    close(fileFd);
                    return false;
                }

                usleep(1000);
                timeout++;
                continue;
            } 
            else {
                syslog(LOG_ERR, "Failed to lock file '%s' ! errno = '%d'", filePath.toStdString().c_str(), errno);
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
            syslog(LOG_ERR, "Failed to write data to file: '%s'!", filePath.toStdString().c_str());
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

int checkLockFile()
{
    int ret;
    QString homeDir;
    QString lockFileDir;
    QString lockFilePath;

    homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    lockFileDir = homeDir + "/.kmre";
    lockFilePath = lockFileDir + "/kylin-kmre-window.lock";

    QDir pending(lockFileDir);
    if (!pending.exists()) {
        // mkdir
        pending.mkpath(lockFileDir);
        // chmod 0755
        QFile::setPermissions(lockFileDir,
                              QFile::ReadUser  | QFile::WriteUser  | QFile::ExeUser  |
                              QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                              QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
    }

    umask(0000);
    lockFd = open(lockFilePath.toStdString().c_str(), O_RDWR | O_CLOEXEC | O_CREAT, 0666);
    if (lockFd < 0) {
        fprintf(stderr, "Failed to open lock file\n");
        syslog(LOG_ERR, "utils: Failed to open lock file.");
        return -1;
    }

    ret = flock(lockFd, LOCK_EX | LOCK_NB);
    if (ret < 0) {
        if (errno == EWOULDBLOCK) {
            fprintf(stderr, "File has been locked for another process.\n");
            syslog(LOG_ERR, "utils: Lock file has been locked for another process.");
        } else {
            fprintf(stderr, "Failed to test lock file.\n");
            syslog(LOG_ERR, "utils: Failed to test lock file.");
        }
        return -1;
    }

    ret = flock(lockFd, LOCK_EX);
    if (ret < 0) {
        fprintf(stderr, "Failed to lock file.\n");
        syslog(LOG_ERR, "utils: Failed to lock file.");
        return -1;
    }

    return 0;
}

void releaseLockFile()
{
    if (lockFd >= 0) {
        flock(lockFd, LOCK_UN);
        close(lockFd);
        lockFd = -1;
    }
}

bool tryLockFile(const QString& lockFilePath, int msecond)
{
    umask(0000);
    lockFd = open(lockFilePath.toStdString().c_str(), O_RDWR | O_CLOEXEC | O_CREAT, 0666);
    if (lockFd < 0) {
        syslog(LOG_ERR, "Failed to open lock file(%s)", lockFilePath.toStdString().c_str());
        return false;
    }

    if (flock(lockFd, LOCK_EX | LOCK_NB) != 0) {
        if (errno == EWOULDBLOCK) {
            syslog(LOG_ERR, " The lock file has been already locked by another process. Try again later.");
            usleep(msecond * 1000);
            if (flock(lockFd, LOCK_EX | LOCK_NB) == 0) {
                return true;
            }
        } 
        else {
            syslog(LOG_ERR, "Failed to lock file! errno = '%d'", errno);
        }
        return false;
    }

    return true;
}

bool tryLockKylinKmreWindow()
{
    return tryLockFile(QDir::temp().absoluteFilePath(WindowLockFile), 100);
}

void unLockKylinKmreWindow()
{
    syslog(LOG_DEBUG, "[%s] Remove lock file: '%s'", __func__, WindowLockFile);
    remove(QDir::temp().absoluteFilePath(WindowLockFile).toStdString().c_str());
}

QString readFileContent(const QString &path)
{
    QFile file(path);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Read file content failed to open" << path;
            return "";
        }

        //QTextStream ts(&file);
        //QString str = ts.readAll();
        const QString &str = QString::fromUtf8(file.readAll());
        file.close();

        return str;
    }

    return "";
}

bool isFileSuffixSupported(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix();

    QStringList errorList;
    errorList << "gif";

    if (errorList.indexOf(suffix.toUpper()) != -1) {
        return false;
    }

    if (suffix == "png" || suffix == "jpg") {
        return true;
    }

    return false;
}

const QFileInfoList getAllFilesInfo(const QString &dir, bool recursive)
{
    QFileInfoList infos;

    if (!recursive) {
        auto finfos = QDir(dir).entryInfoList(QDir::Files);//auto finfos = QDir(dir).entryInfoList(QStringList() << "*.png", QDir::Files);
        for (QFileInfo finfo : finfos) {
            if (isFileSuffixSupported(finfo.absoluteFilePath())) {
                infos << finfo;
            }
        }
        return infos;
    }

    QDirIterator dirIterator(dir, QDir::Files, QDirIterator::Subdirectories);
    while(dirIterator.hasNext()) {
        dirIterator.next();
        if (isFileSuffixSupported(dirIterator.fileInfo().absoluteFilePath())) {
            infos << dirIterator.fileInfo();
        }
    }
}

int isInCpuinfo(const char *fmt, const char *str)
{
    FILE *cpuinfo;
    char field[256];
    char format[256];
    int found = 0;

    sprintf(format, "%s : %s", fmt, "%255s");

    cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo) {
        do {
            if (fscanf(cpuinfo, format, field) == 1) {
                if (strncmp(field, str, strlen(str)) == 0)
                    found = 1;
                break;
            }
        } while (fgets(field, 256, cpuinfo));
        fclose(cpuinfo);
    }

    return found;
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

    containerIconPath = KmreEnv::GetAndroidPath() + "/data/local/icons/" + packageName + ".png";
    QFile containerIconFile(containerIconPath);
    if (containerIconFile.exists()) {
        return containerIconPath;
    }

    return QString("");
}

bool isFileExist(QString fullFilePath)
{
    QFileInfo fileInfo(fullFilePath);
    if (fileInfo.isFile()) {
        return true;
    }
    return false;
}

int currentDisplayOrientation(int initialOrientation, int imageRotation)
{
    if (initialOrientation == Qt::PortraitOrientation) {//纵向
        if (imageRotation == 0 || imageRotation == 2) {
            return Qt::PortraitOrientation;//纵向
        } else {
            return Qt::LandscapeOrientation;//横向
        }
    } else {
        if (imageRotation == 0 || imageRotation == 2) {
            return Qt::LandscapeOrientation;//横向
        } else {
            return Qt::PortraitOrientation;//纵向
        }
    }
}

bool readX11WindowFullScreenFromConifg(const QString &pkgName)
{
    bool fullscreen = false;

    if (pkgName.isEmpty()) {
        return false;
    }

    QFile file("/usr/share/kmre/app.json");
    if (!file.exists()) {
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "failed to open /usr/share/kmre/app.json";
        return false;
    }

    QString content = file.readAll();
    file.close();

    QJsonParseError json_error;
    const QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &json_error);
    if (!jsonDocument.isNull() && (json_error.error == QJsonParseError::NoError)) {
        QJsonObject jsonObject = jsonDocument.object();
        if (jsonObject.isEmpty() || jsonObject.size() == 0) {
            return false;
        }

        if (jsonObject.contains("items")) {
            QJsonArray array = jsonObject["items"].toArray();
            for (const auto res : array) {
                const auto object = res.toObject();
                if (object.isEmpty() || object.size()  == 0) {
                    continue;
                }
                if (object.contains("pkgName")) {
                    QString name = object["pkgName"].toString();
                    if (name == pkgName) {
                        if (object.contains("x11WindowFullscreen")) {
                            fullscreen = object["x11WindowFullscreen"].toBool();
                        }
                        break;
                    }
                }
            }
        }
    }

    return fullscreen;
}

//从/usr/share/kmre/app.json文件中读取应用是否使用统一分辨率的尺寸进行显示
bool readAppNonuniformFromConifg(const QString &pkgName)
{
    bool nonuniform = false;

    QFile file("/usr/share/kmre/app.json");
    if (!file.exists()) {
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "failed to open /usr/share/kmre/app.json";
        return false;
    }

    QString content = file.readAll();
    file.close();
    QJsonParseError json_error;
    const QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &json_error);
    if (!jsonDocument.isNull() && (json_error.error == QJsonParseError::NoError)) {
        QJsonObject jsonObject = jsonDocument.object();
        if (jsonObject.isEmpty() || jsonObject.size() == 0) {
            return false;
        }

        if (jsonObject.contains("items")) {
            QJsonArray array = jsonObject["items"].toArray();
            for (const auto res : array) {
                const auto object = res.toObject();
                if (object.isEmpty() || object.size()  == 0) {
                    continue;
                }
                if (object.contains("pkgName")) {
                    QString name = object["pkgName"].toString();
                    if (name == pkgName) {
                        if (object.contains("nonuniform")) {
                            nonuniform = object["nonuniform"].toBool();
                        }
                        break;
                    }
                }
            }
        }
    }

    return nonuniform;
}

int isZoomSupported(const QString &packageName)
{
    QStringList zoom1PkgList;
    zoom1PkgList << "com.tencent.mm" << "com.sina.weibo"  << "com.baidu.tieba"
                << "com.sina.oasis" << "com.zhihu.android"  << "com.founder.apabi.reader"
                << "com.sina.news" << "com.tencent.androidqqmail"  << "com.netease.mobimail";
    QStringList zoom2PkgList;
    zoom2PkgList << "com.baidu.netdisk" << "com.tencent.mobileqq"  << "com.tencent.qqlite"
                << "com.douban.frodo" << "com.alicloud.databox"  << "com.baidu.searchbox"
                << "com.tencent.mtt" << "cn.xuexi.android"  << "com.cubic.autohome"
                << "com.ss.android.article.news" << "com.ss.android.auto";
    QStringList zoom3PkgList;
    zoom3PkgList << "cn.wps.moffice_eng";

    if (zoom1PkgList.indexOf(packageName) != -1) {
        return 1;
    }
    if (zoom2PkgList.indexOf(packageName) != -1) {
        return 2;
    }
    if (zoom3PkgList.indexOf(packageName) != -1) {
        return 3;
    }

    return 0;
}

void getWindowScaleAndCenterPosition(int scaleWidth, int scaleHeight, int width, int height, int &x, int &y, float &scale)
{
    double windowAspect, scaleAspect;
    int newX, newY;

    windowAspect = double(width) / double(height);
    scaleAspect = double(scaleWidth) / double(scaleHeight);

    if (windowAspect > scaleAspect) {
        scale = float(height) / float(scaleHeight);
        newX = int(double(height) * scaleAspect);
        newY = height;
    }
    else {
        scale = float(width) / float(scaleWidth);
        newX = width;
        newY = int(float(width) / scaleAspect);
    }

    x = width / 2 - newX / 2;
    y = height / 2 - newY / 2;
}

QString getFixedDefaultCameraDevice(const QString &deviceName)
{
    QString defaultCameraDevice = "";

    if (deviceName.isNull() || deviceName.isEmpty()) {
        foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
            defaultCameraDevice = cameraInfo.deviceName();
            break;
        }
    }
    else {
        bool exits = false;
        int i = 0;
        QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
        for (QList<QCameraInfo>::Iterator it = cameras.begin(); it != cameras.end(); ++it) {
            QString name = it->deviceName();
            if (i == 0) {
                defaultCameraDevice = name;
            }

            if (name == deviceName) {
                exits = true;
            }

            i++;
        }

        if (exits) {//deviceName在摄像头列表中存在时
            defaultCameraDevice = deviceName;
        }
    }

    return defaultCameraDevice;
}

QString getDefaultScreenshotFolder()
{
    QString screenshotDirectory;
#if QT_VERSION >= 0x040400
#if QT_VERSION >= 0x050000
    QString pdir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (pdir.isEmpty()) pdir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (pdir.isEmpty()) pdir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#else
    QString pdir = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
    if (pdir.isEmpty()) pdir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    if (pdir.isEmpty()) pdir = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#endif
    if (pdir.isEmpty()) pdir = "/tmp";
    if (!QFile::exists(pdir)) {
        pdir = "/tmp";
    }
    QString default_screenshot_path = pdir;
    if (!QFile::exists(default_screenshot_path)) {
        if (!QDir().mkdir(default_screenshot_path)) {
        }
    }
    if (QFile::exists(default_screenshot_path)) {
        screenshotDirectory = default_screenshot_path;
    }
#endif

    if (screenshotDirectory.isEmpty()) {
        screenshotDirectory = QDir::toNativeSeparators(screenshotDirectory);
        if (!QFile::exists(screenshotDirectory)) {
            if (!QDir().mkdir(screenshotDirectory)) {
            }
        }
    }

    return screenshotDirectory;
}

QString getDebPkgVersion(const QString &debPkgName)
{
    QByteArray data;
    QProcess process;
    QString version;
    QStringList options;
    options << "-W" << "-f=${Version}\n" << debPkgName;
    process.start("dpkg-query", options);
    process.waitForFinished();
    process.waitForReadyRead();
    data = process.readAllStandardOutput();
    version = QString(data);
    version.replace(QString("\n"), QString(""));

    return version;
}

void clearApkFiles()
{
    QString targetDir = QString("%1/data/local/tmp/").arg(KmreEnv::GetAndroidPath());
    if (QDir(targetDir).exists()) {
        QDirIterator::IteratorFlag iter_flag;
        iter_flag = QDirIterator::NoIteratorFlags;//iter_flag = QDirIterator::Subdirectories;
        QDirIterator iter(targetDir, QDir::Files | QDir::NoDotAndDotDot, iter_flag);
        QFileInfo srcInfo;
        QString destFilepath;
        while (iter.hasNext()) {
            srcInfo = iter.next();
            destFilepath = iter.filePath();
            if (srcInfo.isFile() && srcInfo.suffix().toLower() == "apk") {
                if (QFile::exists(destFilepath)) {
                    QFile::remove(destFilepath);
                }
            }
        }
    }
}

QStringList convertUrlsToUris(const QList<QUrl> &urls)
{
    QStringList uriList;
    uriList.reserve(urls.size());

    for (const QUrl &url : urls) {
        uriList << url.toString();
    }

    return uriList;
}

void openFolder(const QString &path)
{
    if (path.isEmpty() || path.isNull()) {
        return;
    }
    QDir dir(path);    
    if (dir.exists()) {
        QDBusInterface interface("org.freedesktop.FileManager1", "/org/freedesktop/FileManager1", "org.freedesktop.FileManager1", QDBusConnection::sessionBus());
        interface.call("ShowFolders", convertUrlsToUris(QList<QUrl>() << QUrl::fromLocalFile(path)), QString());
    }
}

static char* shmOfDisplayType = nullptr;
static const int shmSizeOfDisplayType = 32;

void initShmOfDisplayType()
{
    if (!shmOfDisplayType) {
        shmOfDisplayType = (char *)mmap(NULL, shmSizeOfDisplayType, PROT_READ | PROT_WRITE,  
                                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);  
        if (shmOfDisplayType) {
            memset(shmOfDisplayType, 0, shmSizeOfDisplayType);
        }
        else {
            syslog(LOG_ERR, "[%s]Mmap shm of display type failed!", __func__);
        }
    }
}

void destroyShmOfDisplayType()
{
    munmap(shmOfDisplayType, shmSizeOfDisplayType);
    shmOfDisplayType = nullptr;
}

void saveDisplayTypeToShm(const QString& displayType)
{
    if (shmOfDisplayType) {
        snprintf(shmOfDisplayType, shmSizeOfDisplayType, "%s", displayType.toStdString().c_str());
    }
    else {
        syslog(LOG_ERR, "[%s]Get shm of display type failed!", __func__);
    }
}

QString getDisplayTypeFromShm()
{
    QString displayType;

    if (shmOfDisplayType) {
        displayType = QString(shmOfDisplayType);
    }
    else {
        syslog(LOG_ERR, "[%s]Get shm of display type failed!", __func__);
    }

    return std::move(displayType);
}

bool getAppNameFromDesktop(const QString &pkgName, QString &appName_en, QString &appName_zh)
{
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString desktopFilePath = homeDir + "/.local/share/applications/" + pkgName + ".desktop";

    QSettings settings(desktopFilePath, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    appName_en = settings.value("Desktop Entry/Name", "").toString();
    appName_zh = settings.value("Desktop Entry/Name[zh_CN]", "").toString();

    return true;
}

void showUserGuide()
{
    QString serviceName = "com.kylinUserGuide.hotel_" + QString::number(getuid());

    QDBusInterface userGuideDbus(serviceName, "/", "com.guide.hotel", QDBusConnection::sessionBus());
    if (userGuideDbus.isValid()) {
        userGuideDbus.call("showGuide", "kmre");
    }
    else {
        syslog(LOG_ERR, "[%s]Can't find user guide service!", __func__);
    }
}

QString getRealPkgName(const QString &pkgName)
{
    QStringList nameList = pkgName.split("+", QString::SkipEmptyParts);
    if (nameList.size() >= 1) {
        return nameList.at(0);
    }
    return pkgName;
}

QString getCpuArch() 
{
    struct utsname buf;

    if (uname(&buf) != 0) {
        syslog(LOG_ERR, "[%s]Can't get CPU Architecture!", __func__);
        return "";
    }

    syslog(LOG_DEBUG, "[%s]CPU Architecture: '%s'", __func__, buf.machine);
    return QString(buf.machine);
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
