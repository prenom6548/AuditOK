#include "dbusconnect.h"

dbusconnect* dbusconnect::updateMutual = nullptr;
using namespace std;

dbusconnect* dbusconnect::getInstance(QObject *parent)
{
    static QMutex mutex;
    if(nullptr == updateMutual)
    {
        QMutexLocker locker(&mutex);
        updateMutual = new dbusconnect(parent);
    }
    return updateMutual;
}

dbusconnect::dbusconnect(QObject *parent) : QObject(parent)
{
//    qRegisterMetaType<AppMsg>("AppMsg"); //注册信号槽类型
//    qRegisterMetaType<AppAllMsg>("AppAllMsg"); //注册信号槽类型
    interface = new QDBusInterface(UPGRADE_SERVICE,
                                   UPGRADE_PATH,
                                   UPGRADE_INTERFACE,
                                   QDBusConnection::systemBus());
    qDebug() << "interface 初始链接";
}

dbusconnect::~dbusconnect()
{
//    qDebug() << "dbus quit------>";
//    cleanUpdateList();
//    setImportantStatus(false);
//    fileUnLock();
}

