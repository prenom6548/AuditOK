#include <QDBusMetaType>
#include <QDebug>
#include "service_dbus.h"

#define PROCESS_STATUS_PID_KEY "Pid"
#define PROCESS_STATUS_ACTIVE_KEY "Active"
#define PROCESS_STATUS_START_TIME_KEY "StartTime"

namespace process_manager {

DBusService::DBusService(QObject *obj, process_manager::Daemon &daemon)
    : QDBusAbstractAdaptor(obj)
    , m_daemon(daemon)
{
    qDBusRegisterMetaType<ProcessStatus>();
    qDBusRegisterMetaType<ProcessStatusMap>();
}

DBusService::~DBusService() {
    // nothing to do
}

void DBusService::start(QString name, [[maybe_unused]] QDBusMessage message, bool &result, QString &errorMessage) {
    std::pair<bool, std::string> res = m_daemon.startProcess(name.toStdString());
    result = res.first;
    errorMessage = QString::fromStdString(res.second);
}

void DBusService::stop(QString name, [[maybe_unused]] QDBusMessage message, bool &result, QString &errorMessage) {
    std::pair<bool, std::string> res = m_daemon.stopProcess(name.toStdString());
    result = res.first;
    errorMessage = QString::fromStdString(res.second);
}

void DBusService::restart(QString name, [[maybe_unused]] QDBusMessage message, bool &result, QString &errorMessage) {
    std::pair<bool, std::string> res = m_daemon.restart(name.toStdString());
    result = res.first;
    errorMessage = QString::fromStdString(res.second);
}

void DBusService::status(QString name, [[maybe_unused]] QDBusMessage message, ProcessStatusMap &status) {
    auto processes = m_daemon.processes();
    if (name.isEmpty()) {
        // 返回全部进程的状态
        for (const auto &[processName, processObj]: processes) {
            ProcessStatus itemStatus;
            itemStatus.insert(PROCESS_STATUS_ACTIVE_KEY, processObj->isRunning());
            itemStatus.insert(PROCESS_STATUS_PID_KEY, processObj->pid());
            itemStatus.insert(PROCESS_STATUS_START_TIME_KEY, (unsigned long long )processObj->startTime());
            status.insert(QString::fromStdString(processName), itemStatus);
        }
    } else {
        auto iter = processes.find(name.toStdString());
        if (iter != processes.end()) {
            ProcessStatus itemStatus;
            itemStatus.insert(PROCESS_STATUS_ACTIVE_KEY, iter->second->isRunning());
            itemStatus.insert(PROCESS_STATUS_PID_KEY, iter->second->pid());
            itemStatus.insert(PROCESS_STATUS_START_TIME_KEY, (unsigned long long )iter->second->startTime());
            status.insert(QString::fromStdString(iter->first), itemStatus);
        }
    }
}

}
