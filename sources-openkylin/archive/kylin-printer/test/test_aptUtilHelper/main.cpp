#include "ukui_apt.h"
#include <QEventLoop>
#include <QCoreApplication>

void apt_test()
{
    bool install = false;

    AptUtilHelper* aptHelper = new AptUtilHelper(DeviceInformation());
    QEventLoop eventloop;

    QObject::connect(aptHelper, &AptUtilHelper::succeed, [&eventloop, &install]() {
        // qDebug() << devInfo.name <<"`s package: Install succeed";
        install = true;
        eventloop.quit();
    });
    QObject::connect(aptHelper, &AptUtilHelper::failed, [&eventloop, &install]() {
        // qDebug() << devInfo.name <<"`s package: Install failed";
        install = false;
        eventloop.quit();
    });
    aptHelper->startWorker();
    eventloop.exec();
}

void deb_test()
{
    bool install = false;

    // DebUtilHelper* aptHelper = new DebUtilHelper("/home/kylin/upgrade_hub/kylin-update-frontend/kylin-update-frontend_1.0.0.36kord_amd64.deb");
    DebUtilHelper* aptHelper = new DebUtilHelper("/home/kylin/upgrade_hub/kylin-system-updater/kylin-calculator_1.1.0~rc12_amd64.deb");
    QEventLoop eventloop;

    QObject::connect(aptHelper, &DebUtilHelper::succeed, [&eventloop, &install]() {
        // qDebug() << devInfo.name <<"`s package: Install succeed";
        install = true;
        eventloop.quit();
    });
    QObject::connect(aptHelper, &DebUtilHelper::failed, [&eventloop, &install]() {
        // qDebug() << devInfo.name <<"`s package: Install failed";
        install = false;
        eventloop.quit();
    });
    aptHelper->startWorker();
    eventloop.exec();
}


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    // apt_test();
    deb_test();
    app.exec();
}
