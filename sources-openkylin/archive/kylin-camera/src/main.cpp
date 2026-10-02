#include "mainwindow.h"
#include "titlebar.h"
#include <QApplication>
#include <QProcessEnvironment>
#include <QMainWindow>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QTranslator>
#include <QCoreApplication>
#include <QPixmap>
#include <QDebug>
#include <QMessageBox>
#include <QLibraryInfo>
#include <unistd.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/file.h>
#include <fcntl.h>
#include <syslog.h>
#include <KWindowEffects>
#include "customstyle.h"
#include "util/xatom-helper.h"
#include <log.hpp>
#include <kylin_system/window_management.hpp>
#include <single_application/single_application.hpp>
#include "windowmanager/windowmanager.h"
#include <log.hpp>
#include <QDesktopWidget>

#ifdef NOTKAIMING
#define TRANS_PATH  "/usr/share/kylin-camera/translations/"
#else
#define TRANS_PATH  "/opt/apps/top.openkylin.kylin-camera/files"
#endif
#define USD_GSETTING_PATH "org.ukui.SettingsDaemon.plugins.xsettings"
#define USD_GESTTING_SCALEKEY "scaling-factor"
QByteArray qtPlatform(void){
    QByteArray scale = "1";
    // 获取当前的环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // 读取环境变量
    QString path = env.value("XDG_SESSION_TYPE");
    qDebug() << "XDG_SESSION_TYPE:"<< path;
    if(path == "wayland") {
        // 设置一个新的环境变量
        env.insert("QT_QPA_PLATFORM","wayland");
        // 将修改后的环境变量应用到当前进程
        qputenv("QT_QPA_PLATFORM","wayland");

        // if (QGSettings::isSchemaInstalled(USD_GSETTING_PATH)) {
        //            QGSettings scaleSettings(USD_GSETTING_PATH);
        //            if (scaleSettings.keys().contains("scalingFactor")) {
        //                scale = scaleSettings.get(USD_GESTTING_SCALEKEY).toByteArray();
        //                qputenv("QT_SCALE_FACTOR", scale);
        //            }
        //        }

    }
    // 再次读取并打印环境变量以确认设置成功
    QString newPlatform = env.value("QT_QPA_PLATFORM");
    qDebug() << "QT_QPA_PLATFORM:"<< newPlatform;
    return scale;
}

int main(int argc, char *argv[])
{
    QByteArray scale  = qtPlatform();
    /* 使用sdk管理日志 */
    qInstallMessageHandler(kdk::kabase::Log::logOutput);
    QString kylinCameraFileName;
    if (argc > 1) {
        kylinCameraFileName = argv[1];
    }
    qDebug() << "Load translation file："<< "kylin-camera_" ;
    printf("Program start ...\n");
    // 适配4K屏
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    #endif

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    #endif

    kdk::kabase::QtSingleApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    app.setWindowIcon(QIcon::fromTheme("kylin-camera"));
    app.setApplicationVersion(APP_VERSION);
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    if (app.isRunning()) {
        qDebug() << "is running";
        app.sendMessage(kylinCameraFileName);
        return 0;
    }

    //加载翻译文件
    QString trans_path;
    if (QDir(TRANS_PATH).exists()) {
        trans_path = TRANS_PATH;
    }
    else {
        trans_path = qApp->applicationDirPath() + "/translations";
    }
    QTranslator app_trans;
    QTranslator qt_trans;
    QTranslator trans_SDK;
    QString locale = QLocale::system().name();
    QString qt_trans_path;
    qt_trans_path = QLibraryInfo::location(QLibraryInfo::TranslationsPath);// /usr/share/qt5/translations

    if(!app_trans.load("kylin-camera_" + locale + ".qm", trans_path))
        qDebug() << "Load translation file："<< "kylin-camera_" + locale + ".qm from" << trans_path << "failed!";
    else
        app.installTranslator(&app_trans);

    if(!qt_trans.load("qt_" + locale + ".qm", qt_trans_path))
        qDebug() << "Load translation file："<< "qt_" + locale + ".qm from" << qt_trans_path << "failed!";
    else
        app.installTranslator(&qt_trans);
    if(!trans_SDK.load(":/translations/gui_" + locale + ".qm"))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        app.installTranslator(&trans_SDK);




    QStringList strList;
    for(int i = 0; argv[i];i++){
        if(argv[i]){
            QString a = argv[i];
            strList.push_back(a);
            qDebug() << i + ":" + a;
        }
    }
    MainWindow w(strList);
    w.setAccessibleName("kylin-camera_main_MainWindow_w");

    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        QObject::connect(&app, &kdk::kabase::QtSingleApplication::messageReceived, [&](){
           kdk::WindowManager::activateWindow(w.windowId());
        });
    }
#ifndef __V10__
    //添加窗管协议
    kdk::kabase::WindowManagement::setWindowMotifHint(w.winId());
#endif
    app.setActivationWindow(&w);
    app.setApplicationName(QApplication::tr("Camera"));
    w.show();

    int screen_num = qApp->desktop()->screenNumber(&w);
    kdk::WindowManager::setGeometry(w.windowHandle(),
                                       QRect(qApp->desktop()->screenGeometry(screen_num).center() - w.geometry().center(),
                                       QSize(w.size())));
    return app.exec();

}
