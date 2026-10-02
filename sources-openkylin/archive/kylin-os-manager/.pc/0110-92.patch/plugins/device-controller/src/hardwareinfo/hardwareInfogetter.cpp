#include "hardwareinfogetter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../include/common.h"

HardwareInfoGetter::HardwareInfoGetter(QObject *parent) : QObject(parent) {

}

QString HardwareInfoGetter::getInfo(DeviceType type) {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        switch (type) {
            case DeviceType::PRINTER:
                response = manager.call("getPrinterDriver");
                result = response.arguments().at(0).toString();
                break;
            case DeviceType::SCANNER:
                response = manager.call("getScannerDriver");
                result = response.arguments().at(0).toString();
                break;
            case DeviceType::GraphicCard:
                response = manager.call("getGraphicCardDriver");
                result = response.arguments().at(0).toString();
                break;
            case DeviceType::SoundCard:
                response = manager.call("getSoundcardInfo");
                result = response.arguments().at(0).toString();
                break;
            case DeviceType::NetworkCard:
                response = manager.call("getNetWorkcardInfo");
                result = response.arguments().at(0).toString();
                break;
            case DeviceType::Bluetooth:
                response = manager.call("getBluetoothInfo");
                result = response.arguments().at(0).toString();
                break;
            case DeviceType::InputDev:
                response = manager.call("getInputDeviceDriver");
                result = response.arguments().at(0).toString();
                break;
            case DeviceType::VideoOrAudio:
                response = manager.call("getVideoDeviceDriver");
                result = response.arguments().at(0).toString();
                break;
            case DeviceType::Other:
                response = manager.call("getOtherDeviceDriver");
                result = response.arguments().at(0).toString();
                break;
            default:
                break;
        }
        return result;
    } else {
        return "";
    }
}

QList<GraphicCardInfo> HardwareInfoGetter::getGraphicHardwares() {
    QString jsonString = getInfo(DeviceType::GraphicCard);

    QList<GraphicCardInfo> graphicCardList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject graphicCardJson = jsonValue.toObject();
        GraphicCardInfo graphicCard;
        if (static_cast<DeviceType>(graphicCardJson["type"].toInt()) != DeviceType::GraphicCard) {
            continue;
        }
        graphicCard.type = static_cast<DeviceType>(graphicCardJson["type"].toInt());
        graphicCard.vendor = graphicCardJson["vendor"].toString();
        graphicCard.model = graphicCardJson["model"].toString();
        graphicCard.serial = graphicCardJson["serial"].toString();
        graphicCard.name = graphicCardJson["name"].toString();
        graphicCard.driver_now = graphicCardJson["driver_now"].toString();
        graphicCard.driver_version_now = graphicCardJson["driver_version_now"].toString();
        graphicCard.sysPath = graphicCardJson["sysPath"].toString();
        graphicCard.devicePath = graphicCardJson["devicePath"].toString();
        graphicCard.busNumber = graphicCardJson["busNumber"].toString();
        graphicCard.deviceNumber = graphicCardJson["deviceNumber"].toString();
        graphicCard.VID = graphicCardJson["VID"].toString();
        graphicCard.PID = graphicCardJson["PID"].toString();
        graphicCard.upgradeable = graphicCardJson["upgradeable"].toBool();
        graphicCard.modalias = graphicCardJson["modalias"].toString();

        QJsonArray driverArray = graphicCardJson["driverNameList"].toArray();
        QList<DriverInfo> driverList;

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

            driverList.append(driverInfo);
        }

        graphicCard.driverNameList = driverList;
        graphicCardList.append(graphicCard);
    }
    emit getHwSucceed();
    return graphicCardList;
}

QList<HardWareInfo> HardwareInfoGetter::getScannerHardwares() {
    QString info = getInfo(DeviceType::SCANNER);
    QList<HardWareInfo> hardwareList = convertJsonToHardWareList(info, DeviceType::SCANNER);
    emit getHwSucceed();
    return hardwareList;
}

QList<HardWareInfo> HardwareInfoGetter::getPrinterHardwares() {
    QString info = getInfo(DeviceType::PRINTER);
    QList<HardWareInfo> hardwareList = convertJsonToHardWareList(info, DeviceType::PRINTER);
    emit getHwSucceed();
    return hardwareList;
}

QList<HardWareInfo> HardwareInfoGetter::getInputHardwares() {
    QString info = getInfo(DeviceType::InputDev);
    QList<HardWareInfo> hardwareList = convertJsonToHardWareList(info, DeviceType::InputDev);
    emit getHwSucceed();
    return hardwareList;
}

QList<HardWareInfo> HardwareInfoGetter::getVideoHardwares() {
    QString info = getInfo(DeviceType::VideoOrAudio);
    QList<HardWareInfo> hardwareList = convertJsonToHardWareList(info, DeviceType::VideoOrAudio);
    emit getHwSucceed();
    return hardwareList;
}

QList<HardWareInfo> HardwareInfoGetter::getOtherHardwares() {
    QString info = getInfo(DeviceType::Other);
    QList<HardWareInfo> hardwareList = convertJsonToHardWareList(info, DeviceType::Other);
    emit getHwSucceed();
    return hardwareList;
}

QList<HardWareInfo> HardwareInfoGetter::convertJsonToHardWareList(const QString &jsonString, DeviceType type) {
    QList<HardWareInfo> hardwareList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject hardwareJson = jsonValue.toObject();
        HardWareInfo hardwareInfo;
        if (static_cast<DeviceType>(hardwareJson["type"].toInt()) != type) {
            continue;
        }
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

        QJsonArray driverArray = hardwareJson["driverNameList"].toArray();
        QList<DriverInfo> driverList;

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

            driverList.append(driverInfo);
        }

        hardwareInfo.driverNameList = driverList;
        hardwareList.append(hardwareInfo);
    }
    return hardwareList;
}


QList<BluetoothInfo> HardwareInfoGetter::getBluetoothHardwares() {
    QString jsonString = getInfo(DeviceType::Bluetooth);

    QList<BluetoothInfo> bluetoothList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject bluetoothJson = jsonValue.toObject();
        BluetoothInfo bluetooth;
        if (static_cast<DeviceType>(bluetoothJson["type"].toInt()) != DeviceType::Bluetooth) {
            continue;
        }
        bluetooth.type = static_cast<DeviceType>(bluetoothJson["type"].toInt());
        bluetooth.vendor = bluetoothJson["vendor"].toString();
        bluetooth.model = bluetoothJson["model"].toString();
        bluetooth.serial = bluetoothJson["serial"].toString();
        bluetooth.name = bluetoothJson["name"].toString();
        bluetooth.driver_now = bluetoothJson["driver_now"].toString();
        bluetooth.driver_version_now = bluetoothJson["driver_version_now"].toString();
        bluetooth.sysPath = bluetoothJson["sysPath"].toString();
        bluetooth.devicePath = bluetoothJson["devicePath"].toString();
        bluetooth.busNumber = bluetoothJson["busNumber"].toString();
        bluetooth.deviceNumber = bluetoothJson["deviceNumber"].toString();
        bluetooth.VID = bluetoothJson["VID"].toString();
        bluetooth.PID = bluetoothJson["PID"].toString();
        bluetooth.upgradeable = bluetoothJson["upgradeable"].toBool();
        bluetooth.address = bluetoothJson["address"].toString();
        bluetooth.alcmtu = bluetoothJson["alcmtu"].toString();
        bluetooth.bus = bluetoothJson["bus"].toString();
        bluetooth.dev_version = bluetoothJson["dev_version"].toString();
        bluetooth.features = bluetoothJson["features"].toString();
        bluetooth.link_mode = bluetoothJson["link_mode"].toString();
        bluetooth.link_policy = bluetoothJson["link_policy"].toString();
        bluetooth.dev_name = bluetoothJson["dev_name"].toString();
        bluetooth.packettype = bluetoothJson["packettype"].toString();
        bluetooth.scomtu = bluetoothJson["scomtu"].toString();
        bluetooth.speed = bluetoothJson["speed"].toString();

        QJsonObject modeinfoJson = bluetoothJson["modeinfo"].toObject();
        bluetooth.modeinfo.driverName = modeinfoJson["driverName"].toString();
        bluetooth.modeinfo.filename = modeinfoJson["filename"].toString();
        bluetooth.modeinfo.license = modeinfoJson["license"].toString();
        bluetooth.modeinfo.description = modeinfoJson["description"].toString();
        bluetooth.modeinfo.author = modeinfoJson["author"].toString();
        bluetooth.modeinfo.version = modeinfoJson["version"].toString();
        bluetooth.modeinfo.srcversion = modeinfoJson["srcversion"].toString();
        bluetooth.modeinfo.alias = modeinfoJson["alias"].toString().split(" ");
        bluetooth.modeinfo.depends = modeinfoJson["depends"].toString();
        bluetooth.modeinfo.softdep = modeinfoJson["softdep"].toString();
        bluetooth.modeinfo.vermagic = modeinfoJson["vermagic"].toString();

        QJsonArray driverArray = bluetoothJson["driverNameList"].toArray();
        QList<DriverInfo> driverList;

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

            driverList.append(driverInfo);
        }

        bluetooth.driverNameList = driverList;
        bluetoothList.append(bluetooth);
    }
    emit getHwSucceed();
    return bluetoothList;
}

QList<NetworkCardInfo> HardwareInfoGetter::getNetworkHardwares() {
    QString jsonString = getInfo(DeviceType::NetworkCard);

    QList<NetworkCardInfo> networkCardList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject networkCardJson = jsonValue.toObject();
        NetworkCardInfo networkCard;
        if (static_cast<DeviceType>(networkCardJson["type"].toInt()) != DeviceType::NetworkCard) {
            continue;
        }
        networkCard.type = static_cast<DeviceType>(networkCardJson["type"].toInt());
        networkCard.vendor = networkCardJson["vendor"].toString();
        networkCard.model = networkCardJson["model"].toString();
        networkCard.serial = networkCardJson["serial"].toString();
        networkCard.name = networkCardJson["name"].toString();
        networkCard.driver_now = networkCardJson["driver_now"].toString();
        networkCard.driver_version_now = networkCardJson["driver_version_now"].toString();
        networkCard.sysPath = networkCardJson["sysPath"].toString();
        networkCard.devicePath = networkCardJson["devicePath"].toString();
        networkCard.busNumber = networkCardJson["busNumber"].toString();
        networkCard.deviceNumber = networkCardJson["deviceNumber"].toString();
        networkCard.VID = networkCardJson["VID"].toString();
        networkCard.PID = networkCardJson["PID"].toString();
        networkCard.upgradeable = networkCardJson["upgradeable"].toBool();
        networkCard.isWireless = networkCardJson["isWireless"].toBool();
        networkCard.ip_addr = networkCardJson["ip_addr"].toString();
        networkCard.MAC_addr = networkCardJson["MAC_addr"].toString();

        QJsonObject modeinfoJson = networkCardJson["modeinfo"].toObject();
        networkCard.modeinfo.driverName = modeinfoJson["driverName"].toString();
        networkCard.modeinfo.filename = modeinfoJson["filename"].toString();
        networkCard.modeinfo.license = modeinfoJson["license"].toString();
        networkCard.modeinfo.description = modeinfoJson["description"].toString();
        networkCard.modeinfo.author = modeinfoJson["author"].toString();
        networkCard.modeinfo.version = modeinfoJson["version"].toString();
        networkCard.modeinfo.srcversion = modeinfoJson["srcversion"].toString();
        networkCard.modeinfo.alias = modeinfoJson["alias"].toString().split(" ");
        networkCard.modeinfo.depends = modeinfoJson["depends"].toString();
        networkCard.modeinfo.softdep = modeinfoJson["softdep"].toString();
        networkCard.modeinfo.vermagic = modeinfoJson["vermagic"].toString();

        QJsonArray driverArray = networkCardJson["driverNameList"].toArray();
        QList<DriverInfo> driverList;

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

            driverList.append(driverInfo);
        }

        networkCard.driverNameList = driverList;
        networkCardList.append(networkCard);
    }
    emit getHwSucceed();
    return networkCardList;
}

QList<SoundCardInfo> HardwareInfoGetter::getSoundcardHardwares() {
    QString jsonString = getInfo(DeviceType::SoundCard);

    QList<SoundCardInfo> soundCardList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject soundCardJson = jsonValue.toObject();
        SoundCardInfo soundCard;
        if (static_cast<DeviceType>(soundCardJson["type"].toInt()) != DeviceType::SoundCard) {
            continue;
        }
        soundCard.type = static_cast<DeviceType>(soundCardJson["type"].toInt());
        soundCard.vendor = soundCardJson["vendor"].toString();
        soundCard.model = soundCardJson["model"].toString();
        soundCard.serial = soundCardJson["serial"].toString();
        soundCard.name = soundCardJson["name"].toString();
        soundCard.driver_now = soundCardJson["driver_now"].toString();
        soundCard.driver_version_now = soundCardJson["driver_version_now"].toString();
        soundCard.sysPath = soundCardJson["sysPath"].toString();
        soundCard.devicePath = soundCardJson["devicePath"].toString();
        soundCard.busNumber = soundCardJson["busNumber"].toString();
        soundCard.deviceNumber = soundCardJson["deviceNumber"].toString();
        soundCard.VID = soundCardJson["VID"].toString();
        soundCard.PID = soundCardJson["PID"].toString();
        soundCard.upgradeable = soundCardJson["upgradeable"].toBool();
        soundCard.bus = soundCardJson["bus"].toString();
        soundCard.clock = soundCardJson["clock"].toString();
        soundCard.width = soundCardJson["width"].toString();

        QJsonObject modeinfoJson = soundCardJson["modeinfo"].toObject();
        soundCard.modeinfo.driverName = modeinfoJson["driverName"].toString();
        soundCard.modeinfo.filename = modeinfoJson["filename"].toString();
        soundCard.modeinfo.license = modeinfoJson["license"].toString();
        soundCard.modeinfo.description = modeinfoJson["description"].toString();
        soundCard.modeinfo.author = modeinfoJson["author"].toString();
        soundCard.modeinfo.version = modeinfoJson["version"].toString();
        soundCard.modeinfo.srcversion = modeinfoJson["srcversion"].toString();
        soundCard.modeinfo.alias = modeinfoJson["alias"].toString().split(" ");
        soundCard.modeinfo.depends = modeinfoJson["depends"].toString();
        soundCard.modeinfo.softdep = modeinfoJson["softdep"].toString();
        soundCard.modeinfo.vermagic = modeinfoJson["vermagic"].toString();

        QJsonArray driverArray = soundCardJson["driverNameList"].toArray();
        QList<DriverInfo> driverList;

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

            driverList.append(driverInfo);
        }

        soundCard.driverNameList = driverList;
        soundCardList.append(soundCard);
    }
    emit getHwSucceed();
    return soundCardList;
}

PcInfo HardwareInfoGetter::getPcInfo() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        response = manager.call("getPcInfo");
        result = response.arguments().at(0).toString();
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    QJsonObject jsonObject = jsonDoc.array().at(0).toObject();

    PcInfo pcInfo;

    pcInfo.hostname = jsonObject.value("hostname").toString();
    pcInfo.sysversion = jsonObject.value("sysversion").toString();
    pcInfo.kernelversion = jsonObject.value("kernelversion").toString();
    pcInfo.arch = jsonObject.value("arch").toString();
    pcInfo.sysbits = jsonObject.value("sysbits").toString();
    pcInfo.manufacturer = jsonObject.value("manufacturer").toString();
    pcInfo.version = jsonObject.value("version").toString();
    pcInfo.model = jsonObject.value("model").toString();
    pcInfo.serial = jsonObject.value("serial").toString();

    return pcInfo;
}
