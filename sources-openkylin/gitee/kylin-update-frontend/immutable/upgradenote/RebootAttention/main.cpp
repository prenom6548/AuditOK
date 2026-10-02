#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator ;
    qDebug()<<"the system.name is "<<QLocale::system().name();
    translator.load("/usr/share/upgradenote/translations/" + QLocale::system().name() +".qm");
    a.installTranslator(&translator);
    MainWindow w;
//    w.show();
    return a.exec();
}
