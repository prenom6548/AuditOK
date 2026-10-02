#ifndef KYLIN_OS_MANAGER_SRC_PROCESS_MANAGER_CLIENT_DBUS_H
#define KYLIN_OS_MANAGER_SRC_PROCESS_MANAGER_CLIENT_DBUS_H

#include <QString>

namespace process_manager {

class DBusClient {
public:
    DBusClient();
    ~DBusClient();

    enum Operate {
        START,
        STOP,
        RESTART
    };

    void managerProcess(Operate operate, const QString &name);
    void status(const QString &name);

private:
    std::string formatTime(time_t time);
};

}

#endif
