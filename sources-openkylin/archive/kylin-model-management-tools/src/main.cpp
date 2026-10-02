#include "downloader.h"
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Downloader downloader;
    MainWindow w{&downloader};
    w.show();
    return a.exec();
}
