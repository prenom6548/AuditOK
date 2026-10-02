#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>

#include "connectivitymanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("kylin connectivity");
    app.setApplicationName("kylin connectivity manager");

    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.isConnected()) {
        outLog("d-bus connection fail !");
        return -1;
    }

    if (!systemBus.registerService("com.kylin.connectivity.manager")) {
        outLog(QString("d-bus register service fail ! d-bus error : ") +
               QDBusError::errorString(systemBus.lastError().type()));
        return -1;
    }

    if (!systemBus.registerObject("/", new ConnectivityManager,
                                  QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        outLog(QString("d-bus register object fail ! d-bus error : ") +
               QDBusError::errorString(systemBus.lastError().type()));
        return -1;
    }

    return app.exec();
}