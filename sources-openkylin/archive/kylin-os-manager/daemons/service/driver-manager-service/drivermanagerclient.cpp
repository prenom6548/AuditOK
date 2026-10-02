#include "drivermanagerclient.h"
#include "getDriver.h"
#include "include/globalsignal.h"
#include "dbus_args.h"

DrivermanagerClient::DrivermanagerClient(QObject *parent) : QObject(parent) {
    initDrivermanagerClient();
}

void DrivermanagerClient::initDrivermanagerClient() {
    QDBusConnection::systemBus().connect(dbus::DBUS_SERVICENAME,
                                         dbus::DBUS_USB_HOT_INFO_TOOL,
                                         dbus::DBUS_USB_HOT_INFO_INTERFACE_TOOL, "installDriverSuccessSignal", this,
                                         SLOT(updateDevInfo(QString)));
}

void DrivermanagerClient::updateDevInfo(QString devString) {
    QString drivername = devString.split("=")[0];
    QString driverversion = devString.split("=")[1];
    for(int i = 0; i < DRIVER_GETTER->m_printerHardwares.length(); ++i){
        for (int j = 0; j < DRIVER_GETTER->m_printerHardwares[i].driverNameList.length(); ++j) {
            if (DRIVER_GETTER->m_printerHardwares[i].driverNameList[j].driverName.contains(drivername) && DRIVER_GETTER->m_printerHardwares[i].driverNameList[j].version.contains(driverversion)){
                DRIVER_GETTER->m_printerHardwares[i].driver_now = drivername;
                DRIVER_GETTER->m_printerHardwares[i].driver_version_now = driverversion;
                DRIVER_GETTER->m_printerHardwares[i].upgradeable = false;
            }
        }
    }

    for(int i = 0; i < DRIVER_GETTER->m_scannerHardwares.length(); ++i){
        for (int j = 0; j < DRIVER_GETTER->m_scannerHardwares[i].driverNameList.length(); ++j) {
            if (DRIVER_GETTER->m_scannerHardwares[i].driverNameList[j].driverName.contains(drivername) && DRIVER_GETTER->m_scannerHardwares[i].driverNameList[j].version.contains(driverversion)){
                DRIVER_GETTER->m_scannerHardwares[i].driver_now = drivername;
                DRIVER_GETTER->m_scannerHardwares[i].driver_version_now = driverversion;
                DRIVER_GETTER->m_scannerHardwares[i].upgradeable = false;
            }
        }
    }
    emit g_signal->installSuccessSignal();
}
