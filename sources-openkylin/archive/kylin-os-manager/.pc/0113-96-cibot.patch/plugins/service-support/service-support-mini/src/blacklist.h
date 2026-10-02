#ifndef KYLIN_OS_MANAGER_PLUGINS_SERVICE_SUPPORT_SERVICE_SUPPORT_MINI_BLACKLIST_H
#define KYLIN_OS_MANAGER_PLUGINS_SERVICE_SUPPORT_SERVICE_SUPPORT_MINI_BLACKLIST_H

#include <QString>

class Blacklist {
public:
    static void Add(QString exec_name);
    static void Remove(QString exec_name);
    static bool Contain(QString exec_name);

private:
    static QString Path();
    static QString CurrentDataTime();
};

#endif
