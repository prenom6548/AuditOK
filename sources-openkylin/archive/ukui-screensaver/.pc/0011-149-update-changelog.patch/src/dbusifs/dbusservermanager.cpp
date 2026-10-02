#include "dbusservermanager.h"
#include "../common/definetypes.h"
#include <QDebug>
#include <QDBusReply>

DBusServerManager *DBusServerManager::m_instance = nullptr;
DBusServerManager::MemCollector DBusServerManager::m_memCollector;
DBusServerManager::DBusServerManager(QObject *parent) : QObject{ parent }
{
    init();
}

DBusServerManager *DBusServerManager::instance(QObject *parent)
{
    if (!m_instance) {
        m_instance = new DBusServerManager(parent);
    }
    return m_instance;
}

void DBusServerManager::init()
{
    if (!m_dbusService) {
        m_dbusService
            = new QDBusInterface(FD_DBUS_SERVICE, FD_DBUS_PATH, FD_DBUS_INTERFACE, QDBusConnection::sessionBus());
        connect(
            m_dbusService,
            SIGNAL(NameOwnerChanged(QString, QString, QString)),
            this,
            SLOT(onDBusNameOwnerChanged(QString, QString, QString)));
    }
}

void DBusServerManager::onDBusNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner);
    //    qDebug() << "service status changed:" << name
    //             << (newOwner.isEmpty() ? "inactivate" : "activate");
    Q_EMIT dbusServerActiveChanged(name, !newOwner.isEmpty());
}

bool DBusServerManager::serviceExists(const QString &strService)
{
    if (!m_dbusService) {
        return false;
    }
    QDBusReply<bool> reply = m_dbusService->call("NameHasOwner", strService);
    if (!reply.isValid()) {
        qWarning() << "check service exists error:" << reply.error();
        return false;
    }
    return reply.value();
}
