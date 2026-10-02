#ifndef KYLIN_OS_MANAGER_SRC_PROCESS_MANAGER_SERVICE_DBUS_H
#define KYLIN_OS_MANAGER_SRC_PROCESS_MANAGER_SERVICE_DBUS_H

#include <QDBusAbstractAdaptor>
#include <QDBusMessage>
#include <QMap>
#include <QVariant>
#include <QString>
#include "daemon.h"

typedef QMap<QString, QVariant> ProcessStatus;
typedef QMap<QString, ProcessStatus> ProcessStatusMap;
Q_DECLARE_METATYPE(ProcessStatus)
Q_DECLARE_METATYPE(ProcessStatusMap)

namespace process_manager {

class DBusService: public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.kom.process.manager")

public:
    DBusService(QObject *obj, Daemon &daemon);
    ~DBusService();

private:
    Daemon &m_daemon;

public Q_SLOTS:
    void start(QString name, QDBusMessage message, bool &result, QString &errorMessage);
    void stop(QString name, QDBusMessage message, bool &result, QString &errorMessage);
    void restart(QString name, QDBusMessage message, bool &result, QString &errorMessage);
    void status(QString name, QDBusMessage message, ProcessStatusMap &status);
};

}

#endif
