#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include <QDebug>
#include <QObject>
#include <singleapplication.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include "log.hpp"
#include "asrdbusservice.h"

using namespace kdk::kabase;

const QString VERSION = "1.0.0";

int main(int argc, char *argv[])
{
    qInstallMessageHandler(kdk::kabase::Log::logOutput);

// 修改分辨率的显示
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    qDebug() << "setAttribute success";
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    qDebug() << "setHighDpiScaleFactorRoundingPolicy success";
#endif

    kdk::QtSingleApplication app(argc, argv);
    if (app.isRunning()) {
        qInfo() << "kylin asr assistant is already running !";
        app.sendMessage("running", 4000);
        return 0;
    }
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationVersion(VERSION);
    app.setWindowIcon(QIcon::fromTheme("ukui-kylin-asrassistant-symbolic"));

    QString tranPath("/usr/share/kylin-asrassistant/translations/");
    QTranslator tran;
    if (!tran.load(QLocale(), QString("kylin-asrassistant"), QString("_"), tranPath)) {
        qWarning() << "man load app translation file fail";
    } else {
        app.installTranslator(&tran);
    }

    /* 加载Qt翻译文件 */
    QTranslator trans_qt;
    QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if (!trans_qt.load(QLocale(), "qt", "_", qtTransPath)) {
        qWarning() << "main Load qt translations file" << QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_qt);
    }

    // 加载SDK的翻译
    QString locale = QLocale::system().name();
    QTranslator sdk_trans;
    if (!sdk_trans.load(":/translations/gui_" + locale + ".qm")) {
        qWarning() << "Load sdk translation file failed!";
    } else {
        app.installTranslator(&sdk_trans);
    }

    MainWindow::getInstance();
    kdk::UkuiStyleHelper::self()->removeHeader(MainWindow::getInstance());
    MainWindow::getInstance()->setWindowTitle(QObject::tr("Asr assistant"));
    app.setActivationWindow(MainWindow::getInstance());
    if (argc == 2 && (QLatin1String(argv[1]) == "--show" || QLatin1String(argv[1]) == "-s")) {
        MainWindow::getInstance()->showView();
    }
    QObject::connect(&app, &kdk::QtSingleApplication::messageReceived,
                     [=]() { MainWindow::getInstance()->showView(); });

    AsrDbusService dbusService;

    return app.exec();
}
