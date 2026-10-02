#include "cgroupcontrolinterface.h"

#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDebug>
#include <QGuiApplication>

namespace
{
const char *daemon_dbus_service = "com.kylin.ProcessManagerDaemon";
const char *cgroup_control_path = "/com/kylin/ProcessManagerDaemon/CgroupControl";
const char *cgroup_control_interface = "com.kylin.ProcessManagerDaemon.CgroupControl";

}

CgroupControlInterface::CgroupControlInterface()
    : dbusInterface(daemon_dbus_service, cgroup_control_path,
                    cgroup_control_interface, QDBusConnection::systemBus())
{
}

int CgroupControlInterface::limitProcessCPUPercent(int pid, double cpuPercent)
{
    qDebug() << "CgroupControlInterface::limitProcessCPUPercent: "
             << QString::number(pid)
             << QString::number(cpuPercent);

    QList<QVariant> argumentList;
    argumentList << pid
                 << cpuPercent
                 << "auto";
    return blockCallWithArgumentList("LimitProcessCPUPercent", argumentList).toInt();
}

int CgroupControlInterface::limitProcessCPUWeight(int pid, int cpuWeight)
{
    qDebug() << "CgroupControlInterface::limitProcessCPUWeight: "
             << QString::number(pid)
             << QString::number(cpuWeight);

    QList<QVariant> argumentList;
    argumentList << pid
                 << cpuWeight
                 << "auto";
    return blockCallWithArgumentList("LimitProcessCPUWeight", argumentList).toInt();
}

int CgroupControlInterface::limitProcessMemoryMax(int pid, uint64_t memoryMax)
{
    qDebug() << "CgroupControlInterface::limitProcessMemoryMax: "
             << QString::number(pid)
             << QString::number(memoryMax);

    QList<QVariant> argumentList;
    argumentList << pid
                 << QVariant::fromValue(memoryMax)
                 << "auto";
    return blockCallWithArgumentList("LimitProcessMemoryMax", argumentList).toInt();
}

int CgroupControlInterface::limitProcessIOWeight(int pid, int ioWeight)
{
    qDebug() << "CgroupControlInterface::limitProcessIOWeight: "
             << QString::number(pid)
             << QString::number(ioWeight);

    QList<QVariant> argumentList;
    argumentList << pid
                 << ioWeight
                 << "auto";
    return blockCallWithArgumentList("LimitProcessIOWeight", argumentList).toInt();
}

int CgroupControlInterface::freezeProcess(int pid)
{
    qDebug() << "CgroupControlInterface::freezeProcess: "
             << QString::number(pid);

    QList<QVariant> argumentList;
    argumentList << pid
                 << "auto";
    return blockCallWithArgumentList("FreezeProcess", argumentList).toInt();
}

int CgroupControlInterface::limitThreadCPUPercent(int pid, int tid, double cpuPercent)
{
    qDebug() << "CgroupControlInterface::limitThreadCPUPercent: "
             << QString::number(pid)
             << QString::number(tid)
             << QString::number(cpuPercent);

    QList<QVariant> argumentList;
    argumentList << pid
                 << tid
                 << cpuPercent
                 << "auto";
    return blockCallWithArgumentList("LimitThreadCPUPercent", argumentList).toInt();
}

int CgroupControlInterface::limitThreadCPUWeight(int pid, int tid, int cpuWeight)
{
    qDebug() << "CgroupControlInterface::limitThreadCPUWeight: "
             << QString::number(pid)
             << QString::number(tid)
             << QString::number(cpuWeight);

    QList<QVariant> argumentList;
    argumentList << pid
                 << tid
                 << cpuWeight
                 << "auto";
    return blockCallWithArgumentList("LimitThreadCPUWeight", argumentList).toInt();
}

int CgroupControlInterface::limitThreadMemoryMax(int pid, int tid, uint64_t memoryMax)
{
    qDebug() << "CgroupControlInterface::limitThreadMemoryMax: "
             << QString::number(pid)
             << QString::number(tid)
             << QString::number(memoryMax);

    QList<QVariant> argumentList;
    argumentList << pid
                 << tid
                 << QVariant::fromValue(memoryMax)
                 << "auto";
    return blockCallWithArgumentList("LimitThreadMemoryMax", argumentList).toInt();
}

int CgroupControlInterface::limitThreadIOWeight(int pid, int tid, int ioWeight)
{
    qDebug() << "CgroupControlInterface::limitThreadIOWeight: "
             << QString::number(pid)
             << QString::number(tid)
             << QString::number(ioWeight);

    QList<QVariant> argumentList;
    argumentList << pid
                 << tid
                 << ioWeight
                 << "auto";
    return blockCallWithArgumentList("LimitThreadIOWeight", argumentList).toInt();
}

int CgroupControlInterface::freezeThread(int pid, int tid)
{
    qDebug() << "CgroupControlInterface::freezeThread: "
             << QString::number(pid)
             << QString::number(tid);

    QList<QVariant> argumentList;
    argumentList << pid
                 << tid
                 << "auto";
    return blockCallWithArgumentList("FreezeThread", argumentList).toInt();
}

bool CgroupControlInterface::releaseLimitByIndex(int index)
{
    qDebug() << "CgroupControlInterface::releaseLimitByIndex: "
             << QString::number(index);

    QList<QVariant> argumentList;
    argumentList << index;
    return blockCallWithArgumentList("ReleaseLimitByIndex", argumentList).toBool();
}

bool CgroupControlInterface::setCurrentUserProperty(std::string currentUserUID,
                                                    std::string currentUserSlice, std::string currentUserService, std::string currentUserSession)
{
    qDebug() << "CgroupControlInterface::setCurrentUserProperty: "
             << QString::fromStdString(currentUserUID)
             << QString::fromStdString(currentUserSlice)
             << QString::fromStdString(currentUserService)
             << QString::fromStdString(currentUserSession);

    QList<QVariant> argumentList;
    argumentList << QString::fromStdString(currentUserUID)
                 << QString::fromStdString(currentUserSlice)
                 << QString::fromStdString(currentUserService)
                 << QString::fromStdString(currentUserSession);
    return blockCallWithArgumentList("SetCurrentUserProperty", argumentList).toBool();
}

void CgroupControlInterface::setGroupPath(std::string currentUserUID, std::string name, std::string path)
{
    qDebug() << "CgroupControlInterface::setGroupPath: "
             << QString::fromStdString(currentUserUID)
             << QString::fromStdString(name)
             << QString::fromStdString(path);

    QList<QVariant> argumentList;
    argumentList << QString::fromStdString(currentUserUID)
                 << QString::fromStdString(name)
                 << QString::fromStdString(path);
    asyncCallWithArgumentList("SetGroupPath", argumentList);
}

void CgroupControlInterface::setGroupProperty(std::string currentUserUID, std::string name, std::string node, std::string value)
{
    qDebug() << "CgroupControlInterface::setGroupProperty: "
             << QString::fromStdString(currentUserUID)
             << QString::fromStdString(name)
             << QString::fromStdString(node)
             << QString::fromStdString(value);

    QList<QVariant> argumentList;
    argumentList << QString::fromStdString(currentUserUID)
                 << QString::fromStdString(name)
                 << QString::fromStdString(node)
                 << QString::fromStdString(value);
    asyncCallWithArgumentList("SetGroupProperty", argumentList);
}

void CgroupControlInterface::createGroupPath(std::string currentUserUID, std::string name)
{
    qDebug() << "CgroupControlInterface::createGroupPath: "
             << QString::fromStdString(currentUserUID)
             << QString::fromStdString(name);

    QList<QVariant> argumentList;
    argumentList << QString::fromStdString(currentUserUID)
                 << QString::fromStdString(name);
    asyncCallWithArgumentList("CreateGroupPath", argumentList);
}

void CgroupControlInterface::removeGroupPath(std::string currentUserUID, std::string name)
{
    qDebug() << "CgroupControlInterface::removeGroupPath: "
             << QString::fromStdString(currentUserUID)
             << QString::fromStdString(name);

    QList<QVariant> argumentList;
    argumentList << QString::fromStdString(currentUserUID)
                 << QString::fromStdString(name);
    asyncCallWithArgumentList("RemoveGroupPath", argumentList);
}

void CgroupControlInterface::limitProcessByGroup(std::string name, int pid)
{
    qDebug() << "CgroupControlInterface::limitProcessByGroup: "
             << QString::fromStdString(name)
             << pid;

    QList<QVariant> argumentList;
    argumentList << QString::fromStdString(name)
                 << pid;

    asyncCallWithArgumentList("LimitProcessByGroup", argumentList);
}

void CgroupControlInterface::setGroupEnable(bool enable)
{
    qDebug() << "CgroupControlInterface::setGroupEnable: "
             << enable;

    QList<QVariant> argumentList;
    argumentList << enable;

    asyncCallWithArgumentList("SetGroupEnable", argumentList);
}

void CgroupControlInterface::setFreezeEnable(bool enable)
{
    qDebug() << "CgroupControlInterface::setFreezeEnable: "
             << enable;

    QList<QVariant> argumentList;
    argumentList << enable;

    asyncCallWithArgumentList("SetFreezeEnable", argumentList);
}

void CgroupControlInterface::thawedProcess(int pid)
{
    qDebug() << "CgroupControlInterface::thawedProcess: "
             << pid;

    QList<QVariant> argumentList;
    argumentList << pid;

    asyncCallWithArgumentList("ThawedProcess", argumentList);
}

void CgroupControlInterface::asyncCallWithArgumentList(const QString &method, const QList<QVariant> &args)
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

QVariant CgroupControlInterface::blockCallWithArgumentList(const QString &method, const QList<QVariant> &args)
{
    QDBusMessage msg = dbusInterface.callWithArgumentList(QDBus::Block, method, args);
    if (msg.type() == QDBusMessage::ErrorMessage) {
        qWarning() << QString("Call daemon dbus %1 error: ").arg(method) << msg.errorMessage();
    } else {
        QVariant result = msg.arguments().at(0);
        return result;
    }
    return QVariant();
}
