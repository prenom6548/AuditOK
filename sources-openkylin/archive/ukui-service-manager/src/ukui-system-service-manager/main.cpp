#include <QCoreApplication>

#include <QDBusConnection>

#include <QDebug>

#include "systemserviceregister.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    bool serviceRegistered = QDBusConnection::systemBus().registerService("org.ukui.serviceManager");
    if (!serviceRegistered) {
        qDebug() << QDBusConnection::systemBus().lastError();
    }
    Q_ASSERT(serviceRegistered);

    SystemServiceRegister serviceRegister;
    QDBusConnection::systemBus().registerObject("/org/ukui/serviceManager", "org.ukui.serviceManager", &serviceRegister, QDBusConnection::ExportAllSlots);

    return a.exec();
}
