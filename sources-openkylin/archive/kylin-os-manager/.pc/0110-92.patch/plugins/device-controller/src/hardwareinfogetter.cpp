#include "hardwareinfogetter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "include/globalsignal.h"
#include "include/common.h"
#include "kom-buriedpoint.h"

HardwareInfoGetter *HardwareInfoGetter::instance = new HardwareInfoGetter();

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
        return "get info error";
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
        graphicCard.subsystem = graphicCardJson["subsystem"].toString();
        graphicCard.memorysize = graphicCardJson["memorysize"].toString();
        graphicCard.businfo = graphicCardJson["businfo"].toString();
        graphicCard.clock = graphicCardJson["clock"].toString();
        graphicCard.physid = graphicCardJson["physid"].toString();
        graphicCard.version = graphicCardJson["version"].toString();
        graphicCard.width = graphicCardJson["width"].toString();
        graphicCard.gddrsize = graphicCardJson["gddrsize"].toString();
        graphicCard.egl_version = graphicCardJson["egl_version"].toString();
        graphicCard.egl_apis = graphicCardJson["egl_apis"].toString();
        graphicCard.gl_version = graphicCardJson["gl_version"].toString();
        graphicCard.glsl_version = graphicCardJson["glsl_version"].toString();

        QJsonArray driverArray = graphicCardJson["driverNameList"].toArray();

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

            graphicCard.driverNameList.append(driverInfo);
        }

        graphicCardList.append(graphicCard);
    }
    if (jsonString == "get info error") {
        m_getFailFlag++;
    } else {
        m_getFlag++;
        for (int i = 0; i < graphicCardList.length(); ++i) {
            uploadMapMessage(DeviceType::SCANNER, graphicCardList[i]);
        }
    }
    if (m_getFlag == 9) {
        emit g_signal->getHwInfoComplete();
        m_getsucceed = true;
    } else if ((m_getFailFlag + m_getFlag) == 9) {
        m_getsucceed = false;
        emit g_signal->getHwInfoComplete();
        emit g_signal->getHwInfoFailed();
    }
    m_graphicHardwares = graphicCardList;
    return graphicCardList;
}

QList<HardWareInfo> HardwareInfoGetter::getScannerHardwares() {
    QString info = getInfo(DeviceType::SCANNER);
    QList<HardWareInfo> hardwareList = convertJsonToHardWareList(info, DeviceType::SCANNER);

    if (info == "get info error") {
        m_getFailFlag++;
    } else {
        m_getFlag++;
        for (int i = 0; i < hardwareList.length(); ++i) {
            uploadMapMessage(DeviceType::SCANNER, hardwareList[i]);
        }
    }

    if (m_getFlag == 9) {
        m_getsucceed = true;
        emit g_signal->getHwInfoComplete();
    } else if ((m_getFailFlag + m_getFlag) == 9) {
        m_getsucceed = false;
        emit g_signal->getHwInfoComplete();
        emit g_signal->getHwInfoFailed();
    }
    m_scannerHardwares = hardwareList;
    return hardwareList;
}

QList<HardWareInfo> HardwareInfoGetter::getPrinterHardwares() {
    QString info = getInfo(DeviceType::PRINTER);
    QList<HardWareInfo> hardwareList = convertJsonToHardWareList(info, DeviceType::PRINTER);
    if (info == "get info error") {
        m_getFailFlag++;
    } else {
        for (int i = 0; i < hardwareList.length(); ++i) {
            uploadMapMessage(DeviceType::PRINTER, hardwareList[i]);
        }
        m_getFlag++;
    }

    if (m_getFlag == 9) {
        m_getsucceed = true;
        emit g_signal->getHwInfoComplete();
    } else if ((m_getFailFlag + m_getFlag) == 9) {
        m_getsucceed = false;
        emit g_signal->getHwInfoComplete();
        emit g_signal->getHwInfoFailed();
    }
    m_printerHardwares = hardwareList;
    return hardwareList;
}

QList<HardWareInfo> HardwareInfoGetter::getInputHardwares() {
    QString info = getInfo(DeviceType::InputDev);
    QList<HardWareInfo> hardwareList = convertJsonToHardWareList(info, DeviceType::InputDev);
    if (info == "get info error") {
        m_getFailFlag++;
    } else {
        for (int i = 0; i < hardwareList.length(); ++i) {
            uploadMapMessage(DeviceType::InputDev, hardwareList[i]);
        }
        m_getFlag++;
    }
    if (m_getFlag == 9) {
        m_getsucceed = true;
        emit g_signal->getHwInfoComplete();
    } else if ((m_getFailFlag + m_getFlag) == 9) {
        m_getsucceed = false;
        emit g_signal->getHwInfoComplete();
        emit g_signal->getHwInfoFailed();
    }
    m_inputHardwares = hardwareList;
    return hardwareList;
}

QList<HardWareInfo> HardwareInfoGetter::getVideoHardwares() {
    QString info = getInfo(DeviceType::VideoOrAudio);
    QList<HardWareInfo> hardwareList = convertJsonToHardWareList(info, DeviceType::VideoOrAudio);
    if (info == "get info error") {
        m_getFailFlag++;
    } else {
        for (int i = 0; i < hardwareList.length(); ++i) {
            uploadMapMessage(DeviceType::VideoOrAudio, hardwareList[i]);
        }
        m_getFlag++;
    }
    if (m_getFlag == 9) {
        m_getsucceed = true;
        emit g_signal->getHwInfoComplete();
    } else if ((m_getFailFlag + m_getFlag) == 9) {
        m_getsucceed = false;
        emit g_signal->getHwInfoComplete();
        emit g_signal->getHwInfoFailed();
    }
    m_videoHardwares = hardwareList;
    return hardwareList;
}

QList<HardWareInfo> HardwareInfoGetter::getOtherHardwares() {
    QString info = getInfo(DeviceType::Other);
    QList<HardWareInfo> hardwareList = convertJsonToHardWareList(info, DeviceType::Other);
    if (info == "get info error") {
        m_getFailFlag++;
    } else {
        for (int i = 0; i < hardwareList.length(); ++i) {
            uploadMapMessage(DeviceType::Other, hardwareList[i]);
        }
        m_getFlag++;
    }
    if (m_getFlag == 9) {
        m_getsucceed = true;
        emit g_signal->getHwInfoComplete();
    } else if ((m_getFailFlag + m_getFlag) == 9) {
        m_getsucceed = false;
        emit g_signal->getHwInfoComplete();
        emit g_signal->getHwInfoFailed();
    }
    m_otherHardwares = hardwareList;
    return hardwareList;
}

QList<HardWareInfo> HardwareInfoGetter::convertJsonToHardWareList(const QString &jsonString, DeviceType type) {
    QList<HardWareInfo> hardwareList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject hardwareJson = jsonValue.toObject();
        HardWareInfo hardwareInfo;
//        if (static_cast<DeviceType>(hardwareJson["type"].toInt()) != type) {
//            continue;
//        }
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

        QJsonObject modeinfoJson = bluetoothJson["modeinfo"].toArray().at(0).toObject();
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

            bluetooth.driverNameList.append(driverInfo);
        }

        bluetoothList.append(bluetooth);
    }
    if (jsonString == "get info error") {
        m_getFailFlag++;
    } else {
        m_getFlag++;
    }
    if (m_getFlag == 9) {
        m_getsucceed = true;
        emit g_signal->getHwInfoComplete();
    } else if ((m_getFailFlag + m_getFlag) == 9) {
        m_getsucceed = false;
        emit g_signal->getHwInfoComplete();
        emit g_signal->getHwInfoFailed();
    }
    m_bluetoothHardwares = bluetoothList;
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
        networkCard.isWireless = networkCardJson["isWireless"].toInt();
        networkCard.ip_addr = networkCardJson["ip_addr"].toString();
        networkCard.MAC_addr = networkCardJson["MAC_addr"].toString();
        networkCard.net_musk = networkCardJson["net_musk"].toString();
        networkCard.send_bytes = networkCardJson["send_bytes"].toString();
        networkCard.recv_bytes = networkCardJson["recv_bytes"].toString();
        networkCard.MTU = networkCardJson["MTU"].toString();
        networkCard.bus = networkCardJson["bus"].toString();

        QJsonObject modeinfoJson = networkCardJson["modeinfo"].toArray().at(0).toObject();
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

            networkCard.driverNameList.append(driverInfo);
        }

        networkCardList.append(networkCard);
    }
    if (jsonString == "get info error") {
        m_getFailFlag++;
    } else {
        m_getFlag++;
    }
    if (m_getFlag == 9) {
        m_getsucceed = true;
        emit g_signal->getHwInfoComplete();
    } else if ((m_getFailFlag + m_getFlag) == 9) {
        m_getsucceed = false;
        emit g_signal->getHwInfoComplete();
        emit g_signal->getHwInfoFailed();
    }
    m_networkCardHardwares = networkCardList;
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

        QJsonObject modeinfoJson = soundCardJson["modeinfo"].toArray().at(0).toObject();
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

            soundCard.driverNameList.append(driverInfo);
        }

        soundCardList.append(soundCard);
    }
    if (jsonString == "get info error") {
        m_getFailFlag++;
    } else {
        m_getFlag++;
    }
    if (m_getFlag == 9) {
        m_getsucceed = true;
        emit g_signal->getHwInfoComplete();
    } else if ((m_getFailFlag + m_getFlag) == 9) {
        m_getsucceed = false;
        emit g_signal->getHwInfoComplete();
        emit g_signal->getHwInfoFailed();
    }
    m_soundCardHardwares = soundCardList;
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

    m_pcinfo = pcInfo;
    return pcInfo;
}

QList<CpuInfo> HardwareInfoGetter::getCpuInfo() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        response = manager.call("getCpuInfo");
        result = response.arguments().at(0).toString();
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject cpuJson = jsonValue.toObject();
        CpuInfo cpuinfo;

        cpuinfo.vendor = cpuJson["vendor"].toString();
        cpuinfo.model = cpuJson["model"].toString();
        cpuinfo.arch = cpuJson["arch"].toString();
        cpuinfo.cores = cpuJson["cores"].toString();
        cpuinfo.onlinecores = cpuJson["onlinecores"].toString();
        cpuinfo.threadprecore = cpuJson["threadprecore"].toString();
        cpuinfo.maxMHz = cpuJson["maxMHz"].toString();
        cpuinfo.L1dcache = cpuJson["L1dcache"].toString();
        cpuinfo.L1icache = cpuJson["L1icache"].toString();
        cpuinfo.L2cache = cpuJson["L2cache"].toString();
        cpuinfo.L3cache = cpuJson["L3cache"].toString();
        cpuinfo.L4cache = cpuJson["L4cache"].toString();

        m_cpuinfo.append(cpuinfo);
    }

    return m_cpuinfo;
}

QList<MemoryInfo> HardwareInfoGetter::getMemoryInfo() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        response = manager.call("getMemoryInfo");
        result = response.arguments().at(0).toString();
    }

    QList<MemoryInfo> memoryinfoList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject memoryJson = jsonValue.toObject();
        MemoryInfo memory;

        memory.locator = memoryJson["locator"].toString();
        memory.totalwidth = memoryJson["totalwidth"].toString();
        memory.size = memoryJson["size"].toString();
        memory.serial = memoryJson["serial"].toString();
        memory.verdor = memoryJson["verdor"].toString();
        memory.datawidth = memoryJson["datawidth"].toString();
        memory.type = memoryJson["type"].toString();
        memory.speed = memoryJson["speed"].toString();
        memory.configuredspeed = memoryJson["configuredspeed"].toString();
        memory.banklocator = memoryJson["banklocator"].toString();
        memory.partnumber = memoryJson["partnumber"].toString();
        memory.displayname = memoryJson["displayname"].toString();

        memoryinfoList.append(memory);
    }

    m_memoryinfoList = memoryinfoList;
    return m_memoryinfoList;
}

QList<BaseboardInfo> HardwareInfoGetter::getBaseboardInfo() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        response = manager.call("getBaseboardInfo");
        result = response.arguments().at(0).toString();
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject baseboardJson = jsonValue.toObject();
        BaseboardInfo baseboardinfo;

        baseboardinfo.vendor = baseboardJson["vendor"].toString();
        baseboardinfo.model = baseboardJson["model"].toString();
        baseboardinfo.version = baseboardJson["version"].toString();
        baseboardinfo.chassishandle = baseboardJson["chassishandle"].toString();
        baseboardinfo.biosvendor = baseboardJson["biosvendor"].toString();
        baseboardinfo.biosversion = baseboardJson["biosversion"].toString();
        baseboardinfo.chipset = baseboardJson["chipset"].toString();
        baseboardinfo.serial_num = baseboardJson["serial_num"].toString();
        baseboardinfo.publish_date = baseboardJson["publish_date"].toString();

        m_baseboardinfo.append(baseboardinfo);
    }

    return m_baseboardinfo;
}

QList<DiskInfo> HardwareInfoGetter::getDiskInfo() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        response = manager.call("getDiskInfo");
        result = response.arguments().at(0).toString();
    }

    QList<DiskInfo> diskinfoList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject memoryJson = jsonValue.toObject();
        DiskInfo disk;

        disk.model = memoryJson["model"].toString();
        disk.vendor = memoryJson["vendor"].toString();
        disk.size = memoryJson["size"].toString();
        disk.serial = memoryJson["serial"].toString();
        disk.name = memoryJson["name"].toString();
        disk.frimwareversion = memoryJson["frimwareversion"].toString();
        disk.type = memoryJson["type"].toString();
        disk.interface = memoryJson["interface"].toString();
        disk.is_main = memoryJson["is_main"].toInt();
        disk.ufs = memoryJson["ufs"].toString();
        disk.ufsrevision = memoryJson["ufsrevision"].toString();

        diskinfoList.append(disk);
    }

    m_diskinfoList = diskinfoList;
    return diskinfoList;
}

void HardwareInfoGetter::refreshDiskInfo() {
    m_oldDiskInfoList.clear();
    m_oldDiskInfoList.append(m_diskinfoList);
    m_diskinfoList.clear();
    getDiskInfo();
}

void HardwareInfoGetter::genMonitorInfoDataFile() {
    QString strMonitorInfoFile = "/tmp/youker-assistant-monitorinfo.dat";
    QFile tmpFile(strMonitorInfoFile);
    if (tmpFile.exists()) {
        tmpFile.remove();
    }
    QString cmd = "xrandr --prop";
    QProcess *p = new QProcess();
    p->start(cmd);
    p->waitForFinished();
    // 将monitor信息写入临时文件
    if (!tmpFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream tsFile(&tmpFile);
    while (p->canReadLine()) {
        QString str = p->readLine();
        str = str.left(str.length() - 1);
        tsFile << str << endl;  //写入文件
    }
    tmpFile.close();
    delete p;
}

QList<MonitorInfo> HardwareInfoGetter::getMonitorInfo() {
    genMonitorInfoDataFile();

    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        response = manager.call("getMonitorInfo");
        result = response.arguments().at(0).toString();
    }

    QList<MonitorInfo> monitorinfoList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject memoryJson = jsonValue.toObject();
        MonitorInfo monitor;

        monitor.vendor = memoryJson["vendor"].toString();
        monitor.name = memoryJson["name"].toString();
        monitor.size = memoryJson["size"].toString() + tr(" inch");
        monitor.ratio = memoryJson["ratio"].toString();
        monitor.resolution = memoryJson["resolution"].toString();
        monitor.max_resolution = memoryJson["max_resolution"].toString();
        monitor.is_main = memoryJson["is_main"].toString();
        monitor.gamma = memoryJson["gamma"].toString();
        monitor.interface = memoryJson["interface"].toString();
        monitor.model = memoryJson["model"].toString();
        monitor.visible_area = memoryJson["visible_area"].toString();
        monitor.year = memoryJson["year"].toString();
        monitor.week = memoryJson["week"].toString();

        monitorinfoList.append(monitor);
    }

    m_monitorList = monitorinfoList;
    return monitorinfoList;
}

void HardwareInfoGetter::refreshMonitorInfo() {
    genMonitorInfoDataFile();
    m_oldMonitorInfoList.clear();
    m_oldMonitorInfoList.append(m_monitorList);
    m_monitorList.clear();
    getMonitorInfo();
}


HardwareInfoGetter *HardwareInfoGetter::getInstance() {
    return instance;
}

QList<KeyboardInfo> HardwareInfoGetter::getKeyboardInfo() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        response = manager.call("getKeyboardInfo");
        result = response.arguments().at(0).toString();
    }

    QList<KeyboardInfo> keyboardinfoList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject memoryJson = jsonValue.toObject();
        KeyboardInfo keyboardInfo;

        keyboardInfo.vendor = memoryJson["vendor"].toString();
        keyboardInfo.name = memoryJson["name"].toString();
        keyboardInfo.devtype = memoryJson["devtype"].toString();
        keyboardInfo.interface = memoryJson["interface"].toString();
        keyboardInfo.model = memoryJson["model"].toString();
        keyboardInfo.driver = memoryJson["driver"].toString();

        keyboardinfoList.append(keyboardInfo);
    }

    m_keyboardInfoList = keyboardinfoList;
    return keyboardinfoList;
}

void HardwareInfoGetter::refreshKeyboardInfo() {
    m_oldKeyboardInfoList.clear();
    m_oldKeyboardInfoList.append(m_keyboardInfoList);
    m_keyboardInfoList.clear();
    getKeyboardInfo();
}

QList<MouseInfo> HardwareInfoGetter::getMouseInfo() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        response = manager.call("getMouseInfo");
        result = response.arguments().at(0).toString();
    }

    QList<MouseInfo> mouseinfoList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject memoryJson = jsonValue.toObject();
        MouseInfo mouseInfo;

        mouseInfo.vendor = memoryJson["vendor"].toString();
        mouseInfo.name = memoryJson["name"].toString();
        mouseInfo.devtype = memoryJson["devtype"].toString();
        mouseInfo.interface = memoryJson["interface"].toString();
        mouseInfo.model = memoryJson["model"].toString();
        mouseInfo.driver = memoryJson["driver"].toString();

        mouseinfoList.append(mouseInfo);
    }

    m_mouseInfoList = mouseinfoList;
    return mouseinfoList;
}

void HardwareInfoGetter::refreshMouseInfo() {
    m_oldMouseInfoList.clear();
    m_oldMouseInfoList.append(m_mouseInfoList);
    m_mouseInfoList.clear();
    getMouseInfo();
}

void HardwareInfoGetter::refreshBluetoothInfo() {
    m_oldBluetoothInfoList.clear();
    m_oldBluetoothInfoList.append(m_bluetoothHardwares);
    m_bluetoothHardwares.clear();
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    response = manager.call("redetectBluetoothDev");
    regetBluetoothHardwares();
}

void HardwareInfoGetter::refreshNetworkInfo() {
    m_oldNetworkInfoList.clear();
    m_oldNetworkInfoList.append(m_networkCardHardwares);
    m_networkCardHardwares.clear();
    regetNetworkHardwares();
}

void HardwareInfoGetter::refreshCdInfo() {
    m_oldCdInfoList.clear();
    m_oldCdInfoList.append(m_cdInfoList);
    m_cdInfoList.clear();
    getCDInfo();
}

QList<CDInfo> HardwareInfoGetter::getCDInfo() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        response = manager.call("getCDInfo");
        result = response.arguments().at(0).toString();
    }

    QList<CDInfo> cdinfoList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject memoryJson = jsonValue.toObject();
        CDInfo cdInfo;

        cdInfo.vendor = memoryJson["vendor"].toString();
        cdInfo.model = memoryJson["model"].toString();
        cdInfo.businfo = memoryJson["businfo"].toString();
        cdInfo.name = memoryJson["name"].toString();
        cdInfo.version = memoryJson["version"].toString();
        cdInfo.devicenum = memoryJson["devicenum"].toString();
        cdInfo.driver = memoryJson["driver"].toString();
        cdInfo.speed = memoryJson["speed"].toString();
        cdInfo.serialnum = memoryJson["serialnum"].toString();

        cdinfoList.append(cdInfo);
    }

    m_cdInfoList = cdinfoList;
    return cdinfoList;
}

QList<CameraInfo> HardwareInfoGetter::getCameraInfo() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        response = manager.call("getCameraInfo");
        result = response.arguments().at(0).toString();
    }

    QList<CameraInfo> camerainfoList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject memoryJson = jsonValue.toObject();
        CameraInfo cameraInfo;

        cameraInfo.vendor = memoryJson["vendor"].toString();
        cameraInfo.model = memoryJson["model"].toString();
        cameraInfo.businfo = memoryJson["businfo"].toString();
        cameraInfo.name = memoryJson["name"].toString();
        cameraInfo.version = memoryJson["version"].toString();
        cameraInfo.interface = memoryJson["interface"].toString();
        cameraInfo.driver = memoryJson["driver"].toString();
        cameraInfo.speed = memoryJson["speed"].toString();

        camerainfoList.append(cameraInfo);
    }

    m_cameraInfoList = camerainfoList;
    return camerainfoList;
}

QList<FanInfo> HardwareInfoGetter::getFanInfo() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        response = manager.call("getFanInfo");
        result = response.arguments().at(0).toString();
    }

    QList<FanInfo> faninfoList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject memoryJson = jsonValue.toObject();
        FanInfo fanInfo;

        fanInfo.name = memoryJson["name"].toString();
        fanInfo.speed = memoryJson["speed"].toString();

        faninfoList.append(fanInfo);
    }

    m_fanInfoList = faninfoList;
    return faninfoList;
}

QList<BatteryInfo> HardwareInfoGetter::getBatteryInfo() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;
    if (manager.isValid()) {
        response = manager.call("getBatteryInfo");
        result = response.arguments().at(0).toString();
    }

    QList<BatteryInfo> batteryinfoList;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray jsonArray = jsonDoc.array();

    for (const QJsonValue &jsonValue: jsonArray) {
        QJsonObject memoryJson = jsonValue.toObject();
        BatteryInfo batteryInfo;

        batteryInfo.vendor = memoryJson["vendor"].toString();
        batteryInfo.model = memoryJson["model"].toString();
        batteryInfo.serailnum = memoryJson["serailnum"].toString();
        batteryInfo.state = memoryJson["state"].toString();
        batteryInfo.energy = memoryJson["energy"].toString();
        batteryInfo.used_times = memoryJson["used_times"].toString();
        batteryInfo.energy_full = memoryJson["energy_full"].toString();
        batteryInfo.time_to_empty = memoryJson["time_to_empty"].toString();
        batteryInfo.percentage = memoryJson["percentage"].toString();

        batteryinfoList.append(batteryInfo);
    }

    m_batteryInfoList = batteryinfoList;
    return batteryinfoList;
}

void HardwareInfoGetter::refreshFlag() {
    m_getFlag = 0;
    m_getFailFlag = 0;
}

void HardwareInfoGetter::uploadMapMessage(DeviceType type, HardWareInfo result) {
    QMap<QString, QString> buried_data;
    buried_data.insert("Vendor", result.vendor);
    buried_data.insert("model", result.model);
    buried_data.insert("name", result.name);
    buried_data.insert("driver_now", result.driver_now);
    buried_data.insert("driver_version_now", result.driver_version_now);
    buried_data.insert("VID", result.VID);
    buried_data.insert("PID", result.PID);
    if (result.driverNameList.length() != 0) {
        buried_data.insert("ResultDriver", result.driverNameList[0].driverName);
        buried_data.insert("ResultDriverVersion", result.driverNameList[0].version);
    } else {
        buried_data.insert("ResultDriver", "");
        buried_data.insert("ResultDriverVersion", "");
    }

    switch (type) {
        case DeviceType::GraphicCard:
            buried_data.insert("event_code", "GraphicCard Get Result");
//            kom::BuriedPoint::uploadMessage(kom::PluginsDriverManager, buried_data);
            kom::BuriedPoint::uploadMessage(kom::PluginsDriverManager, "GraphicCard Get Result", buried_data);
            break;
        case DeviceType::PRINTER:
            buried_data.insert("event_code", "PRINTER Get Result");
//            kom::BuriedPoint::uploadMessage(kom::PluginsDriverManager, buried_data);
            kom::BuriedPoint::uploadMessage(kom::PluginsDriverManager, "PRINTER Get Result", buried_data);
            break;
        case DeviceType::SCANNER:
            buried_data.insert("event_code", "SCANNER Get Result");
//            kom::BuriedPoint::uploadMessage(kom::PluginsDriverManager, buried_data);
            kom::BuriedPoint::uploadMessage(kom::PluginsDriverManager, "SCANNER Get Result", buried_data);
            break;
        case DeviceType::VideoOrAudio:
            buried_data.insert("event_code", "VideoOrAudio Get Result");
//            kom::BuriedPoint::uploadMessage(kom::PluginsDriverManager, buried_data);
            kom::BuriedPoint::uploadMessage(kom::PluginsDriverManager, "VideoOrAudio Get Result", buried_data);
            break;
        case DeviceType::Other:
            buried_data.insert("event_code", "OtherHW Get Result");
//            kom::BuriedPoint::uploadMessage(kom::PluginsDriverManager, buried_data);
            kom::BuriedPoint::uploadMessage(kom::PluginsDriverManager, "OtherHW Get Result", buried_data);
            break;
        case DeviceType::InputDev:
            buried_data.insert("event_code", "InputDevHW Get Result");
//            kom::BuriedPoint::uploadMessage(kom::PluginsDriverManager, buried_data);
            kom::BuriedPoint::uploadMessage(kom::PluginsDriverManager, "InputDevHW Get Result", buried_data);
            break;//
    }
}

void HardwareInfoGetter::refreshServerInfo(DeviceType deviceType) {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin-driver-installer", "/tools", "tools.tool", bus);
    QString result;
    QDBusMessage response;

    switch (deviceType) {
        case DeviceType::SCANNER:
            response = manager.call("redetectScanner");
            break;
        case DeviceType::GraphicCard:
            response = manager.call("redetectGraphicCard");
            break;
        case DeviceType::PRINTER:
            response = manager.call("redetectPrinter");
            break;
        case DeviceType::InputDev:
            response = manager.call("redetectInputDev");
            break;
        case DeviceType::VideoOrAudio:
            response = manager.call("redetectVideoDev");
            break;
        case DeviceType::Other:
            response = manager.call("redetectOtherDev");
            break;
    }
}

QList<BluetoothInfo> HardwareInfoGetter::regetBluetoothHardwares() {
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

        QJsonObject modeinfoJson = bluetoothJson["modeinfo"].toArray().at(0).toObject();
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

            bluetooth.driverNameList.append(driverInfo);
        }

        bluetoothList.append(bluetooth);
    }
    m_bluetoothHardwares = bluetoothList;
    return bluetoothList;
}

QList<NetworkCardInfo> HardwareInfoGetter::regetNetworkHardwares() {
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
        networkCard.isWireless = networkCardJson["isWireless"].toInt();
        networkCard.ip_addr = networkCardJson["ip_addr"].toString();
        networkCard.MAC_addr = networkCardJson["MAC_addr"].toString();
        networkCard.net_musk = networkCardJson["net_musk"].toString();
        networkCard.send_bytes = networkCardJson["send_bytes"].toString();
        networkCard.recv_bytes = networkCardJson["recv_bytes"].toString();
        networkCard.MTU = networkCardJson["MTU"].toString();
        networkCard.bus = networkCardJson["bus"].toString();

        QJsonObject modeinfoJson = networkCardJson["modeinfo"].toArray().at(0).toObject();
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

            networkCard.driverNameList.append(driverInfo);
        }

        networkCardList.append(networkCard);
    }

    m_networkCardHardwares = networkCardList;

    return networkCardList;
}
