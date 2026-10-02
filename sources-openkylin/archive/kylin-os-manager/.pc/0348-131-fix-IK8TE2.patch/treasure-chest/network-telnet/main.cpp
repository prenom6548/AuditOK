#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDebug>
#include <singleapplication.h>
#include "kom-utils.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    kdk::QtSingleApplication app(argc, argv);
    if (app.isRunning()) {
        app.sendMessage("running", 4000);
        return 0;
    }

	QTranslator translator;
	if (translator.load(QLocale(), "komt-network-telnet", "_", "/usr/share/kylin-os-manager/treasure-chest/kylin-telnet/translations"))
		QApplication::installTranslator(&translator);
	else
		qCritical() << "Load translation fail";

	QTranslator qtTranslator;
	if (qtTranslator.load(QLocale(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
		QApplication::installTranslator(&qtTranslator);
	else
		qCritical() << "Load Qt translation file fail";

    QTranslator sdk_translator;
    if (sdk_translator.load(":/translations/gui_" + QLocale::system().name() + ".qm")) {
        QApplication::installTranslator(&sdk_translator);
    } else {
        qCritical() << "Load sdk translation file fail";
    }

    kom::KomUtils::SetWaylandActivationWindow(app);

	networktelnet::MainWindow mainwindow;
    app.setActivationWindow(&mainwindow);
	mainwindow.show();

	return app.exec();
}
