#include <time.h>
#include <iostream>
#include <iomanip>
#include <ios>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDebug>
#include <QMap>
#include <QVariant>
#include <QDBusArgument>
#include <client_dbus.h>

#define SERVICE_NAME "com.kylin.kom.process.manager"
#define OBJECT_PATH "/com/kylin/kom/process/manager"
#define INTERFACE_NAME "com.kylin.kom.process.manager"

namespace process_manager {

DBusClient::DBusClient() {

}

DBusClient::~DBusClient() {

}

void DBusClient::managerProcess(Operate operate, const QString &name) {
    QString methodName;
    switch (operate) {
    case Operate::START:
        methodName = "start";
        break;
    case Operate::RESTART:
        methodName = "restart";
        break;
    case Operate::STOP:
        methodName = "stop";
        break;
    }

    auto message = QDBusMessage::createMethodCall(SERVICE_NAME, OBJECT_PATH, INTERFACE_NAME, methodName);
    message << name;

    auto response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ErrorMessage) {
        qCritical() << name << " fail to start.";
        return;
    }
    auto reply = response.arguments();
    if (reply.size() < 2) {
        qCritical() << name << " fail to start.";
        return;
    }
    if (!reply.first().toBool())
        qCritical() << name << " fail to start." << " ErrorMessage: " << reply.at(1).toString();
}

void DBusClient::status(const QString &name) {
    auto message = QDBusMessage::createMethodCall(SERVICE_NAME, OBJECT_PATH, INTERFACE_NAME, "status");
    message << name;

    auto response= QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "Failed to obtain message.";
        return;
    }
    auto reply = response.arguments();
    if (reply.isEmpty()) {
        qCritical() << "Failed to obtain message";
        return;
    }

    QMap<QString, QMap<QString, QVariant>> processStatusMap;
    reply.first().value<QDBusArgument>() >> processStatusMap;

    for (auto iter = processStatusMap.constBegin(); iter != processStatusMap.end(); iter++) {
        auto processName = iter.key();
        auto processStatus = iter.value();
        auto pid = processStatus.value("Pid").toUInt();
        auto active = processStatus.value("Active").toBool();
        auto startTime = processStatus.value("StartTime").toULongLong();

        if (active) {
            std::cout << std::right;
            std::cout << "[" << processName.toStdString() << "]\n"
                      << std::setw(16) << "Active: " << "\033[32mRUNNING\033[0m\n"
                      << std::setw(16) << "StartTime: " << formatTime(startTime) << "\n"
                      << std::setw(16) << "Pid: " << pid << "\n"
                      << std::endl;
        } else {
            std::cout << std::right;
            std::cout << "[" << processName.toStdString() << "]\n"
                      << std::setw(16) << "Active: " << "\033[31mSTOPPINNG\033[0m\n"
                      << std::endl;
        }
    }
}

std::string DBusClient::formatTime(time_t time) {
    struct tm t;
    localtime_r(&time, &t);

    char tmp[32];
    memset(tmp, '\0', sizeof(tmp));
    strftime(tmp, sizeof(tmp), "%a %Y-%m-%d %H:%M:%S", &t);

    return std::string {tmp};
}

}
