#include "sessionserviceregister.h"

#include <QDBusConnection>
#include <QDBusInterface>

#include <QProcess>
#include <QDebug>

SessionServiceRegister::SessionServiceRegister(QObject *parent)
    : ServiceRegister{parent}
{

}

bool SessionServiceRegister::registerServicesInternal(const QString &pluginPath, const QStringList &serviceNames, const QString &serviceSpace, bool resident, const QStringList &args)
{
    // 判断是否存在对应serviceSpace服务，如果不存在则拉起serviceSpace的进程
    QString serviceName = QString("org.ukui.Service.%1").arg(serviceSpace);
    qDebug() << serviceName << serviceSpace << resident;
    QString servicePath = "/org/ukui/ServiceSpace/" + serviceSpace;
    QString serviceInterface = "org.ukui.ServiceSpace." + serviceSpace;
    QDBusInterface interface(serviceName, servicePath, serviceInterface, QDBusConnection::sessionBus(), nullptr);
    if (interface.isValid()) {
        auto message = interface.call(QDBus::Block, "launchServices", pluginPath, serviceNames, resident, args);
        auto args = message.arguments();
        if (!args.isEmpty()) {
            return args.first().toBool();
        } else {
            qCritical() << message.errorMessage();
            return false;
        }
    } else {
        QStringList tmpServices;
        for (auto tmpString : serviceNames) {
            tmpServices << "-s" << tmpString;
        }
        QProcess p;
        p.setProgram("ukui-session-service");
        QStringList tmp = QStringList() << "-p" << pluginPath << "-n" << serviceSpace << "-r" << (resident? "true": "false") << tmpServices;
        for (auto arg : args) {
            tmp << "-a" << arg;
        }
        p.setArguments(tmp);
        p.startDetached();
        while(1) {
            QDBusInterface interface(serviceName, "/", QString(), QDBusConnection::sessionBus(), nullptr);
            if (interface.isValid()) {
                return true;
            }
        }
    }

    return false;
}
