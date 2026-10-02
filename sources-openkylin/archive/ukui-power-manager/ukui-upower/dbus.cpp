#include "dbus.h"

DBus::DBus(QObject *parent) : QObject(parent), QDBusContext(){}

DBus::~DBus() {}

void DBus::showServiceMsg()
{
    QDBusConnection con = connection();
    QDBusMessage msg = message();
    int str = con.interface()->servicePid(msg.service()).value();
    qDebug() << "Sender Name" << con.interface()->serviceOwner(msg.service()).value();
    qDebug() << "Sender Uid" << con.interface()->serviceUid(msg.service()).value();
    qDebug() << "Sender Pid " << str;
    qDebug() << executeLinuxCmd(QString::number(str));
}

QString DBus::executeLinuxCmd(QString strCmd)
{
    QProcess process;
    process.start("bash", QStringList() << "-c" << QString("ps -p %1 -o comm=").arg(strCmd));
    process.waitForFinished();
    QString strResult = process.readAllStandardOutput() + process.readAllStandardError();
    return strResult;
}
