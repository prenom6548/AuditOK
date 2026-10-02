/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "systemddbusinterface.h"
#include "loghelper.h"
#include "misc.h"
#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDebug>

namespace
{

const char *login1_user_interface = "org.freedesktop.login1.User";
const char *login1_user_path = "/org/freedesktop/login1/user/self";
const char *login1_session_interface = "org.freedesktop.login1.Session";
const char *login1_session_path = "/org/freedesktop/login1/session/auto";

const char *systemd_service = "org.freedesktop.systemd1";
const char *systemd_path = "/org/freedesktop/systemd1";
const char *systemd_manager_inerface = "org.freedesktop.systemd1.Manager";

const char *login1_service = "org.freedesktop.login1";
const char *login1_path = "/org/freedesktop/login1";
const char *login1_manager_interface = "org.freedesktop.login1.Manager";
const char *dbus_properties_interface = "org.freedesktop.DBus.Properties";

struct UnitProcess
{
    QString unitName;
    int pid;
    QString cmdline;
};

const QDBusArgument &operator>>(const QDBusArgument &argument, UnitProcess &process)
{
    argument.beginStructure();

    argument >> process.unitName;
    argument >> process.pid;
    argument >> process.cmdline;

    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QList<UnitProcess> &processes)
{
    argument.beginArray();
    processes.clear();

    while (!argument.atEnd()) {
        UnitProcess process;
        argument >> process;
        processes.append(process);
    }

    argument.endArray();
    return argument;
}

std::vector<int> processIdsFromUnitProcesses(const QList<UnitProcess> &processes)
{
    std::vector<int> pids;
    for (const auto &process : processes) {
        pids.emplace_back(process.pid);
    }

    return pids;
}

QVariant propertyValueFromString(
    const QString &property, const QString &value)
{
    if (property == "CPUWeight") {
        return value.toULongLong();
    }

    if (property == "CPUQuotaPerSecUSec") {
        return value.isEmpty() ? UINT64_MAX : value.toULongLong();
    }

    return value;
}

}

SystemdDbusInterface::SystemdDbusInterface()
{
    registerDbusTypes();
}

std::string SystemdDbusInterface::currentUserUID() const
{
    return getCurrentLoginProperty("UID", login1_user_interface, login1_user_path);
}

std::string SystemdDbusInterface::currentActiveUID() const
{
    QDBusInterface loginManager(
        login1_service, login1_path, login1_manager_interface, QDBusConnection::systemBus());

    if (!loginManager.isValid()) {
        return "";
    }

    QDBusReply<QDBusArgument> reply = loginManager.call("ListSessions");
    if (!reply.isValid()) {
        return "";
    }

    std::unordered_map<QString, uint> sessionPaths;
    const QDBusArgument &argument = reply.value();
    argument.beginArray();
    while (!argument.atEnd()) {
        argument.beginStructure();
        QString sessionId;
        uint uid;
        QString userName;
        QString seatId;
        QDBusObjectPath path;
        argument >> sessionId >> uid >> userName >> seatId >> path;
        argument.endStructure();
        sessionPaths[path.path()] = uid;
    }
    argument.endArray();

    for (const auto &path : sessionPaths) {
        QDBusInterface sessionInterface(
            login1_service,
            path.first,
            login1_session_interface,
            QDBusConnection::systemBus());

        QVariant activeVariant = sessionInterface.property("Active");

        if (!activeVariant.isValid()) {
            continue;
        }

        bool isActive = activeVariant.toBool();
        if (isActive) {
            return std::to_string(path.second);
        }
    }
    return "";
}

std::string SystemdDbusInterface::currentUserSliceName() const
{
    return getCurrentLoginProperty("Slice", login1_user_interface, login1_user_path);
}

std::string SystemdDbusInterface::currentUserServiceName() const
{
    return getCurrentLoginProperty("Service", login1_user_interface, login1_user_path);
}

std::string SystemdDbusInterface::currentSessionScopeName() const
{
    return getCurrentLoginProperty("Scope", login1_session_interface, login1_session_path);
}

void SystemdDbusInterface::createPersistentProcessGroup(const std::string &groupName)
{
    asyncCallSystemdDbus(
        "StartUnit", {QString::fromStdString(groupName), "fail"},
        QDBusConnection::sessionBus());
}

void SystemdDbusInterface::createTransientProcessGroup(
    const std::string &name, std::vector<int> pids, const std::string &parentSlice)
{
    QList<uint> pidList = misc::stl2qt::intVector2QtUintList(pids);
    const QString scopeName = QString::fromStdString(name);
    const QString mode = QStringLiteral("fail");
    const QStringList controllers = {"cpu"};
    NamedVariantList properties = {
        NamedVariant({QStringLiteral("PIDs"), QDBusVariant(QVariant::fromValue(pidList))}),
        NamedVariant({QStringLiteral("Slice"), QDBusVariant(QVariant::fromValue(QString::fromStdString(parentSlice)))}),
        NamedVariant({QStringLiteral("DelegateControllers"), QDBusVariant(QVariant::fromValue(controllers))})};
    QList<QPair<QString, NamedVariantList>> aux;

    asyncCallSystemdDbus(
        "StartTransientUnit", {scopeName, mode, QVariant::fromValue(properties), QVariant::fromValue(aux)},
        QDBusConnection::sessionBus());
}

void SystemdDbusInterface::setProcessGroupResourceLimit(
    const std::string &unitName, const std::string &attributeName,
    const std::string &value)
{
    if (attributeName == "Freezer") {
        setFreezerProperty(unitName, value);
    } else {
        setGeneralProperty(unitName, attributeName, value);
    }
}

void SystemdDbusInterface::setSystemdUnitPropertyEnabled(const QString &unitName, const QString &propertyName, bool enabled)
{
    NamedVariantList properties = {
        NamedVariant({propertyName, QDBusVariant(QVariant::fromValue(enabled))}),
    };

    asyncCallSystemdDbus(
        "SetUnitProperties", {unitName, true, QVariant::fromValue(properties)},
        QDBusConnection::systemBus());
}

void SystemdDbusInterface::setSystemdUnitRun(
        const QString &unitName, bool start, bool mode)
{
    if (mode == 1) {
        asyncCallSystemdDbus(
            start?"StartUnit":"StopUnit", {unitName, "replace"}, QDBusConnection::systemBus());
    } else {
        QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            PRINT_NOTICE("SystemdDbusInterface::getServiceMainpid, bus is not connected\n");
            return;
        }

        QDBusInterface systemdInterface(
            systemd_service, systemd_path,
            systemd_manager_inerface, bus);
        QDBusReply<QDBusObjectPath> reply = systemdInterface.call(start?"StartUnit":"StopUnit", unitName, "replace");
        if (reply.isValid()) {
            PRINT_DBG("service start/stop success: %s\n", reply.value().path().toStdString().c_str());
        } else {
            PRINT_NOTICE("service start/stop fail %s\n", reply.error().message().toStdString().c_str());
        }
    }
}

void SystemdDbusInterface::setSystemdUnitFreezelimit(const QString &unitName, bool freeze, bool mode)
{
    if (freeze == true) {
        freezeUnit(unitName.toStdString(), mode);
    } else {
        thawUnit(unitName.toStdString(), mode);
    }
}

int SystemdDbusInterface::getServiceMainpid(const QString &unitName, bool mode)
{
    std::string escaped;
    std::string cppStr = unitName.toStdString();
    escaped.reserve(cppStr.length() * 3);

    for (unsigned char c : cppStr) {
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            (c == '_')) {
            // 合法字符直接保留
            escaped.push_back(static_cast<char>(c));
        } else {
            // 非法字符转义为 _XX 格式
            char hex_code[4];
            std::snprintf(hex_code, sizeof(hex_code), "_%02x", c); // 小写十六进制
            escaped.append(hex_code);
        }
    }

    PRINT_DBG("service name %s\n", escaped.c_str());
    std::string path="/org/freedesktop/systemd1/unit/";
    path.append(escaped);

    QDBusConnection bus = mode?QDBusConnection::systemBus():QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        PRINT_NOTICE("SystemdDbusInterface::getServiceMainpid, bus is not connected\n");
        return -1;
    }

    QDBusInterface systemdInterface(
        systemd_service, QString::fromStdString(path),
        dbus_properties_interface, bus);
    QDBusReply<QVariant> reply = systemdInterface.call("Get", "org.freedesktop.systemd1.Service", "MainPID");

    if (!reply.isValid()) {
        PRINT_NOTICE("getServiceMainpid dbus call failed: %s\n", reply.error().message().toStdString().c_str());
        return -2;
    }

    int mainPid = reply.value().toUInt();
    PRINT_DBG("SystemdDbusInterface::getServiceMainpid: %d\n", mainPid);
    return mainPid;
}

std::vector<int> SystemdDbusInterface::getUnitProcessIds(const std::string &unitName) const
{
    QDBusInterface systemdInterface(systemd_service, systemd_path, systemd_manager_inerface);
    QDBusMessage processInfos = systemdInterface.call("GetUnitProcesses", unitName.c_str());
    auto arguments = processInfos.arguments();
    if (arguments.isEmpty()) {
        return {};
    }
    QVariant processes = arguments.constFirst();
    if (!processes.canConvert<QDBusArgument>()) {
        return {};
    }

    auto processesInfo = processes.value<QDBusArgument>();
    QList<UnitProcess> unitProcesses;
    processesInfo >> unitProcesses;

    return processIdsFromUnitProcesses(unitProcesses);
}

void SystemdDbusInterface::registerDbusTypes()
{
    qDBusRegisterMetaType<NamedVariant>();
    qDBusRegisterMetaType<NamedVariantList>();
    qDBusRegisterMetaType<QPair<QString, NamedVariantList>>();
    qDBusRegisterMetaType<QList<QPair<QString, NamedVariantList>>>();
}

std::string SystemdDbusInterface::getCurrentLoginProperty(
    const std::string &property, const std::string &interface,
    const std::string &path) const
{
    QDBusInterface systemdInterface(
        login1_service, QString::fromStdString(path),
        dbus_properties_interface, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = systemdInterface.call("Get", QString::fromStdString(interface), QString::fromStdString(property));
    return reply.value().toString().toStdString();
}

void SystemdDbusInterface::setGeneralProperty(
    const std::string &unitName, const std::string &propertyName,
    const std::string &value)
{
    const QString name = QString::fromStdString(unitName);
    const QString property = QString::fromStdString(propertyName);
    const QVariant propertyValue = propertyValueFromString(property, QString::fromStdString(value));

    NamedVariantList properties = {
        NamedVariant({property, QDBusVariant(propertyValue)})};

    PRINT_DBG("Set unit properties: %s, %s, %s\n", unitName.c_str(), propertyName.c_str(), propertyValue.toString().toStdString().c_str());

    asyncCallSystemdDbus(
        "SetUnitProperties", {name, true, QVariant::fromValue(properties)},
        QDBusConnection::sessionBus());
}

void SystemdDbusInterface::setFreezerProperty(
    const std::string &unitName, const std::string &value)
{
    if (value == "1") {
        freezeUnit(unitName, false); // 用户权限
    } else {
        thawUnit(unitName, false); // 用户权限
    }
}

void SystemdDbusInterface::freezeUnit(const std::string &unitName, bool mode)
{
    if (mode == 1) {
        asyncCallSystemdDbus(
            "FreezeUnit", {QString::fromStdString(unitName)},
            QDBusConnection::systemBus());
    } else {
        asyncCallSystemdDbus(
            "FreezeUnit", {QString::fromStdString(unitName)},
            QDBusConnection::sessionBus());
    }
}

void SystemdDbusInterface::thawUnit(const std::string &unitName, bool mode)
{
    if (mode == 1) {
        asyncCallSystemdDbus(
            "ThawUnit", {QString::fromStdString(unitName)},
            QDBusConnection::systemBus());
    } else {
        asyncCallSystemdDbus(
            "ThawUnit", {QString::fromStdString(unitName)},
            QDBusConnection::sessionBus());
    }
}

void SystemdDbusInterface::asyncCallSystemdDbus(
    const QString &method, const QList<QVariant> &arguments, QDBusConnection dbusConnection)
{
    QDBusMessage message = QDBusMessage::createMethodCall(
        systemd_service, systemd_path,
        systemd_manager_inerface, method);

    message.setArguments(arguments);
    QDBusPendingCall pcall = dbusConnection.asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, qApp,
                     [watcher, method, arguments](QDBusPendingCallWatcher *call) {
        QDBusPendingReply<> reply = *call;
        if (reply.isError()) {
            QStringList strList;
            for (const QVariant &var : arguments) {
                // QVariant::toString()可处理大部分内置类型（int、bool、QString等）
                strList << var.toString();
            }
            PRINT_NOTICE("Call SystemdDbusInterface::%s error: %s, %s\n", method.toStdString().c_str(), reply.error().message().toStdString().c_str(), strList.join(", ").toStdString().c_str());
        }
        call->deleteLater();
        watcher->deleteLater();
    });
}
