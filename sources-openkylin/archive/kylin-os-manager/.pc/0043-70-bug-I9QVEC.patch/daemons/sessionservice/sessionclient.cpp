#include "sessionclient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

SessionClient::SessionClient(QObject *parent) : QObject(parent) {
    initHotplugClient();
}

void SessionClient::initHotplugClient() {
    // 连接信号槽
    QDBusConnection::systemBus().connect("com.drivermanager.USBMonitor",
                                         "/com/drivermanager/USBMonitor",
                                         "tools.tool", "newDeviceConnectSignal", this,
                                         SLOT(onNewDeviceConnectSlot(QString)));

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
    args.push_back(tr("Driver Manager"));
    args.push_back(tr("The system detects that a new device has an installable driver. Do you want to install and refresh the driver list?"));
    args.push_back(QStringList() << "close" << tr("Close") << "install" << tr("Install"));
    args.push_back(QMap<QString, QVariant>());
    args.push_back((qint32)-1);
    message.setArguments(args);

    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                         "/org/freedesktop/Notifications",
                                         "org.freedesktop.Notifications", "ActionInvoked", this,
                                         SLOT(actionInvokeSlot(quint32, QString)));

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
    if(actionkey == "close"){
        auto message = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                      "/org/freedesktop/Notifications",
                                                      "org.freedesktop.Notifications",
                                                      "CloseNotification");
        QList<QVariant> args;
        args.push_back(id);
        message.setArguments(args);
        auto pending = QDBusConnection::sessionBus().asyncCall(message);
    }

    if (actionkey == "install"){
        QProcess proc;
        QStringList options;
        options << "--jumpTab" << "DriverManager";
        proc.closeWriteChannel();
        proc.startDetached("kylin-os-manager",options);
    }
}
