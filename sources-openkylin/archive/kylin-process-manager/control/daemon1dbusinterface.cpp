#include "daemon1dbusinterface.h"
#include "loghelper.h"
#include "misc.h"
#include <QApplication>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>
#include <iostream>

namespace {

const char *daemon1_dbus_service = "com.kylin.ProcessResourceManagerDaemon";
const char *daemon1_dbus_path = "/com/kylin/ProcessResourceManagerDaemon";
const char *daemon1_dbus_interface = "com.kylin.ProcessResourceManagerDaemon";

}

Daemon1DbusInterface::Daemon1DbusInterface()
    : dbusInterface(daemon1_dbus_service, daemon1_dbus_path,
        daemon1_dbus_interface, QDBusConnection::systemBus())
{

}

void Daemon1DbusInterface::insertTopTask(int pid, int topType, int topPrio, int topSlice, int topTypeByTgid)
{
    // qDebug() << "Daemon1DbusInterface::insertTopTask: "
    //     << QString::number(pid)
    //     << QString::number(topType)
    //     << QString::number(topPrio)
    //     << QString::number(topSlice)
    //     << QString::number(topTypeByTgid);
    PRINT_DBG("Daemon1DbusInterface::insertTopTask: %d %d %d %d %d\n", pid, topType, topPrio, topSlice, topTypeByTgid);
    QList<QVariant> argumentList;
    argumentList
        << QVariant::fromValue(pid)
        << QVariant::fromValue(topType)
        << QVariant::fromValue(topPrio)
        << QVariant::fromValue(topSlice)
        << QVariant::fromValue(topTypeByTgid);
    asyncCallWithArgumentList("InsertTopTask", argumentList);
}

void Daemon1DbusInterface::eraseTopTask(int pid)
{
    // qDebug() << "Daemon1DbusInterface::eraseTopTask: "
    //     << QString::number(pid);
    PRINT_DBG("Daemon1DbusInterface::eraseTopTask: %d\n", pid);
    QList<QVariant> argumentList;
    argumentList
        << QVariant::fromValue(pid);
    asyncCallWithArgumentList("EraseTopTask", argumentList);
}

void Daemon1DbusInterface::enable()
{
    QList<QVariant> argumentList;
    asyncCallWithArgumentList("Enable", argumentList);
}

void Daemon1DbusInterface::disable()
{
    QList<QVariant> argumentList;
    asyncCallWithArgumentList("Disable", argumentList);
}

void Daemon1DbusInterface::clearTopList()
{
    QList<QVariant> argumentList;
    asyncCallWithArgumentList("ClearTopList", argumentList);
}

void Daemon1DbusInterface::asyncCallWithArgumentList(const QString &method, const QList<QVariant> &args)
{
    QDBusPendingCall pcall = dbusInterface.asyncCallWithArgumentList(method, args);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, qApp,
                     [watcher, method](QDBusPendingCallWatcher *call) {
        QDBusPendingReply<> reply = *call;
        if (reply.isError()) {
            PRINT_NOTICE("Call daemon dbus error: %s\n", reply.error().message().toStdString().c_str());
        }
        call->deleteLater();
        watcher->deleteLater();
    });
}