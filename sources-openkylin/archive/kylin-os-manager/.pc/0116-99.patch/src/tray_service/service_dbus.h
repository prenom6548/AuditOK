#ifndef KYLIN_OS_MANAGER_SRC_TRAY_CESERVICE_SERVI_DBUS_H
#define KYLIN_OS_MANAGER_SRC_TRAY_SERVICE_SERVICE_DBUS_H

#include <QDBusAbstractAdaptor>
#include <QDBusMessage>
#include <QString>
#include <QMap>
#include <QVariant>

#include "daemon.h"

typedef QMap<QString, QVariant> TrayStatus;
typedef QMap<QString, TrayStatus> TrayStatusMap;
Q_DECLARE_METATYPE(TrayStatus)
Q_DECLARE_METATYPE(TrayStatusMap);

namespace tray {

class DBusService : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.kom.tray")

public:
    DBusService(QObject *obj, Daemon &daemon);
    ~DBusService();

private:
    Daemon &m_daemon;

public Q_SLOTS:
    void start(QString trayName, QDBusMessage message, bool &result, QString &errorMessage);
    void stop(QString trayName, QDBusMessage message, bool &result, QString &errorMessage);
    void status(QString trayName, QDBusMessage message, TrayStatusMap &trayStatusMap);
};

}

#endif
