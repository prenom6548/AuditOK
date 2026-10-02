#include <QDebug>
#include <QDBusConnection>
#include <QProcess>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QThread>
#include "dbus.h"

using namespace dbus;

void Dbus::creatDbusServer()
{
    Tool *toolBus = new Tool(this);
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.registerService(DBUS_SERVICENAME)) {
        qDebug() << "creat dbus service error";
    }
    if (!bus.registerObject(DBUS_PARH_TOOL, DBUS_INTERFACE_TOOL, toolBus, QDBusConnection::ExportAllSlots)) {
        qDebug() << "creat dbus interface error" << DBUS_INTERFACE_TOOL;
    }
}

QStringList Tool::getFileNameFromDir(const QString &arg) const
{
    QStringList tmp;
    QFileInfo info(arg);
    QString path = info.absolutePath();
    tmp = QDir(path).entryList(QStringList() << QString(info.fileName() + "*"), QDir::Files | QDir::Readable, QDir::Name);
    for (QString &str : tmp) {
        str = path + "/" + str;
    }
    return tmp;
}

DbusManager::DbusManager()
{
    QThread *thread = new QThread(this);
    Dbus *dbus = new Dbus();
    connect(this, &DbusManager::creatDbusServer, dbus, &Dbus::creatDbusServer);
    connect(thread, &QThread::finished, dbus, &Dbus::deleteLater);
    dbus->moveToThread(thread);
    thread->start();
    emit creatDbusServer();
}
