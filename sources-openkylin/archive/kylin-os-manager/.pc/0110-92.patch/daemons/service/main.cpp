#include <QCoreApplication>
#include "dbus.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    dbus::DbusManager dbusManager;

    return app.exec();
}