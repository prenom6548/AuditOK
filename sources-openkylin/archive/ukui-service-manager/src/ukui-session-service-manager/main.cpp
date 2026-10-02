#include <QCoreApplication>

#include <QDBusConnection>

#include "sessionserviceregister.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    bool serviceRegistered = QDBusConnection::sessionBus().registerService("org.ukui.serviceManager");
    Q_ASSERT(serviceRegistered);

    SessionServiceRegister serviceRegister;
    QDBusConnection::sessionBus().registerObject("/org/ukui/serviceManager", "org.ukui.serviceManager", &serviceRegister, QDBusConnection::ExportAllSlots);

    return a.exec();
}
