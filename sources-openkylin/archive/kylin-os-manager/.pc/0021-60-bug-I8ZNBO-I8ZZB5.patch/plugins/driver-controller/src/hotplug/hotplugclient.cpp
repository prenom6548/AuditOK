#include "hotplugclient.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../include/globalsignal.h"

HotplugClient::HotplugClient(QObject *parent) : QObject(parent) {
    QDBusConnection bus = QDBusConnection::systemBus();
    if(!bus.connect("", "/com/drivermanager/USBMonitor", "tools.tool", "newDeviceConnectSignal", this,
                    SLOT(plugInEvent(const QString)))){
        qDebug() << "fail to connect hotplug server";
    }

    if(!bus.connect("", "/com/drivermanager/USBMonitor", "tools.tool", "deviceRemoveSignal", this,
                    SLOT(plugoutEvent(const QString)))){
        qDebug() << "fail to connect hotplug server";
    }

}

void HotplugClient::plugoutEvent(const QString jsonString) {
    qDebug() << "A New Dev Plug Out :" << jsonString;
    auto hardwares = convertJsonToHardWareList(jsonString);
    emit g_signal->removeItem(hardwares);

}

void HotplugClient::plugInEvent(const QString jsonString) {
    qDebug() << "A New Dev Plug In :" << jsonString;
    auto hardwares = convertJsonToHardWareList(jsonString);
    emit g_signal->addItem(hardwares);
}

QList<HardWareInfo> HotplugClient::convertJsonToHardWareList(const QString &jsonString) {
    QList<HardWareInfo> hardwareList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject hardwareJson = jsonValue.toObject();
        HardWareInfo hardwareInfo;

        hardwareInfo.type = static_cast<DeviceType>(hardwareJson["type"].toInt());
        hardwareInfo.vendor = hardwareJson["vendor"].toString();
        hardwareInfo.model = hardwareJson["model"].toString();
        hardwareInfo.serial = hardwareJson["serial"].toString();
        hardwareInfo.name = hardwareJson["name"].toString();
        hardwareInfo.driver_now = hardwareJson["driver_now"].toString();
        hardwareInfo.driver_version_now = hardwareJson["driver_version_now"].toString();
        hardwareInfo.sysPath = hardwareJson["sysPath"].toString();
        hardwareInfo.devicePath = hardwareJson["devicePath"].toString();
        hardwareInfo.busNumber = hardwareJson["busNumber"].toString();
        hardwareInfo.deviceNumber = hardwareJson["deviceNumber"].toString();
        hardwareInfo.VID = hardwareJson["VID"].toString();
        hardwareInfo.PID = hardwareJson["PID"].toString();
        hardwareInfo.upgradeable = hardwareJson["upgradeable"].toBool();
        hardwareInfo.isOnline = hardwareJson["isOnline"].toBool();

        QJsonArray driverArray = hardwareJson["driverNameList"].toArray();

        for (const QJsonValue &driverValue: driverArray) {
            QJsonObject driverObject = driverValue.toObject();
            DriverInfo driverInfo;

            driverInfo.driverName = driverObject["driverName"].toString();
            driverInfo.arch = driverObject["arch"].toString();
            driverInfo.version = driverObject["version"].toString();
            driverInfo.priority = driverObject["priority"].toString();
            driverInfo.section = driverObject["section"].toString();
            driverInfo.source = driverObject["source"].toString();
            driverInfo.origin = driverObject["origin"].toString();
            driverInfo.maintainer = driverObject["maintainer"].toString();
            driverInfo.bugs = driverObject["bugs"].toString();
            driverInfo.installedSize = driverObject["installedSize"].toString();
            driverInfo.depends = driverObject["depends"].toString();
            driverInfo.recommends = driverObject["recommends"].toString();
            driverInfo.filename = driverObject["filename"].toString();
            driverInfo.size = driverObject["size"].toString();
            driverInfo.modaliases = driverObject["modaliases"].toVariant().toStringList();

            hardwareInfo.driverNameList.append(driverInfo);
        }

        hardwareList.append(hardwareInfo);
    }
    return hardwareList;
}