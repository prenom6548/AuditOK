#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDebug>
#include <QDesktopWidget>
#include <singleapplication.h>
#include "kom-utils.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    kdk::QtSingleApplication app(argc, argv);
    if (app.isRunning()) {
        qInfo() << "kylin source update tool is already running";
        app.sendMessage("is running");
        return 0;
    }

    bool ok;
    QTranslator qt_translator;
    ok = qt_translator.load(QLocale(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    if (ok) {
        app.installTranslator(&qt_translator);
    } else {
        qCritical() << "Load Qt translation file fail.";
    }
    QTranslator app_translator;
    ok = app_translator.load(QLocale(), "kylin-source-update-tool", "_",
                             "/usr/share/kylin-os-manager/treasure-chest/kylin-source-update-tool/translations/");
    if (ok) {
        app.installTranslator(&app_translator);
    } else {
        qCritical() << "Load app translation file fail.";
    }

    QTranslator sdk_translator;
    if (sdk_translator.load(":/translations/gui_" + QLocale::system().name() + ".qm")) {
        app.installTranslator(&sdk_translator);
    } else {
        qCritical() << "Load sdk translation file fail";
    }

    kom::KomUtils::SetWaylandActivationWindow(app);

    source_update::MainWindow mainwindow;
    app.setActivationWindow(&mainwindow);
    mainwindow.show();

    // 在屏幕中间显示主界面
    auto desktop_width = QApplication::desktop()->width();
    auto desktop_heigth = QApplication::desktop()->height();
    auto x = desktop_width / 2 - mainwindow.width() / 2;
    auto y = desktop_heigth / 2 - mainwindow.height() / 2;
    mainwindow.move(x, y);

    app.exec();
}
