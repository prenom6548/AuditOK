#include <QDBusMetaType>

#include "service_dbus.h"

#define TRAY_STATUS_KEY_UPTIME "uptime"
#define TRAY_STATUS_KEY_ACTIVE "active"

namespace tray {

DBusService::DBusService(QObject *obj, Daemon &daemon)
    : QDBusAbstractAdaptor(obj)
    , m_daemon(daemon)
{
    qDBusRegisterMetaType<TrayStatus>();
    qDBusRegisterMetaType<TrayStatusMap>();
}

DBusService::~DBusService() {

}

void DBusService::start(QString trayName, [[maybe_unused]]QDBusMessage message, bool &result, [[maybe_unused]]QString &errorMessage) {
    if (trayName.isEmpty()) {
        result = false;
        errorMessage = "tray name is empty";
        return;
    }

    auto reply = m_daemon.startTray(trayName.toStdString());
    result = reply.first;
    errorMessage = QString::fromStdString(reply.second);
}

void DBusService::stop(QString trayName, [[maybe_unused]]QDBusMessage message, bool &result, [[maybe_unused]]QString &errorMessage) {
    if (trayName.isEmpty()) {
        result = false;
        errorMessage = "tray name is empty";
        return;
    }

    auto reply = m_daemon.stopTray(trayName.toStdString());
    result = reply.first;
    errorMessage = QString::fromStdString(reply.second);
}

void DBusService::status(QString trayName, [[maybe_unused]]QDBusMessage message, TrayStatusMap &trayStatusMap) {
    if (trayName.isEmpty()) {
        auto trays = m_daemon.trays();
        for (const auto &[name, provider]: m_daemon.trayProviders()) {
            TrayStatus item;
            auto iter = trays.find(name);
            if (iter == trays.end()) {
                item.insert(TRAY_STATUS_KEY_ACTIVE, false);
                item.insert(TRAY_STATUS_KEY_UPTIME, 0);
            } else {
                item.insert(TRAY_STATUS_KEY_ACTIVE, true);
                item.insert(TRAY_STATUS_KEY_UPTIME, (unsigned long long int)iter->second->uptime());
            }
            trayStatusMap.insert(QString::fromStdString(name), item);
        }
    } else {
        auto providers = m_daemon.trayProviders();
        auto trays = m_daemon.trays();
        if (providers.find(trayName.toStdString()) != providers.end()) {
            TrayStatus item;
            auto iter = trays.find(trayName.toStdString());
            if (iter != trays.end()) {
                item.insert(TRAY_STATUS_KEY_ACTIVE, true);
                item.insert(TRAY_STATUS_KEY_UPTIME, (unsigned long long int)iter->second->uptime());
            } else {
                item.insert(TRAY_STATUS_KEY_ACTIVE, false);
                item.insert(TRAY_STATUS_KEY_UPTIME, 0);
            }
            trayStatusMap.insert(trayName, item);
        }
    }
}

}
