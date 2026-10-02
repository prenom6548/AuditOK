#include "sessionclient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "database/drivermanagerdatabase.h"
#include "../service/dbus_args.h"

SessionClient::SessionClient(QObject *parent) : QObject(parent){
    initHotplugClient();
}

void SessionClient::initHotplugClient() {
    // 连接信号槽
    QDBusConnection::systemBus().connect("com.drivermanager.USBMonitor",
                                         "/com/drivermanager/USBMonitor",
                                         "tools.tool", "newDeviceConnectSignal", this,
                                         SLOT(onNewDeviceConnectSlot(QString)));
    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                          "/org/freedesktop/Notifications",
                                          "org.freedesktop.Notifications", "ActionInvoked", this,
                                          SLOT(actionInvokeSlot(quint32, QString)));
}

void SessionClient::onNewDeviceConnectSlot(QString devString) {
    if(!devString.contains("upgradeable")){
        return;
    }
    QJsonDocument jsonDoc = QJsonDocument::fromJson(devString.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();
    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject devJson = jsonValue.toObject();
        if (!(devJson["upgradeable"].toBool())){
            return;
        }else{
            switch (devJson["type"].toInt()) {
            case 0:
                m_devType = "None";
                break;
            case 1:
                m_devType = "printer";
                break;
            case 2:
                m_devType = "scanner";
                break;
            case 3:
                m_devType = "graphiccard";
                break;
            default:
                break;
            }
            m_devModel = devJson["model"].toString();
            QJsonArray driverArray = devJson["driverNameList"].toArray();
            QJsonObject driverObject = driverArray[0].toObject();
            m_driverInfo = driverObject["driverName"].toString().remove(";") + "=" + driverObject["version"].toString();
            m_driverSize = driverObject["size"].toString();
            m_installer = new AptInstaller(m_driverInfo);
            if (oninstall == driverObject["driverName"].toString().remove(";") || m_devType == "printer"){
                return;
            }
        }
    }

    auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  "Notify");
    QList<QVariant> args;
    args.push_back(tr("Kylin Os Manager"));
    args.push_back((quint32)0);
    args.push_back("kylin-os-manager");
    args.push_back(tr("Device Manager"));
    args.push_back(tr("The system detects that a new device has an installable driver. Do you want to install?"));
    args.push_back(QStringList() << "kylin-os-manager-device-manager-close" << tr("Close") << "kylin-os-manager-device-manager-install" << tr("Install"));
    args.push_back(QMap<QString, QVariant>());
    args.push_back((qint32)-1);
    message.setArguments(args);

    auto pending = QDBusConnection::sessionBus().asyncCall(message);
    auto watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        if (!watcher->isValid() || watcher->isError()) {
            qCritical() << "Notify D-Bus share object is error.";
            return;
        }
        QDBusPendingReply<quint32> reply = *watcher;
        if (!reply.isValid() || reply.isError()) {
            qCritical() << "Notify D-Bus return vaule is not valid.";
            return;
        }
    });
}

void SessionClient::actionInvokeSlot(quint32 id, QString actionkey) {
    if(actionkey == "kylin-os-manager-device-manager-close"){
        auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                      "/org/freedesktop/Notifications",
                                                      "org.freedesktop.Notifications",
                                                      "CloseNotification");
        QList<QVariant> args;
        args.push_back(id);
        message.setArguments(args);
        auto pending = QDBusConnection::sessionBus().asyncCall(message);
    }

    if (actionkey == "kylin-os-manager-device-manager-install"){
        connect(m_installer, &AptInstaller::succeed, this, &SessionClient::showSuccessMsg);
        connect(m_installer, &AptInstaller::failed, this, &SessionClient::showFailMsg);
        m_installer->startWorker();
        oninstall = m_driverInfo.split("=")[0];

        auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                      "/org/freedesktop/Notifications",
                                                      "org.freedesktop.Notifications",
                                                      "Notify");
        QList<QVariant> args;
        args.push_back(tr("Kylin Os Manager"));
        args.push_back((quint32)0);
        args.push_back("kylin-os-manager");
        args.push_back(tr("Device Manager"));
        args.push_back(tr("Start installing the corresponding driver"));
        args.push_back(QStringList());
        args.push_back(QMap<QString, QVariant>());
        args.push_back((qint32)-1);
        message.setArguments(args);

        auto pending = QDBusConnection::sessionBus().asyncCall(message);
        auto watcher = new QDBusPendingCallWatcher(pending);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
            watcher->deleteLater();
            if (!watcher->isValid() || watcher->isError()) {
                qCritical() << "Notify D-Bus share object is error.";
                return;
            }
            QDBusPendingReply<quint32> reply = *watcher;
            if (!reply.isValid() || reply.isError()) {
                qCritical() << "Notify D-Bus return vaule is not valid.";
                return;
            }
        });
    }
    if(actionkey == "kylin-os-manager-device-manager-open-scanner"){
        QProcess proc;
        proc.closeWriteChannel();
        proc.startDetached("kylin-scanner");
    }
}

void SessionClient::showSuccessMsg() {
    oninstall = "";
    auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  "Notify");
    QList<QVariant> args;
    args.push_back(tr("Kylin Os Manager"));
    args.push_back((quint32)0);
    args.push_back("kylin-os-manager");
    args.push_back(tr("Device Manager"));
    args.push_back(tr("Driver Installed"));
    if(m_devType == "scanner"){
        args.push_back(QStringList() << "kylin-os-manager-device-manager-open-scanner" << tr("Open Scanner"));
    }else{
        args.push_back(QStringList());
    }
    args.push_back(QMap<QString, QVariant>());
    args.push_back((qint32)-1);
    message.setArguments(args);

    auto pending = QDBusConnection::sessionBus().asyncCall(message);
    auto watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        if (!watcher->isValid() || watcher->isError()) {
            qCritical() << "Notify D-Bus share object is error.";
            return;
        }
        QDBusPendingReply<quint32> reply = *watcher;
        if (!reply.isValid() || reply.isError()) {
            qCritical() << "Notify D-Bus return vaule is not valid.";
            return;
        }
    });
    // 让服务端刷新设备信息
    if(DATABASE_INSTANCE.initDatabase() == DB_OP_SUCC){
        QStringList data;
        data.append(m_devType);
        data.append(m_devModel);
        data.append(m_driverInfo.split("=")[0]);
        data.append(m_driverInfo.split("=")[1]);
        data.append(m_driverSize);
        DATABASE_INSTANCE.insertDataSlot(data);
    }

    QDBusInterface iface(dbus::DBUS_SERVICENAME, dbus::DBUS_USB_HOT_INFO_TOOL, dbus::DBUS_USB_HOT_INFO_INTERFACE_TOOL, QDBusConnection::systemBus());
    if (iface.isValid()) {
        iface.call("installSucceed", m_driverInfo);
    } else {
        qWarning() << "Failed to connect to existing service";
    }
}

void SessionClient::showFailMsg() {
    oninstall = "";
    auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  "Notify");
    QList<QVariant> args;
    args.push_back(tr("Kylin Os Manager"));
    args.push_back((quint32)0);
    args.push_back("kylin-os-manager");
    args.push_back(tr("Device Manager"));
    args.push_back(tr("Driver install failed."));
    args.push_back(QStringList());
    args.push_back(QMap<QString, QVariant>());
    args.push_back((qint32)-1);
    message.setArguments(args);

    auto pending = QDBusConnection::sessionBus().asyncCall(message);
    auto watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        if (!watcher->isValid() || watcher->isError()) {
            qCritical() << "Notify D-Bus share object is error.";
            return;
        }
        QDBusPendingReply<quint32> reply = *watcher;
        if (!reply.isValid() || reply.isError()) {
            qCritical() << "Notify D-Bus return vaule is not valid.";
            return;
        }
    });
}
