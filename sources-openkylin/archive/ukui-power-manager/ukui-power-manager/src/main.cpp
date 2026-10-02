#include "powermanagercenter.h"

#include <QApplication>
#include <ukui-log4qt.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    initUkuiLog4qt("ukui-power-manager");
    QTranslator translator;
    if (translator.load(QLocale(), "ukui-power-manager", "_", QM_FILES_INSTALL_PATH)) {
        a.installTranslator(&translator);
    } else {
        qDebug() << "load ukui-power-manager-tray qm file error";
    }
    if (! QDBusConnection::sessionBus().registerService(QString("org.ukui.ukui-power-manager")))
    {
        qCritical() << "ukui-power-manager has been activated";
        return 1;
    } else {
        PowerManagerCenter powermanagercenter;
        return a.exec();
    }
}
