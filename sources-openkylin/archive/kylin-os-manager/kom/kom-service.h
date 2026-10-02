#ifndef KYLIN_OS_MANAGER_KOM_SERVICE_H
#define KYLIN_OS_MANAGER_KOM_SERVICE_H
#include <QStringList>
#include <QDBusContext>
#include <QDBusConnectionInterface>
namespace kom
{

class KomService
{
public:
    static bool checkEnviron(int pid);
    static const QStringList programWhiteList;
};

} // namespace kom

#endif
