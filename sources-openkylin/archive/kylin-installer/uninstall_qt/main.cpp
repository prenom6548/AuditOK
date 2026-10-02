/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <QApplication>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QString>
#include <QTranslator>
#include <QByteArray>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <QStringList>
#include <QFile>
#include <QHash>
#include <QDebug>
#include <glib.h>
#include "single.h"
#include "lib_fun.h"
#include <qlogging.h>
#include <stdio.h>
#include <stdlib.h>
#include <qdatetime.h>
#include <singleapplication.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include <signal.h>
#include <QDBusPendingCall>

#include "main_window.h"
#include "single_window.h"
#include "xatom-helper.h"
#include "adaptscreeninfo.h"
#include <libkylog.h>
#include "../src/common/utils/CustomBoxUtil.h"

#define IRREGULARPACKAGE_FILE_PATH "/usr/share/kylin-installer/irregularPackage.list"
#define LOCAL_PACKAGE_FLAG "local_pkg"
#define KAIMING_PACKAGE "kaiming"
#define LOCAL_ANDROID_PACKAGE_FLAG "local_Android_pkg"
#define ANDROID_DESKTOP_FILE_PATH ".local/share/applications"
/*
 *now , kylin-uninstall use command lock and uninstall lock
 *cache 35 signal , po#include <singleapplication.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include <signal.h>p corresponding interface
 *command format : kylin-uninstall desktop_path
 */




QHash <QString, QStringList> irregularPackageList;



static void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *message = localMsg.constData();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    switch (type) {
    case QtDebugMsg:
        klog_debug("%s (%s:%u,%s)\n", message, file, context.line, function);
        break;
    case QtInfoMsg:
        klog_info("%s (%s:%u,%s)\n", message, file, context.line, function);
        break;
    case QtWarningMsg:
        klog_warning("%s (%s:%u,%s)\n", message, file, context.line, function);
        break;
    case QtCriticalMsg:
        klog_err("%s (%s:%u,%s)\n", message, file, context.line, function);
        break;
    case QtFatalMsg:
        klog_emerg("%s (%s:%u,%s)\n", message, file, context.line, function);
        break;
    }

    return;
}

void sendmessg_notifily()
{
    QString dbus_service = "org.freedesktop.Notifications";
      QString dbus_path = "/org/freedesktop/Notifications";
      QString dbus_interface = "org.freedesktop.Notifications";
      QString notify = "Notify";
      QList<QVariant> argsset;
      QStringList action;
      QMap<QString,QVariant> hint;
      QString name = QObject::tr("kylin-uninstaller");
      QString title = QObject::tr("Software uninstallation");
      QString string = QObject::tr("There is software being uninstalled. Please try again later.");
      uint32_t replaceid = 0;
      QString icon = "kylin-installer";
      qDebug() << name;
      argsset << name
           << replaceid
           << icon
           << title
           << string
           << action
           << hint
           << -1;
      QDBusInterface *iface = new QDBusInterface(dbus_service, dbus_path, dbus_interface, QDBusConnection::sessionBus());
      iface->asyncCallWithArgumentList(notify,argsset);


}

void readIrregularPackageListFile()
{
    QFile file(IRREGULARPACKAGE_FILE_PATH);
    if (!file.exists()) {
        qWarning() << "不规则包列表文件不存在";
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "文件打开失败";
        return;
    }
    irregularPackageList.clear();
    QTextStream in(&file);
    while (1) {
        QString line = in.readLine();
        if (line.size() == 0) {
            break;
        }
        QStringList lineList = line.split(":");
        QStringList valueList = QString(lineList.at(1)).split(",");
        irregularPackageList.insert(lineList.at(0), valueList);
    }
    return;
}

QString iterPackageHash(const QString &desktopName)
{

    if (desktopName.isEmpty()) {
        return QString();
    }
    /*遍历hash*/
    QString desktopFileName = QString(desktopName);
    QHash<QString, QStringList>::const_iterator iter1 = irregularPackageList.constBegin();
    while (iter1 != irregularPackageList.constEnd()) {
        qDebug()  << "." << iter1.key() << ": " << iter1.value();
        foreach (QString desktop, iter1.value()) {
            QString removePathDesktop = desktop.split("/").last();
            if (desktopFileName.contains(removePathDesktop)) {
                return iter1.key();
            }
        }
        ++iter1;
    }
    return QString();
}

int getLocalPackageDesktopField(char *desktopPath)
{
    GError **error = NULL;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();
    g_key_file_load_from_file(keyfile, desktopPath, flags, error);
    char *keywords = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Keywords", NULL, error);
    QString s_keywords(keywords);
    if (s_keywords == "Android;App;Apk") {
        return 1;
    }
    return 0;
}

QString getKaimingPakckageInfo(QString desktopPath)
{
    QString ret = "";
    QSettings settings(QString(desktopPath), QSettings::IniFormat);
    QStringList list = settings.allKeys();

    // 卸载逻辑有问题
    if (list.contains("Desktop Entry/X-Kaiming")){
        ret = settings.value("Desktop Entry/X-Kaiming").toString();
    }
    return ret;

}

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    qInstallMessageHandler(logOutput);
    if (argc != 2) {
        qDebug() << "argc != 2";
        return 0;
    }

    if (0 == getuid()) {
        printf("权限错误，请以普通用户权限运行程序！\n");
        exit(0);
    }

    QString firstArg = QString(argv[1]);
    qDebug() << "firstArg:::" << firstArg;
    if (firstArg.size() > 512){
        qDebug() << firstArg << "    文件路径过长不符合规范!!!!!!";
        return 0;
    }


    QString id = QString("kylin-uninstaller-" + QLatin1String(getenv("DISPLAY")));
    /* 服务端存在检测包名是否存在功能，此处不再做检测 */
    kdk::QtSingleApplication a(id,argc, argv);



    QFileInfo file("/usr/share/qt5/translations/qt_zh_CN.qm");
    QTranslator * translator = new QTranslator();
    if(file.exists()!=false){
        translator->load("/usr/share/qt5/translations/qt_zh_CN.qm");
        a.installTranslator(translator);

    }
    a.setStyle("ukui");
//    std::string strTmp = argv[1];
//    QString str = QString::fromStdString(strTmp);
    QFile filein(firstArg);
    if (!filein.exists()){
        qDebug() << firstArg << "    文件路径不存在!!!!!!";
        return 0;
     }
    signal(SIGCHLD, SIG_IGN);

    QTranslator *p_translator = new QTranslator;
    if (p_translator->load(QLocale() , QLatin1String("kre_uninstall") , QLatin1String("_") , QLatin1String("/usr/share/kre/kre-install/translations")))
        QApplication::installTranslator(p_translator);
    else
        qDebug() << "cannot load translator kre_uninstall" << QLocale::system().name() << ".qm";

    if (a.isRunning()) {
        a.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : a.applicationFilePath());
        qDebug() << QObject::tr("kylin-uninstaller is already running!");
        sendmessg_notifily();
        return EXIT_SUCCESS;
    }

    /*test print*/
    for (int i = 0 ; i < argc ; i++) {
        syslog(LOG_USER | LOG_INFO , "Info : main args is [%d->%s]\n" , i , argv[i]);
    }

    int i_ret = -1;
    signal(SIGCHLD, SIG_IGN);

    if (argc != 2) {
        printf("Error : cmd format error [kylin-uninstall desktop_path]\n");
        return -1;
    }

    /* set env value */
    setenv("QT_QPA_PLATFORMTHEME" , "ukui" , true);
    readIrregularPackageListFile();
    QString packageName = iterPackageHash(firstArg);
    qDebug() << "origin packageName:::" << packageName;

    /*get pkg_name and runtime*/

    QString p_pkgname;
    QString p_runtime;

    QString zh_name;

    if (!firstArg.isEmpty()) {
        if (firstArg.contains("kylin-v10")) {
            if (snow_analysis_xml(firstArg , p_pkgname, p_runtime))
                return -1;

        } else {
            if (!packageName.isEmpty()) {
                p_pkgname = packageName;

            } else if (firstArg.contains(ANDROID_DESKTOP_FILE_PATH)) {
                QSettings setting(firstArg, QSettings::IniFormat);
                QString categories = setting.value("Desktop Entry/Categories").toString();
                if (categories.contains("Android")) {
                    packageName = getAndroidPackageName(QString(firstArg));
                    qDebug() << "getAndroidPackageName:::" << packageName;
                    zh_name = get_zhname(QString(firstArg));
                    p_runtime = LOCAL_ANDROID_PACKAGE_FLAG;
                } else {
                    packageName = snow_get_local_pkg_name(QString(firstArg));
                    qDebug() << "snow_get_local_pkg_name:::" << packageName;
                    zh_name = get_zhname(QString(firstArg));
                    p_runtime = LOCAL_PACKAGE_FLAG;
                }

            } else if (QString(firstArg).contains("/opt/kare/usr/share/applications/")) {
                packageName = getKarePackageNameFromFile(QString(firstArg));
                qDebug() << "getKarePackageNameFromFile:::" << packageName;
                zh_name = get_namezh_util(QString(firstArg), packageName);
                // fixme:kare目前和dpkg共用同一套逻辑
                p_runtime = LOCAL_PACKAGE_FLAG;

            } else if (QString(firstArg).contains("/usr/share/applications/") || QString(firstArg).contains("/opt/kaiming/")) {
                /*先判断是否是开明包*/
                packageName =  getKaimingPakckageInfo(firstArg);
                qDebug() << "getKaimingPakckageInfo:::" << packageName;
                if (packageName == "") {
                    packageName = snow_get_local_pkg_name(QString(firstArg));
                    qDebug() << "snow_get_local_pkg_name:::" << packageName;
                    p_runtime = LOCAL_PACKAGE_FLAG;
                } else {
                    p_runtime = KAIMING_PACKAGE;
                }
                zh_name = get_namezh_util(QString(firstArg), packageName);

            } else {
                /*  默认处理逻辑,兼容其余场景
                    1. kare; 2. kaiming; 3. dpkg
                */
                qDebug() << "默认处理逻辑,兼容其余场景";
                QString desktopReal;
                desktopReal = resolveRealFile(firstArg);
                qDebug() << "desktopReal:::" << desktopReal;
                packageName = getKarePackageNameFromFile(desktopReal);
                qDebug() << "getKarePackageNameFromFile:::" << packageName;
                if (packageName != "") {
                    p_runtime = LOCAL_PACKAGE_FLAG;
                } else {
                    packageName = getKaimingPakckageInfo(desktopReal);
                    qDebug() << "getKaimingPakckageInfo:::" << packageName;
                    if (packageName != "") {
                        p_runtime = KAIMING_PACKAGE;
                    } else {
                        packageName = snow_get_local_pkg_name(desktopReal);
                        qDebug() << "snow_get_local_pkg_name:::" << packageName;
                        p_runtime = LOCAL_PACKAGE_FLAG;
                    }
                }
                zh_name = get_namezh_util(firstArg, packageName);
            }
        }
    } else {
        qInfo()<<"main fun arg is error";
        return -1;
    }

    qDebug() << "final packageName:::" << packageName;

    if (packageName.isEmpty()||packageName.contains(" ", Qt::CaseSensitive)) {
        QMessageBox *msgBox = CustomBoxUtil::createCritialBox("",QObject::tr("No valid software package found"),"");
        msgBox->exec();
        qInfo()<<"packageName is empty!";
        return -1;
    }

    a.setWindowIcon(QIcon::fromTheme("kylin-installer"));
    main_window w(packageName, p_runtime, firstArg, zh_name);

    kdk::UkuiStyleHelper::self()->removeHeader(&w);
    /* handle double screen problem */
    adaptScreenInfo adapt;
    int x = adapt.m_nScreen_x;
    int y = adapt.m_nScreen_y;
    int width = adapt.m_screenWidth;
    int height = adapt.m_screenHeight;
    w.resize(420,180);
    w.setFixedSize(420,180);
    w.move((width / 2 + x) - w.width() / 2 , (height / 2 + y )- w.height() / 2);

    w.show();

    return a.exec();
}
