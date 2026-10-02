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
#include "dbus_client.h"
#include "dialog.h"
#include "get_userinfo.h"
#include "displayinfo.h"

#include <QDebug>
#include <QFileInfo>
#include <QDesktopWidget>
#include <QScreen>
#include <QApplication>
#include <QCommandLineParser>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRegExp>
#include <QProcessEnvironment>
#include <QSettings>
#include <QDir>
#include <QDomDocument>

#include <iostream>
#include <set>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/syslog.h>

#define PROC_MODULES_PATH "/proc/modules"
#define BUF_SIZE 1024

using namespace std;

const QString& Utils::getUserName()
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

const QString& Utils::getUid()
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

QString Utils::readFile(const QString &path)
{
    QFile file(path);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return "";
        }
        QTextStream stream(&file);
        QString str = stream.readAll();
        file.close();
        return str;
    }
    else {
        return "";
    }
}

int Utils::checkSingle(){
    QString homeDirPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString lockDirPath = homeDirPath + "/.kmre";
    QString lockFilePath = lockDirPath + "/startapp-lock";
    //qDebug()<<"lock file path:"<<lockFilePath;
    std::string str = lockFilePath.toStdString();
    const char* ch = str.c_str();
    int fd = open(ch, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        //qDebug()<<"fail to open .kmre/startapp-lock in personal directory";
        syslog(LOG_ERR, "fail to open .kmre/startapp-lock in personal directory");
        exit(1);
    }

    if (lockf(fd, F_TLOCK, 0)) {
        //qDebug()<<"fail to launch startapp, an instance is aready running";
        syslog(LOG_ERR, "fail to launch startapp, an instance is aready running");
        exit(0);
    }

    return fd;
}

bool Utils::getOptionsFromArgument(bool &fullScreen, int &density, int &width, int &height)
{
    const QStringList& args = qApp->arguments();
    const QCommandLineOption fullscreenOption(QStringList() << "f" << "fullscreen", "window display in fullscreen.");
    //const QCommandLineOption x11fullscreenOption(QStringList() << "F" << "x11 window fullscreen", "X11 window display in fullscreen.");
    //const QCommandLineOption densityOption("d", "fullscreen density", "density", "");
    const QCommandLineOption widthOption("W", "window width", "width", "");
    const QCommandLineOption heightOption("H", "window height", "height", "");

    QCommandLineParser parser;
    //parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);//QCommandLineParser::ParseAsCompactedShortOptions
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(fullscreenOption);
    //parser.addOption(x11fullscreenOption);
    //parser.addOption(densityOption);
    parser.addOption(widthOption);
    parser.addOption(heightOption);

    if (!parser.parse(args)) {
        qCritical() << "Failed to parse argument" << args;
        return false;
    }

//    if (parser.isSet(densityOption)) {
//        density = parser.value(densityOption).toInt();
//    }

    if (parser.isSet(widthOption)) {
        width = parser.value(widthOption).toInt();
    }

    if (parser.isSet(heightOption)) {
        height = parser.value(heightOption).toInt();
    }

    fullScreen = parser.isSet(fullscreenOption);

    return true;
}

void Utils::prepareEnvironment()
{
    struct str_info my_info;
    my_info = UserInfo::getuserinfo();
    kmre::DBusClient::getInstance()->Prepare(my_info.name_info, my_info.uid_info);
}

void Utils::startContainer()
{
    //QString displayInfo;

    struct str_info my_info;
    my_info = UserInfo::getuserinfo();

//    QDesktopWidget* desktopWidget = QApplication::desktop();
//    int g_nActScreenW = desktopWidget->width();
//    int g_nActScreenH = desktopWidget->height();
    QScreen *main_screen = QApplication::primaryScreen();
    int g_nActScreenW = 1280, g_nActScreenH = 720;
    if (main_screen){
        g_nActScreenW = main_screen->geometry().width();
        g_nActScreenH = main_screen->geometry().height();
    }
//    qDebug()<<"screen_width: "<<g_nActScreenW<<"   screen_height: "<<g_nActScreenH;
//    qDebug()<<"user_name: "<<my_info.name_info<<"   user_id: "<<my_info.uid_info;

    //displayInfo = kmre::DBusClient::getInstance()->GetDisplayInformation();

    kmre::DBusClient::getInstance()->StartContainer(my_info.name_info, my_info.uid_info, g_nActScreenW, g_nActScreenH);
}

void Utils::startContainerSilently()
{
    struct str_info userInfo = UserInfo::getuserinfo();
    // will not start container on wayland platform
    kmre::DBusClient::getInstance()->StartContainerSilently(userInfo.name_info, userInfo.uid_info);
}

bool isFileExist(QString fullFilePath)
{
    QFileInfo fileInfo(fullFilePath);
    if(fileInfo.exists()) {
        return true;
    }
    return false;
}

void getAppListFromXml(const QString &filePath, QStringList &appList, 
                        const QString rootTag, const QString childTag)
{
    appList.clear();
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString errorStr;
        int errorLine;
        int errorColumn;

        QDomDocument doc;
        if (doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn)) {
            QDomElement root = doc.documentElement();
            if (root.tagName() == rootTag) {
                QDomNode child = root.firstChild();
                while (!child.isNull()) {
                    if (child.toElement().tagName() == childTag) {
                        appList << child.toElement().text();
                    }
                    child = child.nextSibling();
                }
            }
        }
        else {
            syslog(LOG_ERR, "[%s] Parse error at line '%d' column '%d' : %s", 
                __func__, errorLine, errorColumn, errorStr.toStdString().c_str());
        }
    }
    else {
        syslog(LOG_ERR, "[%s] Open file:'%s' failed!", __func__, filePath.toStdString().c_str());
    }

    // syslog(LOG_DEBUG, "[%s] Get app list from '%s':", __func__, filePath.toStdString().c_str());
    // for (auto app : appList) {
    //     syslog(LOG_DEBUG, "[%s] %s", __func__, app.toStdString().c_str());
    // }
}

bool isAppSupportDDS(const QString& pkgName)
{
    bool isHostSupportDDS = kmre::DBusClient::getInstance()->isHostSupportDDS();

    if (isHostSupportDDS) {
        QStringList supportDDSAppList;
        getAppListFromXml("/usr/share/kmre/config/dynamic_size_app_list.xml", supportDDSAppList, "packages", "item");
        return supportDDSAppList.contains(pkgName);
    }

    return false;
}

bool readGlobalconfigFromFile(const QString &pkgName, int &width, int &height, int &density, int &orientation)
{
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

        bool supportDDS = isAppSupportDDS(pkgName);
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
                        if (object.contains("width")) {
                            width = object["width"].toInt();
                        }

                        if (object.contains("height")) {
                            height = object["height"].toInt();
                        }

                        if (supportDDS && object.contains("density_dds")) {
                            density = object["density_dds"].toInt();
                        }
                        else if (object.contains("density")) {
                            density = object["density"].toInt();
                        }

                        if (object.contains("orientation")) {
                            orientation = object["orientation"].toInt();
                        }

                        break;
                    }
                }
            }
        }
    }

    return true;
}

bool Utils::launchApplication(const QString &pkgName, bool fullScreen, int width, int height, int density)
{
    const char *libPath = "/usr/lib/libkmre.so";
    QString curLibPath = QString(QLatin1String(libPath));
    if (!isFileExist(curLibPath)) {
        //qDebug()<<libPath<<"is not exits!!!";
        syslog(LOG_ERR, "%s is not exits!!!", libPath);
        exit(0);
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath, RTLD_LAZY);
    if (!module_handle) {
        syslog(LOG_ERR, "dlopen %s failed!!!", libPath);
        exit(0);
    }

    bool (*launch_app)(char *appname, bool fullscreen, int width, int height, int density);
    launch_app = (bool(*)(char *, bool, int, int, int))dlsym(module_handle, "launch_app");
    if ((module_error = dlerror()) != NULL) {
        //qDebug()<<"call launch_app failed:"<<module_error;
        syslog(LOG_ERR, "call launch_app failed: %s", module_error);
        dlclose(module_handle);
        exit(0);
    }

    syslog(LOG_DEBUG, "[%s] Start launch app:'%s'...", __func__, pkgName.toUtf8().constData());
    kmre::DBusClient::getInstance()->LaunchApp(pkgName, width, height);
    bool ret = launch_app((char *)pkgName.toUtf8().constData(), fullScreen, width, height, density);
    dlclose(module_handle);

    syslog(LOG_INFO, "[%s] Launch app:'%s' %s!", __func__, pkgName.toUtf8().constData(), ret ? "succeed" : "failed");
    return ret;
}

bool isFullScreenConfiguraed(const QString &pkgName)
{
    QString configFile = QString("%1/.config/kmre/kmre.ini").arg(QDir::homePath());
    QSettings kmreSettings(configFile, QSettings::IniFormat);
    kmreSettings.setIniCodec("UTF-8");

    QStringList bootFullScreenAppsList;
    kmreSettings.beginGroup("boot_fullscreen");
    bootFullScreenAppsList = kmreSettings.value("apps_list").toStringList();
    kmreSettings.endGroup();

    return bootFullScreenAppsList.contains(pkgName);
}

void getApplicationStartConfig(const QString &pkgName, bool &fullScreen, int &width, int &height, int &density)
{
    qDebug() << "get application start configs...";
    syslog(LOG_DEBUG, "[%s] get application start configs...", __func__);

    // max resolution
    const int maxWidth = (gDisplayInfo.physicalWidth > 0) ? gDisplayInfo.physicalWidth : 1080;
    const int maxHeight = (gDisplayInfo.physicalHeight > 0) ? gDisplayInfo.physicalHeight : 1920;
    // default resolution
    const int defaultWidth = (gDisplayInfo.displayWidth > 0) ? gDisplayInfo.displayWidth : 720;
    const int defaultHeight = (gDisplayInfo.displayHeight > 0) ? gDisplayInfo.displayHeight : 1280;
    
    const int minDensity = 160;// min density
    const int defaultDensity = 320;// default density
    
    // read all config from command line
    bool inputFullScreen = false;
    int inputWidth = 0;
    int inputHeight = 0;
    int inputDensity = 0; 

    Utils::getOptionsFromArgument(inputFullScreen, inputDensity, inputWidth, inputHeight);
    width = ((inputWidth > 0) && (inputWidth <= maxWidth)) ? inputWidth : 0;
    height = ((inputHeight > 0) && (inputHeight <= maxHeight)) ? inputHeight : 0;

    if (inputFullScreen) {
        fullScreen = true;
        width = (width == 0) ? maxHeight : width;
        height = (height == 0) ? maxWidth : height;
        density = 320;
        return;
    }

    // read global config (width, height, orientation, density) from config file
    int configWidth = 0;
    int configHeight = 0;
    int configDensity = -1;
    int configOrientation = -1;

    readGlobalconfigFromFile(pkgName, configWidth, configHeight, configDensity, configOrientation);

    int validDensity = (gDisplayInfo.density >= minDensity) ? gDisplayInfo.density : defaultDensity;
    density = (configDensity >= minDensity) ? configDensity : validDensity;
    // 竖屏显示
    width = (width > 0) ? width : (((configWidth > 0) && (configWidth <= maxWidth)) ? configWidth : defaultWidth);
    height = (height > 0) ? height : (((configHeight > 0) && (configHeight <= maxHeight)) ? configHeight : defaultHeight);

    // read custom config (full screen) from config file
    fullScreen = isFullScreenConfiguraed(pkgName);

    if (fullScreen && (configOrientation == -1)) {
        configOrientation = 1;
    }

    if (configOrientation == 1) {// 横屏显示
        if (fullScreen) {
            width = maxHeight;
            height = maxWidth;

            if (configDensity == -1) {
                if ((width >= 1920) && (height >= 1080)) {
                    density = 480;
                }
                else if ((width >= 1440) && (height >= 900)) {
                    density = 400;
                }
                else {
                    density = validDensity;
                }
            }
        }
        else {
            std::swap(width, height);
        }
    }
}

void Utils::startApplication(const QString &pkgName)
{
    qDebug() << "start app: " << pkgName;
    if (pkgName.isEmpty()) {
        return;
    }

    bool fullScreen;
    int width, height, density;
    getApplicationStartConfig(pkgName, fullScreen, width, height, density);

    syslog(LOG_INFO, "[%s] App configs: fullScreen = %d, width = %d, height = %d, density = %d", 
            __func__, fullScreen, width, height, density);

    if (!launchApplication(pkgName, fullScreen, width, height, density)) {
        qDebug() << "start app failed!";
    }

    struct str_info userInfo = UserInfo::getuserinfo();
    qDebug() << "start to change runtime status.";
    kmre::DBusClient::getInstance()->ChangeContainerRuntimeStatus(userInfo.name_info, userInfo.uid_info);
}

bool Utils::isAndroidDeskStart()
{
    struct str_info my_info;
    my_info = UserInfo::getuserinfo();
    //qDebug() << "start check kmre start...";
    QString loop_value = kmre::DBusClient::getInstance()->GetPropOfContainer(my_info.name_info, my_info.uid_info, "sys.kmre.boot_completed");
    if (loop_value == "1") {
        return true;
    }
    else {
        return false;
    }
}

void Utils::startAppDaemon()
{
}

static std::set<std::string> loadedModuleSet;

static void updateModulesLoaded()
{
    FILE* fp = nullptr;
    char line[1024] = {0};

    fp = fopen(PROC_MODULES_PATH, "r");
    if (fp == nullptr) {
        return;
    }

    while (fgets(line, sizeof(line), fp) != nullptr) {
        char* p = line;
        while (p != nullptr && *p != '\0' && !isspace(*p)) {
            p++;
        }

        *p = '\0';
        loadedModuleSet.insert(line);
    }

    fclose(fp);
}

static bool shouldLoadModule(const char* module)
{
    std::set<std::string>::const_iterator it = loadedModuleSet.find(module);
    return (it == loadedModuleSet.end());
}

bool Utils::isKernelModuleLoaded(const std::string& moduleName)
{
    updateModulesLoaded();
    return !shouldLoadModule(moduleName.c_str());
}

bool Utils::isPathFileType(const std::string &path, mode_t fileType)
{
    struct stat sb;
    if (stat(path.c_str(), &sb) != 0) {
        return false;
    }

    return ((sb.st_mode & S_IFMT) == fileType);
}

bool Utils::isPathCharDevice(const std::string &path)
{
    return isPathFileType(path, S_IFCHR);
}

QString Utils::getContainerName()
{
    struct str_info info = UserInfo::getuserinfo();
    return "kmre-" + QString::number(info.uid_info) + "-" + info.name_info;
}
