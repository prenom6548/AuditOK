/*
 * Copyright (C) 2002  KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#define SHADOW 6//阴影宽度
#define BUTTONRADIUS 0//按钮圆角
#define SHADOWALPHA 0.16//阴影透明度
#include "include/qtsingleapplication.h"
#include <QTranslator>
#include <QDebug>
#include <QDateTime>

#include <QGuiApplication>
#include <QApplication>
#include <QStringList>
#include <QStandardPaths>
#include <ukui-log4qt.h>
#include <fcntl.h>
#include <QLibraryInfo>
#include "include/xatom-helper.h"
#include "mainwindow.h"
#include "./include/global.h"
#include "kysdk/applications/ukuistylehelper/ukuistylehelper.h"
#include "kysdk/applications/windowmanager/windowmanager.h"
#if (QT_VERSION <= QT_VERSION_CHECK(5,15,0))
#include "dbusadaptor.h"
#endif
#include "include/qtsingleapplication.h"

// 拉起最小化窗口
void activeMainwindow()
{
    if(QGSettings::isSchemaInstalled(APPDATA))
    {
        QGSettings *p = new QGSettings(APPDATA);
        qDebug()<<"main.cpp activeMainwindow value:"<<p->get("mode").toString();
        p->set("mode","stat4");
        qDebug()<<"main.cpp after set value:"<<p->get("mode").toString();
    }
}
int main(int argc, char *argv[])
{
    QString parm = "--";
    if(argc == 2){
        parm = argv[1];
    }

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QtSingleApplication a(argc,argv);
    //    TODO: 整合qt的标准翻译和自行定制的qm翻译
    //    标准对话框汉化(QT标准翻译
    // 应用内翻译
    //    TODO: 整合qt的标准翻译和自行定制的qm翻译
    //    标准对话框汉化(QT标准翻译
#ifndef QT_NO_TRANSLATION
    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *translator = new QTranslator();
    if (translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        a.installTranslator(translator);
    else
        qDebug() << "Failed to load Chinese translation file.";
#endif
    // 应用内翻译
    QTranslator app_trans;
    QString locale = QLocale::system().name();
    QString trans_path;

    QTranslator trans;
    if(trans.load(":/translations/gui_" + locale + ".qm")){
            a.installTranslator(&trans);
    }

    if(QDir("/opt/apps/top.openkylin.kylin-usb-creator/files/share/kylin-usb-creator/src/translations").exists()){
        trans_path = "/opt/apps/top.openkylin.kylin-usb-creator/files/share/kylin-usb-creator/translations";
    }else{
        trans_path = qApp->applicationDirPath() + ":/src/translations";
    }
    if(locale == "zh_CN"){
        if(!app_trans.load(":/src/translations/kylin-usb-creator_zh_CN.qm"))
        {
            qDebug()<<"Load translation file kylin-usb-creator_zh_CN.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale == "bo_CN"){
        if(!app_trans.load(":/src/translations/kylin-usb-creator_bo_CN.qm"))
        {
            qDebug()<<"Load translation file kylin-usb-creator_bo_CN.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale =="mn_MN"){
        if(!app_trans.load(":/src/translations/kylin-usb-creator_mn.qm"))
        {
            qDebug()<<"Load translation file kylin-usb-creator_mn.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale =="zh_HK"){
        if(!app_trans.load(":/src/translations/kylin-usb-creator_zh_HK.qm"))
        {
            qDebug()<<"Load translation file kylin-usb-creator_zh_HK.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale == "ug_CN"){
        if(!app_trans.load(":/src/translations/kylin-usb-creator_ug.qm"))
        {
            qDebug()<<"Load translation file kylin-usb-creator_ug.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale == "vi_VN"){
        if(!app_trans.load(":/src/translations/kylin-usb-creator_ms.qm"))
        {
            qDebug()<<"Load translation file kylin-usb-creator_ug.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale == "th_TH"){
        if(!app_trans.load(":/src/translations/kylin-usb-creator_vi.qm"))
        {
            qDebug()<<"Load translation file kylin-usb-creator_ug.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }
    else if(locale == "ms_MY"){
        if(!app_trans.load(":/src/translations/kylin-usb-creator_th.qm"))
        {
            qDebug()<<"Load translation file kylin-usb-creator_ug.qm error";
        }else{
            a.installTranslator(&app_trans);
        }
    }


    if(a.isRunning()){
        a.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1):"");
        qWarning()<<"kylin-usb-creator is already running";
        return EXIT_SUCCESS;
    }else {
        MainWindow w(NULL,parm);
        w.handleIconClickedSub("");
        w.setFixedSize(472,648);
        QScreen *primaryScreen = QGuiApplication::primaryScreen();
        int x = primaryScreen->geometry().x() + (primaryScreen->geometry().width()-w.width()) / 2;
        int y = primaryScreen->geometry().y() + (primaryScreen->geometry().height()-w.height()) / 2;
        w.move(x,y);  
        w.show();
        initUkuiLog4qt("kylin-usb-creator");
        a.setApplicationName(QObject::tr("kylin-usb-creator"));
        qApp->setProperty("noChangeSystemFontSize",true);
        qApp->setWindowIcon(QIcon::fromTheme("kylin-usb-creator"));

        QTimer::singleShot(50, [&a, &w,parm]()
        {
            UkuiWindowHelper *winHelper = new UkuiWindowHelper(&w);
            if(Global::isWayland){
                winHelper->removeTitleBar();
            }else{
                // 去除窗管标题栏，传入参数为QWidget *
                kdk::UkuiStyleHelper::self()->removeHeader(&w);
            }

            w.reShow(parm);
        });


        if(Global::isWayland){
        // center window
        int sw=QGuiApplication::primaryScreen()->availableGeometry().width();
        int sh=QGuiApplication::primaryScreen()->availableGeometry().height();
        w.setGeometry(QRect((sw-w.width())/2,(sh-w.height())/2,w.width(),w.height()));
        //kdk::WindowManager::setGeometry(w.windowHandle(),QRect((sw-w.width())/2,(sh-w.height())/2,w.width(),w.height()));
        }
//        QObject::connect(&a,SIGNAL(messageReceived(const QString&)),&w,SLOT(handleIconClickedSub("")));
//        return a.exec();
        kdk::WindowId wayland_mainwindow_id;
        QObject::connect(kdk::WindowManager::self(),&kdk::WindowManager::windowAdded,[&wayland_mainwindow_id](const kdk::WindowId &window_id)
        {
            qDebug()<<"pid is "<< (int)kdk::WindowManager::getPid(window_id);
            qDebug()<<"pid2 is " << getpid();
            if(getpid() == (int)kdk::WindowManager::getPid(window_id))
            {
              wayland_mainwindow_id = window_id;
            }
        });
        QObject::connect(&a,&QtSingleApplication::messageReceived,[&w,&wayland_mainwindow_id](QString message)
        {
            kdk::WindowManager::activateWindow(wayland_mainwindow_id);
        });

        return a.exec();
    }
    return a.exec();
}
