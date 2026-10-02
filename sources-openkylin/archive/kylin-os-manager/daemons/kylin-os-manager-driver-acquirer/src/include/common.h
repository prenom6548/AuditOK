#ifndef COMMON_H
#define COMMON_H
#include <QString>
#include <QProcess>
#include <QStringList>


const QString DBUS_SERVICE_NAME = QString("com.kylin-os-manager-driver-acquirer");
const QString DBUS_OBJECT_PATH = QString("/driveracquirer");
const QString DBUS_OBJECT_INTERFACE = QString("driveracquirer.tool");

QString getDebianArchitecture();

QString getRetFromCommand(const QStringList &command);

bool checkInstallable(const QString name, const QString version);

QString getDebVersion(const QString &debName);

QString getDebName(const QString &debName);

#endif // COMMON_H
