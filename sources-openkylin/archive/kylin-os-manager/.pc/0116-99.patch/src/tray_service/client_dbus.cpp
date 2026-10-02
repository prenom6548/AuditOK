#include <time.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QMap>
#include <QString>
#include <QDebug>

#include "client_dbus.h"

#define SERVICE_NAME "com.kylin.kom.tray"
#define OBJECT_PATH "/com/kylin/kom/tray"
#define INTERFACE_NAME "com.kylin.kom.tray"

namespace tray {

DBusClient::DBusClient() {
    // nothing to do
}

DBusClient::~DBusClient() {
    // nothing to do
}

bool DBusClient::startTray(QString name) {
    auto message = QDBusMessage::createMethodCall(SERVICE_NAME, OBJECT_PATH, INTERFACE_NAME, "start");
    message << name;

    auto response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ErrorMessage) {
        std::cout << "ERR: start tray " << name.toStdString() << " fail." << std::endl;
        return false;
    }

    auto reply = response.arguments();
    if (reply.size() < 2) {
        std::cout << "ERR: start tray " << name.toStdString() << " fail." << std::endl;
        return false;
    }

    if (!reply.at(0).toBool()) {
        std::cout << "ERR: start tray" << name.toStdString() << " fail. " << reply.at(1).toString().toStdString() << std::endl;
        return false;
    }

    return true;
}

bool DBusClient::stopTray(QString name) {
    auto message = QDBusMessage::createMethodCall(SERVICE_NAME, OBJECT_PATH, INTERFACE_NAME, "stop");
    message << name;

    auto response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ErrorMessage) {
        std::cout << "ERR: stop tray " << name.toStdString() << " fail." << std::endl;
        return false;
    }

    auto reply = response.arguments();
    if (reply.size() < 2) {
        std::cout << "ERR: stop tray " << name.toStdString() << " fail." << std::endl;
        return false;
    }

    if (!reply.at(0).toBool()) {
        std::cout << "ERR: stop tray" << name.toStdString() << " fail. " << reply.at(1).toString().toStdString() << std::endl;
        return false;
    }

    return true;
}

bool DBusClient::trayStatus(QString name) {
    auto message = QDBusMessage::createMethodCall(SERVICE_NAME, OBJECT_PATH, INTERFACE_NAME, "status");
    message << name;

    auto response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ErrorMessage) {
        std::cout << "ERR: obtain tray status " << name.toStdString() << " fail." << std::endl;
        return false;
    }

    auto reply = response.arguments();
    if (reply.isEmpty()) {
        std::cout << "ERR: obtain tray status " << name.toStdString() << " fail." << std::endl;
        return false;
    }

    QMap<QString, QMap<QString, QVariant>> trayStatus;
    auto dbusArgument = reply.first().value<QDBusArgument>();
    dbusArgument >> trayStatus;

    QMap<QString, QMap<QString, QVariant>>::iterator iter = trayStatus.begin();
    for (; iter != trayStatus.end(); iter++) {
        unsigned long long int uptime = iter.value().value("uptime").toULongLong();
        bool isActive = iter.value().value("active").toBool();
        if (isActive) {
            std::cout << "[" << iter.key().toStdString() << "] \033[32mRUNNING\033[0m \tuptime: " << uptimeString(uptime) << std::endl;
        } else {
            std::cout << "[" << iter.key().toStdString() << "] \033[31mSTOPPINNG\033[0m" << std::endl;
        }
    }

    return true;
}

std::string DBusClient::uptimeString(time_t sec)
{
    tm *p = gmtime(&sec);

    std::ostringstream oss;
    if (p->tm_yday == 1) {
        oss << p->tm_yday << "day ";
    } else if (p->tm_yday > 1) {
        oss << p->tm_yday << "days ";
    }
    oss << std::setw(2) << std::setfill('0') << p->tm_hour << ":";
    oss << std::setw(2) << std::setfill('0') << p->tm_min << ":";
    oss << std::setw(2) << std::setfill('0') << p->tm_sec;

    return oss.str();
}

}
