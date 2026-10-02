#include "sessionservicelauncher.h"
#include "../../ukuiserviceinterface/serviceinterface.h"
#include "../../ukuiserviceinterface/serviceobject.h"

#include <QPluginLoader>

#include <QDBusConnection>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <QDBusError>

#include <QDebug>

SessionServiceLauncher::SessionServiceLauncher(QObject *parent)
    : ServiceLauncher{parent}
{

}

bool SessionServiceLauncher::launchServices(const QString &pluginPath, const QStringList &serviceNames, bool resident, const QStringList &args)
{
    Q_UNUSED(resident)
    // 加载一个插件中指定的服务，todo：处理非常驻服务的生命周期管理
    QPluginLoader l;
    //l.setFileName("/home/kylin/git/ukui-service-manager/service-plugin-demo/libservice-plugin-demo.so");
    l.setFileName(pluginPath);

    bool pluginLoaded = l.load();
    Q_ASSERT(pluginLoaded);
    if (!pluginLoaded)
        return false;

    auto instance = l.instance();
    if (instance) {
        if (!args.isEmpty()) {
            instance->setProperty("arguments", args);
        }
    }
    auto plugin = dynamic_cast<UKUI::ServiceInterface *>(instance);
    Q_ASSERT(plugin);
    if (!plugin)
        return false;

    auto services = plugin->getServiceObjects();
    Q_ASSERT(services.count() > 0);
    if (services.isEmpty())
        return false;

    bool successed = true;

    auto jsonObjValue = l.metaData().value("MetaData");
    auto jsonObj = jsonObjValue.toObject();
    auto servicesArray = jsonObj["services"].toArray();
    for (auto serviceValue: servicesArray) {
        auto serviceObject = serviceValue.toObject();
        QString serviceName = serviceObject["name"].toString();

        // 如果待启动的服务不包含此插件的某一个服务，则跳过注册环节
        if (!serviceNames.contains(serviceName)) {
            continue;
        }

        QString servicePath = serviceObject["path"].toString();
        QString serviceInterface = serviceObject["interface"].toString();
        QString serviceRegisterOptions = serviceObject["registeroptions"].toString();
        bool useCustomServiceRegisterOptions = false;
        if (!serviceRegisterOptions.isEmpty()) {
            useCustomServiceRegisterOptions = true;
            qDebug() << serviceName << "has customRegisterOptions" << serviceRegisterOptions;
        }
        qDebug() << jsonObj;

        Q_ASSERT(!serviceName.isEmpty());
        Q_ASSERT(!servicePath.isEmpty());
        Q_ASSERT(!serviceInterface.isEmpty());

        if (services.isEmpty()) {
            qCritical() << "配置信息和服务不匹配";
            break;
        }
        auto service = services.takeFirst();

        // 如果一个服务对应的连接已经被某个object，则继续使用之前的object进行注册
        QDBusConnection serviceConnection = connections.value(serviceName, QDBusConnection::sessionBus());
        if (serviceConnection.name() == QDBusConnection::sessionBus().name()) {
            serviceConnection = QDBusConnection::connectToBus(QDBusConnection::SessionBus, serviceName);
        }

        bool serviceRigistered = serviceConnection.registerService(serviceName);
        //Q_ASSERT(serviceRigistered);

        qDebug() << serviceConnection.lastError();

        QDBusConnection::RegisterOptions options;
        if (useCustomServiceRegisterOptions) {
            bool ok = false;
            options = QDBusConnection::RegisterOption(serviceRegisterOptions.toInt(&ok, 16));
            if (!ok) {
                options = QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals|QDBusConnection::ExportAllProperties;
            } else {
                qDebug() << serviceName << "use custom register options" << options;
            }
        } else {
            options = QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals|QDBusConnection::ExportAllProperties;
        }
        bool objectRegistered = serviceConnection.registerObject(servicePath, serviceInterface, service, options);
        //Q_ASSERT(objectRegistered);
        if (!objectRegistered) {
            if (calledFromDBus()) {
                auto lastError = serviceConnection.lastError();
                sendErrorReply(QDBusError::Failed, lastError.message());
            }
        }

        // 加载服务插件声明的子服务
        // TODO: 添加子服务信息到管控相关流程中
        if (!serviceObject["subservices"].isNull()) {
            auto subservices = serviceObject["subservices"].toArray();
            for (auto subserviceValue : subservices) {
                if (subserviceValue.isObject()) {
                    auto subservice = subserviceValue.toObject();
                    auto subservicename = subservice["serviceobjectname"].toString();
                    auto subservicepath = subservice["path"].toString();
                    auto subserviceinterface = subservice["interface"].toString();
                    auto subserviceoptions = subservice["registeroptions"].toString();
                    auto subserviceRegisterOptions = QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals|QDBusConnection::ExportAllProperties;
                    if (!subserviceoptions.isNull()) {
                        bool ok = false;
                        auto tmp = QDBusConnection::RegisterOption(subserviceoptions.toInt(&ok, 16));
                        if (ok) {
                            subserviceRegisterOptions = tmp;
                        }
                    }

                    // 从serviceobject实例中查找子服务，TODO：实现子服务的管控能力
                    auto subserviceobject = qvariant_cast<QObject *>(service->property(subservicename.toLocal8Bit().constData()));
                    if (subserviceobject) {
                        qDebug() << serviceName << "has sub service" << subservicename << subservicepath << subserviceinterface << subserviceRegisterOptions;
                        bool subserviceRegisted = serviceConnection.registerObject(subservicepath, subserviceinterface, subserviceobject, subserviceRegisterOptions);
                        if (!subserviceRegisted) {
                            qWarning() << "failed to register" << subservicename << serviceConnection.lastError();
                        }
                    }
                }
            }
        }

        if (!serviceRigistered || !objectRegistered)
            successed = false;

        qDebug() << serviceConnection.lastError();

        if (successed) {
            connections.insert(serviceName, serviceConnection);
        }
    }

    return successed;
}
