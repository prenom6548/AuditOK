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

#include "daemondbusinterface.h"
#include "loghelper.h"
#include "misc.h"
#include <QApplication>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDebug>
#include <iostream>

namespace
{

const char *daemon_dbus_service = "com.kylin.ProcessManagerDaemon";
const char *daemon_dbus_path = "/com/kylin/ProcessManagerDaemon";
const char *daemon_dbus_interface = "com.kylin.ProcessManagerDaemon";

}

DaemonDbusInterface::DaemonDbusInterface(QObject *parent)
    : QObject(parent)
    , dbusInterface(daemon_dbus_service, daemon_dbus_path,
                    daemon_dbus_interface, QDBusConnection::systemBus())
{
    connect(&dbusInterface, SIGNAL(ResourceThresholdWarning(QString, int)),
            this, SLOT(onResourceWarning(QString, int)));
}

void DaemonDbusInterface::onResourceWarning(const QString &resource, int level)
{
    //qDebug() << "Get resource warning:" << resource << "warning level:" << level;
    PRINT_DBG("Get resource warning: %s warning level: %d", resource.toStdString().c_str(), level);
    Q_EMIT ResourceThresholdWarning(resource.toStdString(), level);
}

double DaemonDbusInterface::getCpuEnergyConsumption()
{
    QDBusPendingReply<double> reply = callWithArgumentList("GetCpuEnergyConsumption", {});
    if (!reply.isValid()) {
        //qWarning() << "GetCpuEnergyConsumption failed: " << reply.error();
        PRINT_NOTICE("GetCpuEnergyConsumption failed: %s\n", reply.error().message().toStdString().c_str());
        return 0;
    }
    return reply.value();
}

void DaemonDbusInterface::reclaimProcesses(const std::vector<int> &pids)
{
    //qDebug() << "DaemonDbusInterface::reclaimProcesses: " << misc::stl2qt::vector2QList<int>(pids);
    QStringList strList;
    for (int pid : misc::stl2qt::vector2QList<int>(pids)) {
        strList << QString::number(pid); // 整数转字符串
    }
    PRINT_DBG("DaemonDbusInterface::reclaimProcesses: %s\n", strList.join(", ").toStdString().c_str());
    QList<QVariant> argumentList = {QVariant::fromValue(misc::stl2qt::vector2QList<int>(pids))};
    asyncCallWithArgumentList("ReclaimProcesses", argumentList);
}

void DaemonDbusInterface::reclaimProcessGroups(const std::vector<std::string> &groupNames)
{
    //qDebug() << "DaemonDbusInterface::reclaimProcesses: " << misc::stl2qt::vectorString2QStringList(groupNames);
    PRINT_DBG("DaemonDbusInterface::reclaimProcesses: %s\n", misc::stl2qt::vectorString2QStringList(groupNames).join(", ").toStdString().c_str());

    QList<QVariant> argumentList = {QVariant::fromValue(misc::stl2qt::vectorString2QStringList(groupNames))};
    asyncCallWithArgumentList("ReclaimProcessGroups", argumentList);
}

void DaemonDbusInterface::asyncCallWithArgumentList(const QString &method, const QList<QVariant> &args)
{
    QDBusPendingCall pcall = dbusInterface.asyncCallWithArgumentList(method, args);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, qApp,
                     [watcher, method](QDBusPendingCallWatcher *call) {
        QDBusPendingReply<> reply = *call;
        if (reply.isError()) {
            //qWarning() << QString("Call daemon dbus %1 error: ").arg(method) << reply.error();
            // QString errMsg = QString("DBus Error: %1 (Name: %2)")
            //                 .arg(reply.error().errorString(reply.error().type()))  // 错误描述（如"Permission denied"）
            //                 .arg(reply.error().name());
            PRINT_NOTICE("Call daemon dbus %s error: %s \n", method.toStdString().c_str(), reply.error().message().toStdString().c_str());
        }
        call->deleteLater();
        watcher->deleteLater();
    });
}

QDBusMessage DaemonDbusInterface::callWithArgumentList(
    const QString &method, const QList<QVariant> &args)
{
    return dbusInterface.callWithArgumentList(QDBus::Block, method, args);
}


void DaemonDbusInterface::openPSR()
{
    QDBusInterface iface(daemon_dbus_service, daemon_dbus_path, daemon_dbus_interface, QDBusConnection::systemBus());
    if (!iface.isValid()) 
    {
        PRINT_NOTICE("DaemonDbusInterface::openPSR()-- iface.is not Valid()\n");
        return;
    }
    QDBusReply<void> reply = iface.call("OpenPSR");

    // 检查调用是否成功
    if (reply.isValid()) {
        PRINT_DBG("DaemonDbusInterface::openPSR() success\n");
    } else {
        PRINT_NOTICE("DaemonDbusInterface::openPSR() failed\n");
    }
}

void DaemonDbusInterface::closePSR() {
    QDBusInterface iface(daemon_dbus_service, daemon_dbus_path, daemon_dbus_interface, QDBusConnection::systemBus());
    if (!iface.isValid()) 
    {
        PRINT_NOTICE("DaemonDbusInterface::closePSR()-- iface.is not Valid()\n");
        return;
    }
    QDBusReply<void> reply = iface.call("ClosePSR");

    // 检查调用是否成功
    if (reply.isValid()) {
        PRINT_DBG("DaemonDbusInterface::closePSR() success\n");
    } else {
        PRINT_NOTICE("DaemonDbusInterface::closePSR() failed\n");
    }
}

void DaemonDbusInterface::setVRRRefreshRate(std::string mode)
{
    QDBusInterface iface(daemon_dbus_service, daemon_dbus_path, daemon_dbus_interface, QDBusConnection::systemBus());
    if (!iface.isValid()) 
    {
        PRINT_NOTICE("DaemonDbusInterface::setVRRRefreshRate()-- iface.is not Valid()\n");
        return;
    }
    QDBusReply<void> reply = iface.call("SetVRRRefreshRate", QString::fromStdString(mode));

    // 检查调用是否成功
    if (reply.isValid()) {
        PRINT_DBG("DaemonDbusInterface::setVRRRefreshRate() success\n");
    } else {
        PRINT_NOTICE("DaemonDbusInterface::setVRRRefreshRate() failed\n");
    }
}

void DaemonDbusInterface::reverseVRRRefreshRate()
{
    QDBusInterface iface(daemon_dbus_service, daemon_dbus_path, daemon_dbus_interface, QDBusConnection::systemBus());
    if (!iface.isValid()) 
    {
        PRINT_NOTICE("DaemonDbusInterface::reverseVRRRefreshRate()-- iface.is not Valid()\n");
        return;
    }
    QDBusReply<void> reply = iface.call("ReverseVRRRefreshRate");

    // 检查调用是否成功
    if (reply.isValid()) {
        PRINT_DBG("DaemonDbusInterface::reverseVRRRefreshRate() success\n");
    } else {
        PRINT_NOTICE("DaemonDbusInterface::reverseVRRRefreshRate() failed\n");
    }
}

void DaemonDbusInterface::setSystemdUnitRun(
    const std::string &unitName, bool start)
{
    // qDebug() << "DaemonDbusInterface::setSystemdUnitRun: "
    //          << QString::fromStdString(unitName)
    //          << start;
    PRINT_DBG("DaemonDbusInterface::setSystemdUnitRun: %s %s\n", unitName.c_str(), start ? "TRUE" : "FALSE");
    QList<QVariant> argumentList;
    argumentList
        << QVariant::fromValue(QString::fromStdString(unitName))
        << QVariant::fromValue(start);
    asyncCallWithArgumentList("SetSystemdUnitRun", argumentList);
}


void DaemonDbusInterface::setCurrentScene(const std::vector<std::string> &currScenes)
{
    // qDebug() << "DaemonDbusInterface::setCurrentScene: "
    // << misc::stl2qt::vectorString2QStringList(currScenes);

    PRINT_DBG("DaemonDbusInterface::setCurrentScene: %s\n", misc::stl2qt::vectorString2QStringList(currScenes).join(", ").toStdString().c_str());

    QList<QVariant> argumentList;
    argumentList
        << misc::stl2qt::vectorString2QStringList(currScenes);
    asyncCallWithArgumentList("SetCurrentScene", argumentList);
}

void DaemonDbusInterface::lockGSettingsKey(const std::string &key, bool lock)
{
    PRINT_DBG("DaemonDbusInterface::lockGSettingsKey: %s %s\n", key.c_str(), lock ? "TRUE" : "FALSE");
    QList<QVariant> argumentList;
    argumentList
        << QVariant::fromValue(QString::fromStdString(key))
        << QVariant::fromValue(lock);
    asyncCallWithArgumentList("LockGSettingsKey", argumentList);
}

void DaemonDbusInterface::setSystemCpuFreq(bool cpufreq)
{
    QDBusInterface iface(daemon_dbus_service, daemon_dbus_path, daemon_dbus_interface, QDBusConnection::systemBus());
    if (!iface.isValid()) 
    {
        PRINT_NOTICE("DaemonDbusInterface::setSystemCpuFreq ,setSystemCpuFreq\n");
        return;
    }
    QDBusReply<void> reply = iface.call("SetSystemCpuFreq", cpufreq);

    // 检查调用是否成功
    if (reply.isValid()) {
        PRINT_DBG("DaemonDbusInterface::setSystemCpuFreq ,setSystemCpuFreq\n");
    } else {
        PRINT_NOTICE("DaemonDbusInterface::setSystemCpuFreq ,setSystemCpuFreq\n");
    }
}
void DaemonDbusInterface::SetKernelSched(bool kernelsched)
{
    QDBusInterface iface(daemon_dbus_service, daemon_dbus_path, daemon_dbus_interface, QDBusConnection::systemBus());
    if (!iface.isValid()) 
    {
        PRINT_NOTICE("DaemonDbusInterface::SetKernelSched ,SetKernelSched\n");
        return;
    }
    QDBusReply<void> reply = iface.call("SetKernelSched", kernelsched);

    // 检查调用是否成功
    if (reply.isValid()) {
        PRINT_DBG("DaemonDbusInterface::SetKernelSched ,SetKernelSched\n");
    } else {
        PRINT_NOTICE("DaemonDbusInterface::SetKernelSched ,SetKernelSched\n");
    }
}
void DaemonDbusInterface::SetDirty(bool dirty)
{
    QDBusInterface iface(daemon_dbus_service, daemon_dbus_path, daemon_dbus_interface, QDBusConnection::systemBus());
    if (!iface.isValid()) 
    {
        PRINT_NOTICE("DaemonDbusInterface::SetDirty ,SetDirty\n");
        return;
    }
    QDBusReply<void> reply = iface.call("SetDirty", dirty);

    // 检查调用是否成功
    if (reply.isValid()) {
        PRINT_DBG("DaemonDbusInterface::SetDirty ,SetDirty\n");
    } else {
        PRINT_NOTICE("DaemonDbusInterface::SetDirty ,SetDirty\n");
    }
}
void DaemonDbusInterface::SetNetParams(bool netparams)
{
    QDBusInterface iface(daemon_dbus_service, daemon_dbus_path, daemon_dbus_interface, QDBusConnection::systemBus());
    if (!iface.isValid()) 
    {
        PRINT_NOTICE("DaemonDbusInterface::SetNetParams ,SetNetParams\n");
        return;
    }
    QDBusReply<void> reply = iface.call("SetNetParams", netparams);

    // 检查调用是否成功
    if (reply.isValid()) {
        PRINT_DBG("DaemonDbusInterface::SetNetParams ,SetNetParams\n");
    } else {
        PRINT_NOTICE("DaemonDbusInterface::SetNetParams ,SetNetParams\n");
    }
}
void DaemonDbusInterface::SetCpuLimit(bool cpulimit)
{
    QDBusInterface iface(daemon_dbus_service, daemon_dbus_path, daemon_dbus_interface, QDBusConnection::systemBus());
    if (!iface.isValid()) 
    {
        PRINT_NOTICE("DaemonDbusInterface::SetCpuLimit ,SetCpuLimit\n");
        return;
    }
    QDBusReply<void> reply = iface.call("SetCpuLimit", cpulimit);

    // 检查调用是否成功
    if (reply.isValid()) {
        PRINT_DBG("DaemonDbusInterface::SetCpuLimit ,SetCpuLimit\n");
    } else {
        PRINT_NOTICE("DaemonDbusInterface::SetCpuLimit ,SetCpuLimit\n");
    }
}
void DaemonDbusInterface::SetSamplingRate(bool samplingrate)
{
    QDBusInterface iface(daemon_dbus_service, daemon_dbus_path, daemon_dbus_interface, QDBusConnection::systemBus());
    if (!iface.isValid()) 
    {
        PRINT_NOTICE("DaemonDbusInterface::SetSamplingRate ,SetSamplingRate\n");
        return;
    }
    QDBusReply<void> reply = iface.call("SetSamplingRate", samplingrate);

    // 检查调用是否成功
    if (reply.isValid()) {
        PRINT_DBG("DaemonDbusInterface::SetSamplingRate ,SetSamplingRate\n");
    } else {
        PRINT_NOTICE("DaemonDbusInterface::SetSamplingRate ,SetSamplingRate\n");
    }
}

