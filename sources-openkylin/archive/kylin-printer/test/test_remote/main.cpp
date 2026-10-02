#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QThread>


#include <QMutex>
#include <QDateTime>
#include <QDir>
#include <QFile>

#include "remoteWidget.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    RemoteWidget test;

    test.show();


    return app.exec();
}
