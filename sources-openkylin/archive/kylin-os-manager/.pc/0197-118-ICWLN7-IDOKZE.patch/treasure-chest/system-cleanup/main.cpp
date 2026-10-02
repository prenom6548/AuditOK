#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDebug>
#include <singleapplication.h>
#include <log.hpp>
#include "singleapplication.h"
#include "mainwindow.h"
#include "kom-utils.h"

int main(int argc, char *argv[])
{
#ifndef DEBUG_MODE
    qInstallMessageHandler(kom::KomUtils::LogOutput);
#endif
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
        app.sendMessage("running", 4000);
        return 0;
    }

    QTranslator translator;
    if (translator.load(QLocale(), "komt-system-cleanup", "_",
                        "/usr/share/kylin-os-manager/treasure-chest/kylin-system-cleanup/translations"))
        QApplication::installTranslator(&translator);
    else
        qCritical() << "****** komt-network-route-track ******\t"
                    << "load translation fail !";

    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(&qtTranslator);
    else
        qCritical() << "****** komt-network-route-track ******\t"
                    << "load Qt translation fail !";

    QTranslator sdk_translator;
    if (sdk_translator.load(":/translations/gui_" + QLocale::system().name() + ".qm")) {
        QApplication::installTranslator(&sdk_translator);
    } else {
        qCritical() << "Load sdk translation file fail";
    }
    systemcleanup::MainWindow mainwindow;
    app.setActivationWindow(&mainwindow);
    mainwindow.show();

    return app.exec();
}
