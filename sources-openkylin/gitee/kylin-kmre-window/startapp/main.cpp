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

#include "dialog.h"
#include "dbus_client.h"
#include "utils.h"
#include "start_control.h"
#include "get_userinfo.h"
#include "preferences.h"
#include "displayinfo.h"
#include "preinstallappmanager.h"

#include <QApplication>
#include <QThread>
#include <QProcess>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QFile>
#include <QSettings>
#include <QTextCodec>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDir>
#include <QDirIterator>
#include <QLabel>
#include <QVBoxLayout>
#include <QLockFile>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <sys/utsname.h>

static bool isUbuntuKylinOS = false;
DisplayInfo gDisplayInfo;

#define LOG_IDENT "KMRE_startapp"

static QString translationPath()
{
    QString path;
    if (QDir("/usr/share/startapp/translations").exists()) {
        path = "/usr/share/startapp/translations";
        return path;
    }
    else {
        return qApp->applicationDirPath() + "/translations";
    }
}

//check UID
static int root_check()
{
    if (getuid() == 0) {
        return 0;
    }

    return -1;
}

static bool shouldStartSilently()
{
    QFile file("/etc/os-release");
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString &content = QString::fromUtf8(file.readAll());
            file.close();
            if (content.contains("edu")) {
                return false;
            }
        }
    }

    Preferences *m_pref = new Preferences;
    if (m_pref && (!m_pref->m_KmreAutoStart)) {
        return false;
    }
    if (m_pref) {
        delete m_pref;
        m_pref = nullptr;
    }

    QString installedFilePath = "/var/lib/kmre/" + Utils::getContainerName() + "/data/local/tmp/installed.json";
    QFile installedFile(installedFilePath);

    if (!installedFile.exists()) {
        //qDebug() << "File " + installedFilePath + " doesn't exist.";
        return false;
    }

    installedFile.open(QFile::ReadOnly);
    if (!installedFile.isOpen()) {
        //qDebug() << "Failed to open file " + installedFilePath + ".";
        return false;
    }

    QByteArray json = installedFile.readAll();
    installedFile.close();

    //qDebug() << json;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError) {
        //qDebug() << "Failed to parse file " + installedFilePath + ".";
        return false;
    }

    QJsonObject obj = doc.object();

    if (!obj.contains(QStringLiteral("data"))) {
        //qDebug() << "Json doesn't have data array.";
        return false;
    }

    QJsonArray dataArray = obj.value("data").toArray();
    //qDebug() << dataArray.isEmpty();
    //qDebug() << dataArray.size();
    //qDebug() << dataArray;
    if (dataArray.isEmpty() || dataArray.size() <= 0) {
        //qDebug() << "Data array doesn't have any item.";
        return false;
    }

    return true;
}

bool checkBinderModule()
{
    return QFile::exists("/dev/binder") || QFile::exists("/dev/binders/binder0");
}

void showMsgForBinderModule()
{
    if (isUbuntuKylinOS) {
        struct utsname un;
        QString headers = "linux-headers-";
        if (uname(&un) == 0) {
            QString headers_version = QString(QLatin1String(un.release));
            headers = headers + headers_version;
        }

        qDebug() << "headers: " << headers;
        QWidget *information = new QWidget;
        information->setWindowTitle(QObject::tr("Tips"));
        information->setWindowIcon(QIcon(":/image/images/kmre.svg"));
        information->setFixedSize(400,180);
        QLabel *m_title = new QLabel;
        m_title->setText(QObject::tr("The %1 package is not installed;").arg(headers));
        QLabel *m_tip = new QLabel;
        m_tip->setText(QObject::tr("After installing the dependency package, re-install the kylin-kmre-modules-dkms package to meet the environment required by KMRE, then restart your computer and try to experience KMRE;"));
        m_title->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_tip->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_title->setWordWrap(true);
        m_tip->setWordWrap(true);
        QVBoxLayout *layout = new QVBoxLayout(information);
        layout->addSpacing(16);
        layout->addWidget(m_title);
        layout->addWidget(m_tip);
        layout->addStretch();
        information->show();
        QDesktopWidget* m = QApplication::desktop();
        QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
        int desk_x = desk_rect.width();
        int desk_y = desk_rect.height();
        int x = information->width();
        int y = information->height();
        information->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
    } 
    else {
        QWidget *information = new QWidget;
        information->setWindowTitle(QObject::tr("Tips"));
        information->setWindowIcon(QIcon(":/image/images/kmre.svg"));
        information->setFixedSize(400,100);
        QLabel *m_tip = new QLabel;
        m_tip->setText(QObject::tr("binder not found, please confirm the environment required by KMRE."));
        m_tip->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_tip->setWordWrap(true);
        QVBoxLayout *layout = new QVBoxLayout(information);
        layout->addSpacing(16);
        layout->addWidget(m_tip);
        layout->addStretch();
        information->show();
        QDesktopWidget* m = QApplication::desktop();
        QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
        int desk_x = desk_rect.width();
        int desk_y = desk_rect.height();
        int x = information->width();
        int y = information->height();
        information->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
    }
}

int main(int argc, char *argv[])
{
    int ret = 0;

    // 适配4K屏
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication a(argc, argv);
    QGuiApplication::setOrganizationName("TianJin Kylin");
    QGuiApplication::setApplicationName("startapp");
    QGuiApplication::setApplicationVersion("3.0");

    openlog(LOG_IDENT, LOG_NDELAY | LOG_NOWAIT | LOG_PID, LOG_USER);

    if (argc<2){
        //qDebug()<<"can not get the pkgname of application";
        syslog(LOG_ERR, "can not get the pkgname of application");
        return 0;
    }

    bool startSilently = (strcmp(argv[1], "start_kmre_silently") == 0);
    bool Silentlywithoutapp = (strcmp(argv[1], "silently_without_apps") == 0);

#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

    QString locale = QLocale::system().name();
    QString translatorFile = "startapp_" + locale + ".qm";
    QTranslator translator;
    if (!translator.load(translatorFile, translationPath())) {
        qDebug() << "Load translation file："<< translatorFile << " failed!";
    }
    else {
        a.installTranslator(&translator);
    }

    //加载Qt对话框默认的国际化
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));// /usr/share/qt5/translations
    a.installTranslator(&qtTranslator);

    qDebug() << "example: startapp com.tencent.mm -W 720 -H 1280";

    if (root_check() == 0) {
        fprintf(stderr, "user is root.\n");
        syslog(LOG_ERR, "main: user is root.");
        Dialog w("unsupported_root");
        w.show();
        return a.exec();
    }

    if (startSilently) {
        for (int i = 0;i < 10; i++) {
            if (checkBinderModule()) {
                break;
            }
            usleep(500*1000);
        }
    }

    if (!checkBinderModule()) {
        if (startSilently) {
            syslog(LOG_ERR, "main: binder not found, please confirm the environment required by KMRE.");
            return 0;
        } 
        else {
            showMsgForBinderModule();
            return a.exec();
        }
    }

    PreInstallAppManager::preInstallApks();

    kmre::DBusClient *dBusClient = kmre::DBusClient::getInstance();
    if (startSilently && (!shouldStartSilently())) {
        dBusClient->StartUserService("cn.kylinos.Kmre.FileWatcher", "/cn/kylinos/Kmre/FileWatcher", "cn.kylinos.Kmre.FileWatcher");
        return 0;
    }

    dBusClient->GetDisplayInformation();
    qDebug() << "Display type: " << gDisplayInfo.displayType;
    qDebug() << "CPU type: " << gDisplayInfo.cpuType;
    qDebug() << "CPU adapted: " << gDisplayInfo.cpuSupported;
    qDebug() << "GPU vendor: " << gDisplayInfo.gpuVendor;
    qDebug() << "GPU model: " << gDisplayInfo.gpuModel;
    qDebug() << "GPU adapted: " << gDisplayInfo.gpuSupported;
    syslog(LOG_INFO, "CPU type '%s' %s", 
            gDisplayInfo.cpuType.toStdString().c_str(),
            gDisplayInfo.cpuSupported ? "adapted" : "not adapted yet!");
    syslog(LOG_INFO, "GPU vendor '%s' (model '%s') %s", 
            gDisplayInfo.gpuVendor.toStdString().c_str(), 
            gDisplayInfo.gpuModel.toStdString().c_str(),
            gDisplayInfo.gpuSupported ? "adapted" : "not adapted yet!");

    Preferences *m_pref = new Preferences;
    m_pref->getmaxWindowNum();

    //当kylin-kmre-manager提供的dbus服务异常时，GetDisplayInformation获取的值会全部为空，且获取过程耗时很长
    if (gDisplayInfo.cpuType.isEmpty() && 
        (gDisplayInfo.displayType == "unknown") && 
        (gDisplayInfo.gpuVendor == "unknown")) {
        fprintf(stderr, "Manager Dbus Service Exception.\n");
        syslog(LOG_ERR, "Manager Dbus Service Exception, the process will exit immediately.");
        Dialog w("manager_service_exception");
        w.show();
        return a.exec();
    }

    QString appName = QString(argv[1]);
    QStringList appList = dBusClient->GetRunningAppList();
    int numOfRunningWindows = appList.size();
    bool isAppRunning = appList.contains(appName);
    //syslog(LOG_DEBUG, "Start app: %s, isAppRunning = %d, numOfRunningWindows = %d", argv[1], isAppRunning, numOfRunningWindows);
    if (isAppRunning) {
        dBusClient->ActiveApp(appName);
        syslog(LOG_DEBUG, "App is running, exit now!");
        return 0;
    }
    
    //don't need to call this func, because kmre manager service has already started in func 'GetDisplayInformation';
    //dBusClient->StartKmreManager();
    //don't need to call this func, because kmre window service has already started in func 'GetRunningAppList';
    //dBusClient->StartKmreWindow();

    //qDebug() << "StartUserService: cn.kylinos.Kmre.Audio";
    dBusClient->StartUserService("cn.kylinos.Kmre.Audio", "/cn/kylinos/Kmre/Audio", "cn.kylinos.Kmre.Audio");
    //qDebug() << "StartUserService: cn.kylinos.Kmre.FileWatcher";
    dBusClient->StartUserService("cn.kylinos.Kmre.FileWatcher", "/cn/kylinos/Kmre/FileWatcher", "cn.kylinos.Kmre.FileWatcher");
    //qDebug() << "StartUserService: com.kylin.Kmre.sensor";
    dBusClient->StartUserService("com.kylin.Kmre.sensor", "/", "com.kylin.Kmre.sensor");
    //qDebug() << "StartUserService: com.kylin.Kmre.gpsserver";
    dBusClient->StartUserService("com.kylin.Kmre.gpsserver", "/", "com.kylin.Kmre.gpsserver");
    
    QLockFile lockFile(QDir::temp().absoluteFilePath("kylin-kmre-startapp.lock"));
    if (!lockFile.tryLock(30000)) {
        syslog(LOG_ERR, "Starting kmre blocked!");
        return -1;
    }

    if (Utils::isAndroidDeskStart()) {
        if (!strcmp(argv[1], "start_kmre")) {
//            qDebug() << "Kmre is aready started";
//            syslog(LOG_INFO, "kmre is aready started");
        }
        else {
            Utils::startApplication(argv[1]);
        }
        return 0;
    }

    if (startSilently) {
        StartControl sc;
        sc.startEnvSilently();
        ret = a.exec();
    }
    else if (Silentlywithoutapp) {
        int lockfd = Utils::checkSingle();
        Dialog w(argv[1]);
        ret = a.exec();

        if (lockfd >= 0) {
            close(lockfd);
        }
    }
    else {
        int lockfd = Utils::checkSingle();
        Dialog w(argv[1]);
        w.show();
        ret = a.exec();

        if (lockfd >= 0) {
            close(lockfd);
        }
    }

    return ret;
}
