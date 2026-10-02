#include "serviceregister.h"

#include <QPluginLoader>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>

#include <QDebug>

#include <unistd.h>

ServiceRegister::ServiceRegister(QObject *parent)
    : QObject{parent}
{

}

bool ServiceRegister::registerServiceByPlugin(const QString &pluginPath, const QStringList &args)
{
    int callerUid = connection().interface()->serviceUid(message().service()).value();
    if (callerUid != getuid()) {
        sendErrorReply(QDBusError::AccessDenied, "caller has no permission to register service");
        return false;
    }

    QPluginLoader l;
    l.setFileName(pluginPath);

    if (!l.load()) {
        qWarning() << l.errorString();
        return false;
    }

    auto metaObjectValue = l.metaData().value("MetaData");
    auto metaObject = metaObjectValue.toObject();

    l.unload();

    QJsonValue tmp = metaObject["services"];
    // 获取当前插件的服务列表，按常驻和非常驻&命名空间进行划分
    auto servicesArray = tmp.toArray();
    if (servicesArray.isEmpty()) {
        QString errorMsg = QString("注册插件服务 %1 错误，未找到服务配置").arg(pluginPath);
        sendErrorReply(QDBusError::InvalidArgs, errorMsg);
        qCritical() << errorMsg;
        return false;
    }

    QHash<QString, QStringList> spacesAndResidentServices;
    QHash<QString, QStringList> spacesAndOneshotServices;
    for (auto serviceValue : servicesArray) {
        auto service = serviceValue.toObject();
        auto serviceName = service["name"].toString();
        auto spaceName = service["namespace"].toString();
        bool resident = service["resident"].toBool();
        if (resident) {
            auto tmpServices = spacesAndResidentServices.value(spaceName);
            tmpServices.append(serviceName);
            spacesAndResidentServices.insert(spaceName, tmpServices);
        } else {
            auto tmpServices = spacesAndOneshotServices.value(spaceName);
            tmpServices.append(serviceName);
            spacesAndOneshotServices.insert(spaceName, tmpServices);
        }
    }

    bool successed = true;

    // 根据服务命名空间注册非常驻服务
    for (auto spaceName : spacesAndOneshotServices.keys()) {
        auto servicesToBeRegistered = spacesAndOneshotServices.value(spaceName);
        bool ok = registerServicesInternal(pluginPath, servicesToBeRegistered, spaceName, false, args);
        if (!ok) {
            qCritical() << "failed to register" << pluginPath << servicesToBeRegistered;
            successed = false;
        }
    }

    // 根据服务命名空间注册常驻服务
    for (auto spaceName : spacesAndResidentServices.keys()) {
        auto servicesToBeRegistered = spacesAndResidentServices.value(spaceName);
        bool ok = registerServicesInternal(pluginPath, servicesToBeRegistered, spaceName, true, args);
        if (!ok) {
            qCritical() << "failed to register" << pluginPath << servicesToBeRegistered;
            successed = false;
        }
    }

    return successed;
}

bool ServiceRegister::registerServicesInternal(const QString &pluginPath, const QStringList &serviceNames, const QString &serviceSpace, bool resident, const QStringList &args)
{
    Q_UNUSED(pluginPath);
    Q_UNUSED(serviceNames);
    Q_UNUSED(serviceSpace);
    Q_UNUSED(resident);
    // 需要重写session service register和system service register的流程
    return true;
}
