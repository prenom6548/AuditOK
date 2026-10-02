#include <QLocale>
#include <QLibraryInfo>
#include <QTranslator>
#include <QDebug>
#include <singleapplication.h>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    kdk::QtSingleApplication app(argc, argv);
    if (app.isRunning()) {
        qInfo() << "kylin tcpdump tool is already running";
        app.sendMessage("is running");
        return 0;
    }

    // 加载翻译文件
    QTranslator qt_translator;
    if (qt_translator.load(QLocale(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qt_translator);
    } else {
        qCritical() << "Load Qt translation file fail.";
    }
    QTranslator app_translator;
    if (app_translator.load(QLocale(), "komt-packet-capture", "_",
                            "/usr/share/kylin-os-manager/treasure-chest/kylin-tcpdump/translations/")) {
        app.installTranslator(&app_translator);
    } else {
        qCritical() << "Load app translation file fail.";
    }
    QTranslator sdk_translator;
    if (sdk_translator.load(":/translations/gui_" + QLocale::system().name() + ".qm")) {
        QApplication::installTranslator(&sdk_translator);
    } else {
        qCritical() << "Load sdk translation file fail";
    }

    MainWindow main_window;
    app.setActivationWindow(&main_window);
    main_window.show();

    app.exec();
}
