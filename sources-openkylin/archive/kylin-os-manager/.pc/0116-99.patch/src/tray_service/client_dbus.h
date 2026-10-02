#ifndef KYLIN_OS_MANAGER_SRC_TRAY_SERVICE_CLIENT_DBUS_H
#define KYLIN_OS_MANAGER_SRC_TRAY_SERVICE_CLIENT_DBUS_H

#include <time.h>
#include <string>

namespace tray {

class DBusClient {
public:
    DBusClient();
    ~DBusClient();

    bool startTray(QString name);
    bool stopTray(QString name);
    bool trayStatus(QString name);

private:
    std::string uptimeString(time_t sec);

};

}

#endif
