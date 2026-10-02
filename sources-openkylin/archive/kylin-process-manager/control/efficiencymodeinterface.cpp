#include "efficiencymodeinterface.h"

#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDebug>
#include <QGuiApplication>

namespace
{
const char *daemon_dbus_service = "com.kylin.ProcessManagerDaemon";
const char *efficiency_mode_path = "/com/kylin/ProcessManagerDaemon/EfficiencyMode";
const char *efficiency_mode_interface = "com.kylin.ProcessManagerDaemon.EfficiencyMode";

}

EfficiencyModeInterface::EfficiencyModeInterface()
    : dbusInterface(daemon_dbus_service, efficiency_mode_path,
                    efficiency_mode_interface, QDBusConnection::systemBus())
{
}

void EfficiencyModeInterface::setPidFilter(int pid, bool enable)
{
    qDebug() << "EfficiencyModeInterface::setPidFilter: "
             << QString::number(pid)
             << enable;

    QList<QVariant> argumentList;
    argumentList << pid
                 << enable;
    return asyncCallWithArgumentList("SetPidFilter", argumentList);
}

void EfficiencyModeInterface::asyncCallWithArgumentList(const QString &method, const QList<QVariant> &args)
{
    QDBusPendingCall pcall = dbusInterface.asyncCallWithArgumentList(method, args);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, qApp,
                     [watcher, method](QDBusPendingCallWatcher *call) {
                         QDBusPendingReply<> reply = *call;
                         if (reply.isError()) {
                             qWarning() << QString("Call daemon dbus %1 error: ").arg(method) << reply.error();
                         }
                         call->deleteLater();
                         watcher->deleteLater();
                     });
}
