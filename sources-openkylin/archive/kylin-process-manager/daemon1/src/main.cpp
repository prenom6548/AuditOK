#include "loghelper.h"
#include "resourcemanager.h"
#include "resourcemanagerservice.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ResourceManager resourceManager;
    QDBusConnection connection = QDBusConnection::systemBus();
    if (!connection.registerService("com.kylin.ProcessResourceManagerDaemon") ||
            !connection.registerObject("/com/kylin/ProcessResourceManagerDaemon", &resourceManager)) 
    {
        PRINT_NOTICE("kylin-process-resource-manager-daemon : register dbus service failed\n");
        return 0;
    }

    ResourceManagerService resourceManagerService(&resourceManager);

    return a.exec();
}
