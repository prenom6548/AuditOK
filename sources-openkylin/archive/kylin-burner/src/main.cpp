/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "src/view/mainwindow.h"
#include "src/frame/configuration.h"
#include "src/frame/logger.h"
#include "src/model/logicmain.h"
#include "src/model/menufactory.h"
#include "src/model/logiccopydisc.h"
#include "src/view/modaldialog.h"
#include "src/view/aboutdialog.h"
#include "src/view/execdialog.h"
#include "src/view/filefilterrules.h"
#include "src/view/viewfilterfiles.h"
#include "src/view/tip_burnresult.h"
#include "src/po/translation.h"
#include "src/common/utils.h"
#include "src/common/trackingmanager.h"

#include <QScreen>
#include <QObject>
#include <QApplication>
#include <QDesktopWidget>
#include <QCommandLineParser>
#include <ukui-log4qt.h>
#include <QTranslator>
#include <QLibraryInfo>
#include <kwidget.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include <windowmanager/windowmanager.h>
#include <kysdk/applications/singleapplication.h>
#include <QTranslator>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDir>
#include <QElapsedTimer>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include "src/view/splashwindow.h"

#ifndef UNIT_TEST
using namespace kdk;

const QString LOCAL_SERVER_NAME = "Kylin-burner-app";
QLocalServer* gLocalServer = nullptr;

bool createLocalServer(QApplication& app) {
    gLocalServer = new QLocalServer(&app);
    // 若服务器已存在（进程崩溃未清理） 先移除
    if (QLocalServer::removeServer(LOCAL_SERVER_NAME)) {
        qWarning() << "清理残留的服务器实例";
    }
    // 监听套接字
    if (!gLocalServer->listen(LOCAL_SERVER_NAME)) {
        qCritical() << "创建本地服务器失败：" << gLocalServer->errorString();
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    //埋点启动上传
    TrackingManager::instance();

    // locale
#if KAIMING_BUILD
    _KYBi18nLoad("kylin-burner", "/opt/apps/top.openkylin.kylin-burner/files/share/locale/");
#else
    _KYBi18nLoad("kylin-burner", "/usr/share/locale/");
#endif
    _KYBi18nDomain("kylin-burner");

    /* 适配4K屏以及分数缩放 */
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    kdk::QtSingleApplication a(argc, argv);
    a.setWindowIcon(QIcon::fromTheme("burner"));
    a.setApplicationDisplayName(_("Burner"));


    QLocalSocket socket;
    socket.connectToServer(LOCAL_SERVER_NAME);
    if (socket.waitForConnected(300)) {
        qDebug() << "kylin-burner is running";
        return 0;
    }

    if (!createLocalServer(a)) {
        qDebug() << "kylin-burner create local server failed";
        return -1;
    }


#ifndef QT_NO_TRANSLATION
    // QT自身标准的翻译
    QString locale = QLocale::system().name();
    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += locale;
    QTranslator *selfTransOfQt = new QTranslator();
    if (selfTransOfQt->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        a.installTranslator(selfTransOfQt);
    else
        qDebug() << "load qt translator file failed!";
#endif

    // 加载sdk控件翻译
    QTranslator trans;
    if (locale == "zh_CN") {
        if (trans.load(":/translations/gui_zh_CN.qm")) {
            a.installTranslator(&trans);
        }
    } else if (locale == "bo_CN") {
        if (trans.load(":/translations/gui_bo_CN.qm")) {
            a.installTranslator(&trans);
        }
    } else if (locale == "mn_MN") {
        if (trans.load(":/translations/gui_mn_MN.qm")) {
            a.installTranslator(&trans);
        }
    } else if (locale == "zh_HK") {
        if (trans.load(":/translations/gui_zh_HK.qm")) {
            a.installTranslator(&trans);
        }
    } else {
        QString sdkTranslatePath = QString(":/translations/gui_%1.qm").arg(locale);
        if (trans.load(sdkTranslatePath)) {
            a.installTranslator(&trans);
        }
    }

    //在程序启动时调用，不需要传入参数
    QCommandLineParser *parser;
    parser = new QCommandLineParser;
    parser->addPositionalArgument( "urls", _("file(s) to open"), "[urls...]" );
    parser->addOption(QCommandLineOption("d", _("Running as debug model.")));
    parser->process(a);
    if (parser->isSet("d"))
        Configuration::instance()->set("logger","level", 4);
    else
        Configuration::instance()->set("logger","level", 1);

    //check already running
    QDBusInterface interface("com.kylin.burner.manager",
                              "/com/kylin/burner/manager",
                              "com.kylin.burner.manager",
                              QDBusConnection::sessionBus());

    QDBusReply<int> reply = interface.call("slotsReShowMainWindow", parser->positionalArguments());
    if (reply.isValid()) {
        switch(reply.value()) {
        case 0:
            qDebug()<<"The kylin-burner process is running, 0 argument, active mainwindow";
            break;
        case 1:
            qDebug()<<"The kylin-burner process is running, 1 argumentactive mainwindow and swith burn image view";
            break;
        default:
            qDebug()<<"The kylin-burner process is running";
            break;
        }
        exit(0);
    }

    Utils::initSystemInfo();

    //EverTime Burner Start, init [DataSetting]--Duplicate Data/Add Data/Delete Data/clean Data
    //it can reduce some error when user some operation
    Configuration::instance()->set("DataSetting", "Duplicate Data", 0);
    Configuration::instance()->set("DataSetting", "Add Data", false);
    Configuration::instance()->set("DataSetting", "Delete Data", false);
    Configuration::instance()->set("DataSetting", "Clean Data", true);

    Configuration::instance()->set("FilterFile","hidden",false);
    Configuration::instance()->set("FilterFile","broken",false);
    Configuration::instance()->set("FilterFile","Repleace",false);

    QString logFilePath = QDir::homePath().append("/.log/kylin-burner/");
    CFG->set("logger","path",logFilePath);
    LOGGER->init();

    MainWindow *mainWindow = MainWindow::Instance(parser->positionalArguments(), nullptr);
    a.setActivationWindow(mainWindow);

    QScreen *mainScreen = QGuiApplication::primaryScreen();
    if(mainScreen){
        int x = mainScreen->geometry().x() +
                (mainScreen->geometry().width() - mainWindow->width()) / 2;
        int y = mainScreen->geometry().y() +
                (mainScreen->geometry().height() - mainWindow->height()) / 2;
        mainWindow->move(x,y);
    }

    mainWindow->show();

    SplashWindow *splash = new SplashWindow(mainWindow);
    splash->show();
    a.processEvents(QEventLoop::AllEvents, 100);

    MenuFactory::instance();
    Tip_BurnResult::instance();
    ViewFilterFiles::instance();
    LogicMain::instance()->init();
    LogicCopyDisc::instance();
    ExecDialog::instance();

    splash->fadeOut();

    if(gLocalServer){
        gLocalServer->close();
        QLocalServer::removeServer(LOCAL_SERVER_NAME);
    }

    MODAL->setAttribute(Qt::WA_NativeWindow);
    MODAL->windowHandle()->setTransientParent(mainWindow->windowHandle());

    FileFilterRules::instance()->setAttribute(Qt::WA_NativeWindow);
    FileFilterRules::instance()->windowHandle()->setTransientParent(mainWindow->windowHandle());

    EXEC->setAttribute(Qt::WA_NativeWindow);
    EXEC->windowHandle()->setTransientParent(mainWindow->windowHandle());

    TIP_BURNRESULT->setAttribute(Qt::WA_NativeWindow);
    TIP_BURNRESULT->windowHandle()->setTransientParent(mainWindow->windowHandle());

    ViewFilterFiles::instance()->setAttribute(Qt::WA_NativeWindow);
    ViewFilterFiles::instance()->windowHandle()->setTransientParent(mainWindow->windowHandle());

    LogicMain::instance()->initISO();

    return a.exec();
}
#endif
