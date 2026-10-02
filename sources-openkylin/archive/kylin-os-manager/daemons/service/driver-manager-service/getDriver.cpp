#include "getDriver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QProcess>
#include "include/globalsignal.h"

getDriver *getDriver::instance = new getDriver();
static QString getPackageVersion(const QString &packageName);
static QString getRetFromCommand(const QStringList &command);
static bool checkInstallable(const QString name, const QString version);

getDriver::getDriver(QObject *parent) : QObject(parent) {
}

QString getDriver::getPcInfo() {
    QEventLoop loop;
    m_pcInfoGetter = new PcIfoGetter();
    connect(m_pcInfoGetter, &PcIfoGetter::succeed, [=]() {
        m_pcInfo = m_pcInfoGetter->getPcInfo();
    });
    QObject::connect(m_pcInfoGetter, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_pcInfoGetter, SIGNAL(failed()), &loop, SLOT(quit()));
    m_pcInfoGetter->startWorker();
    loop.exec();
    return convertClassToString(m_pcInfo);
}

QString getDriver::getCpuInfo() {
    QEventLoop loop;
    m_cpuInfoGetter = new CpuInfoFinder();
    connect(m_cpuInfoGetter, &CpuInfoFinder::succeed, [=]() {
        m_cpuInfo = m_cpuInfoGetter->getCpuInfo();
    });
    QObject::connect(m_cpuInfoGetter, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_cpuInfoGetter, SIGNAL(failed()), &loop, SLOT(quit()));
    m_cpuInfoGetter->startWorker();
    loop.exec();
    return convertClassToString(m_cpuInfo);
}

QString getDriver::getMemoryInfo() {
    QEventLoop loop;
    m_memoryInfoGetter = new MemoryInfoFinder();
    connect(m_memoryInfoGetter, &MemoryInfoFinder::succeed, [=]() {
        m_memoryInfo = m_memoryInfoGetter->getMemoryInfo();
    });
    QObject::connect(m_memoryInfoGetter, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_memoryInfoGetter, SIGNAL(failed()), &loop, SLOT(quit()));
    m_memoryInfoGetter->startWorker();
    loop.exec();
    return convertClassToString(m_memoryInfo);
}

QString getDriver::getBaseboardInfo() {
    QEventLoop loop;
    m_baseboardInfoGetter = new BaseboardInfoFinder();
    connect(m_baseboardInfoGetter, &BaseboardInfoFinder::succeed, [=]() {
        m_baseboardInfo = m_baseboardInfoGetter->getBaseboardInfo();
    });
    QObject::connect(m_baseboardInfoGetter, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_baseboardInfoGetter, SIGNAL(failed()), &loop, SLOT(quit()));
    m_baseboardInfoGetter->startWorker();
    loop.exec();
    return convertClassToString(m_baseboardInfo);
}

QString getDriver::getDiskInfo() {
    QEventLoop loop;
    m_diskInfoGetter = new DiskInfoFinder();
    connect(m_diskInfoGetter, &DiskInfoFinder::succeed, [=]() {
        m_diskInfo = m_diskInfoGetter->getDiskInfo();
    });
    QObject::connect(m_diskInfoGetter, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_diskInfoGetter, SIGNAL(failed()), &loop, SLOT(quit()));
    m_diskInfoGetter->startWorker();
    loop.exec();
    return convertClassToString(m_diskInfo);
}

QString getDriver::getMonitorInfo() {
    QEventLoop loop;
    m_monitorInfoGetter = new MonitorInfoFinder();
    connect(m_monitorInfoGetter, &MonitorInfoFinder::succeed, [=]() {
        m_monitorInfo = m_monitorInfoGetter->getMonitorList();
    });
    QObject::connect(m_monitorInfoGetter, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_monitorInfoGetter, SIGNAL(failed()), &loop, SLOT(quit()));
    m_monitorInfoGetter->startWorker();
    loop.exec();
    return convertClassToString(m_monitorInfo);
}

QString getDriver::getKeyboardInfo() {
    QEventLoop loop;
    m_keyboardInfoGetter = new KeyboardInfoFinder();
    connect(m_keyboardInfoGetter, &KeyboardInfoFinder::succeed, [=]() {
        m_keyboardInfo = m_keyboardInfoGetter->getKeyBoardList();
    });
    QObject::connect(m_keyboardInfoGetter, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_keyboardInfoGetter, SIGNAL(failed()), &loop, SLOT(quit()));
    m_keyboardInfoGetter->startWorker();
    loop.exec();
    return convertClassToString(m_keyboardInfo);
}

QString getDriver::getMouseInfo() {
    QEventLoop loop;
    m_mouseInfoGetter = new MouseInfoFinder();
    connect(m_mouseInfoGetter, &MouseInfoFinder::succeed, [=]() {
        m_mouseInfo = m_mouseInfoGetter->getMouseList();
    });
    QObject::connect(m_mouseInfoGetter, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_mouseInfoGetter, SIGNAL(failed()), &loop, SLOT(quit()));
    m_mouseInfoGetter->startWorker();
    loop.exec();
    return convertClassToString(m_mouseInfo);
}

QString getDriver::getCameraInfo() {
    QEventLoop loop;
    m_cameraInfoGetter = new CameraInfoFinder();
    connect(m_cameraInfoGetter, &CameraInfoFinder::succeed, [=]() {
        m_cameraInfo = m_cameraInfoGetter->getCameraList();
    });
    QObject::connect(m_cameraInfoGetter, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_cameraInfoGetter, SIGNAL(failed()), &loop, SLOT(quit()));
    m_cameraInfoGetter->startWorker();
    loop.exec();
    return convertClassToString(m_cameraInfo);
}

QString getDriver::getBatteryInfo() {
    QEventLoop loop;
    m_batteryInfoGetter = new BatteryInfoFinder();
    connect(m_batteryInfoGetter, &BatteryInfoFinder::succeed, [=]() {
        m_batteryInfo = m_batteryInfoGetter->getBatteryList();
    });
    QObject::connect(m_batteryInfoGetter, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_batteryInfoGetter, SIGNAL(failed()), &loop, SLOT(quit()));
    m_batteryInfoGetter->startWorker();
    loop.exec();
    return convertClassToString(m_batteryInfo);
}

QString getDriver::getCDInfo() {
    QEventLoop loop;
    m_cdInfoGetter = new CDInfoFinder();
    connect(m_cdInfoGetter, &CDInfoFinder::succeed, [=]() {
        m_CDInfo = m_cdInfoGetter->getCDList();
    });
    QObject::connect(m_cdInfoGetter, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_cdInfoGetter, SIGNAL(failed()), &loop, SLOT(quit()));
    m_cdInfoGetter->startWorker();
    loop.exec();
    return convertClassToString(m_CDInfo);
}

QString getDriver::getFanInfo() {
    QEventLoop loop;
    m_fanInfoGetter = new FanInfoFinder();
    connect(m_fanInfoGetter, &FanInfoFinder::succeed, [=]() {
        m_FanInfo = m_fanInfoGetter->getFanList();
    });
    QObject::connect(m_fanInfoGetter, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_fanInfoGetter, SIGNAL(failed()), &loop, SLOT(quit()));
    m_fanInfoGetter->startWorker();
    loop.exec();
    return convertClassToString(m_FanInfo);
}


QString getDriver::getGraphicCardDriver() {
    QEventLoop loop;
    m_graphicDriverFinder = new graphicDriverFinder();
    connect(m_graphicDriverFinder, &graphicDriverFinder::succeed, [=]() {
        m_graphicHardwares = m_graphicDriverFinder->getHardWareInfo();
    });
    QObject::connect(m_graphicDriverFinder, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_graphicDriverFinder, SIGNAL(failed()), &loop, SLOT(quit()));
    m_graphicDriverFinder->startWorker();
    loop.exec();
    return convertClassToString(m_graphicHardwares);
}

QString getDriver::getScannerDriver() {
    QEventLoop loop;
    m_scannerDriverFinder = new scannerDriverFinder;
    connect(m_scannerDriverFinder, &scannerDriverFinder::succeed, [=]() {
        m_scannerHardwares = m_scannerDriverFinder->getScannerInfoList();
    });
    QObject::connect(m_scannerDriverFinder, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_scannerDriverFinder, SIGNAL(failed()), &loop, SLOT(quit()));
    m_scannerDriverFinder->startWorker();
    loop.exec();
    return convertClassToString(m_scannerHardwares);
}

QString getDriver::getPrinterDriver() {
    QEventLoop loop;
    m_printerDriverFinder = new printerDriverFinder();
    connect(m_printerDriverFinder, &printerDriverFinder::succeed, [=]() {
        m_printerHardwares = m_printerDriverFinder->getPrinterInfoList();
    });
    QObject::connect(m_printerDriverFinder, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_printerDriverFinder, SIGNAL(failed()), &loop, SLOT(quit()));
    m_printerDriverFinder->startWorker();
    loop.exec();
    return convertClassToString(m_printerHardwares);
}

QString getDriver::getBluetoothInfo() {
    QEventLoop loop;
    m_bluetoothFinder = new bluetoothFinder();
    connect(m_bluetoothFinder, &bluetoothFinder::succeed, [=]() {
        m_bluetoothHardwares = m_bluetoothFinder->getBluetoothDeviceList();
    });
    QObject::connect(m_bluetoothFinder, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_bluetoothFinder, SIGNAL(failed()), &loop, SLOT(quit()));
    m_bluetoothFinder->startWorker();
    loop.exec();
    return convertClassToString(m_bluetoothHardwares);
}

QString getDriver::getSoundcardInfo() {
    QEventLoop loop;
    m_soundCardFinder = new SoundCardFinder();
    connect(m_soundCardFinder, &SoundCardFinder::succeed, [=]() {
        m_soundCardHardwares = m_soundCardFinder->getSoundCardList();
    });
    QObject::connect(m_soundCardFinder, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_soundCardFinder, SIGNAL(failed()), &loop, SLOT(quit()));
    m_soundCardFinder->startWorker();
    loop.exec();
    return convertClassToString(m_soundCardHardwares);
}

QString getDriver::getNetWorkcardInfo() {
    QEventLoop loop;
    m_networkCardFinder = new NetworkCardFinder();
    connect(m_networkCardFinder, &NetworkCardFinder::succeed, [=]() {
        m_networkCardHardwares = m_networkCardFinder->getNetworkDeviceList();
    });
    QObject::connect(m_networkCardFinder, SIGNAL(succeed()), &loop, SLOT(quit()));
    QObject::connect(m_networkCardFinder, SIGNAL(failed()), &loop, SLOT(quit()));
    m_networkCardFinder->startWorker();
    loop.exec();
    return convertClassToString(m_networkCardHardwares);
}

QString getDriver::getInputDeviceDriver() {
    //    QEventLoop loop;
    //    m_inputDevFinder = new inputdevicedriverfinder;
    //    connect(m_inputDevFinder, &inputdevicedriverfinder::succeed, [=]() {
    //        m_inputHardwares = m_inputDevFinder->getInputDevInfoList();
    //    });
    //    QObject::connect(m_inputDevFinder, SIGNAL(succeed()), &loop, SLOT(quit()));
    //    m_inputDevFinder->startWorker();
    //    loop.exec();
    //    return convertClassToString(m_inputHardwares);
    return "";
}

QString getDriver::getVideoDeviceDriver() {
    //    QEventLoop loop;
    //    m_videoDriverFinder = new videoDriverFinder();
    //    connect(m_videoDriverFinder, &videoDriverFinder::succeed, [=]() {
    //        m_videoHardwares = m_videoDriverFinder->getVideoInfoList();
    //    });
    //    QObject::connect(m_videoDriverFinder, SIGNAL(succeed()), &loop, SLOT(quit()));
    //    m_videoDriverFinder->startWorker();
    //    loop.exec();
    //    return convertClassToString(m_videoHardwares);
    return "";
}

QString getDriver::getOtherDeviceDriver() {
    //    QEventLoop loop;
    //    m_otherDevDriverFinder = new otherDeviceDriverFinder();
    //    connect(m_otherDevDriverFinder, &otherDeviceDriverFinder::succeed, [=]() {
    //        m_otherHardwares = m_otherDevDriverFinder->getOtherDevInfoList();
    //    });
    //    QObject::connect(m_otherDevDriverFinder, SIGNAL(succeed()), &loop, SLOT(quit()));
    //    m_otherDevDriverFinder->startWorker();
    //    loop.exec();
    //    return convertClassToString(m_otherHardwares);
    return "";
}

QString getDriver::convertClassToString(QList<HardWareInfo> hardwareList) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: hardwareList) {
        QJsonObject jsonObject;
        jsonObject["type"] = static_cast<int>(hardwareInfo.type);
        jsonObject["vendor"] = hardwareInfo.vendor;
        jsonObject["model"] = hardwareInfo.model;
        jsonObject["serial"] = hardwareInfo.serial;
        jsonObject["name"] = hardwareInfo.name;
        jsonObject["driver_now"] = hardwareInfo.driver_now;
        jsonObject["driver_version_now"] = hardwareInfo.driver_version_now;
        jsonObject["sysPath"] = hardwareInfo.sysPath;
        jsonObject["devicePath"] = hardwareInfo.devicePath;
        jsonObject["busNumber"] = hardwareInfo.busNumber;
        jsonObject["deviceNumber"] = hardwareInfo.deviceNumber;
        jsonObject["VID"] = hardwareInfo.VID;
        jsonObject["PID"] = hardwareInfo.PID;
        jsonObject["upgradeable"] = hardwareInfo.upgradeable;
        jsonObject["isOnline"] = hardwareInfo.isOnline;

        QJsonArray driverArray;
        for (const DriverInfo &driverInfo: hardwareInfo.driverNameList) {
            QJsonObject driverObject;
            driverObject["driverName"] = driverInfo.driverName;
            driverObject["arch"] = driverInfo.arch;
            driverObject["version"] = driverInfo.version;
            driverObject["priority"] = driverInfo.priority;
            driverObject["section"] = driverInfo.section;
            driverObject["source"] = driverInfo.source;
            driverObject["origin"] = driverInfo.origin;
            driverObject["maintainer"] = driverInfo.maintainer;
            driverObject["bugs"] = driverInfo.bugs;
            driverObject["installedSize"] = driverInfo.installedSize;
            driverObject["depends"] = driverInfo.depends;
            driverObject["recommends"] = driverInfo.recommends;
            driverObject["filename"] = driverInfo.filename;
            driverObject["size"] = driverInfo.size;
            driverObject["modaliases"] = QJsonArray::fromStringList(driverInfo.modaliases);
            driverArray.append(driverObject);
        }

        jsonObject["driverNameList"] = driverArray;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<GraphicCardInfo> graphicHardwareList) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: graphicHardwareList) {
        QJsonObject jsonObject;
        jsonObject["type"] = static_cast<int>(hardwareInfo.type);
        jsonObject["vendor"] = hardwareInfo.vendor;
        jsonObject["model"] = hardwareInfo.model;
        jsonObject["serial"] = hardwareInfo.serial;
        jsonObject["name"] = hardwareInfo.name;
        jsonObject["driver_now"] = hardwareInfo.driver_now;
        jsonObject["driver_version_now"] = hardwareInfo.driver_version_now;
        jsonObject["sysPath"] = hardwareInfo.sysPath;
        jsonObject["devicePath"] = hardwareInfo.devicePath;
        jsonObject["busNumber"] = hardwareInfo.busNumber;
        jsonObject["deviceNumber"] = hardwareInfo.deviceNumber;
        jsonObject["VID"] = hardwareInfo.VID;
        jsonObject["PID"] = hardwareInfo.PID;
        jsonObject["upgradeable"] = hardwareInfo.upgradeable;
        jsonObject["modalias"] = hardwareInfo.modalias;
        jsonObject["subsystem"] = hardwareInfo.subsystem;
        jsonObject["memorysize"] = hardwareInfo.memorysize;
        jsonObject["businfo"] = hardwareInfo.businfo;
        jsonObject["clock"] = hardwareInfo.clock;
        jsonObject["physid"] = hardwareInfo.physid;
        jsonObject["version"] = hardwareInfo.version;
        jsonObject["width"] = hardwareInfo.width;
        jsonObject["gddrsize"] = hardwareInfo.gddrsize;
        jsonObject["egl_version"] = hardwareInfo.egl_version;
        jsonObject["egl_apis"] = hardwareInfo.egl_apis;
        jsonObject["gl_version"] = hardwareInfo.gl_version;
        jsonObject["glsl_version"] = hardwareInfo.glsl_version;

        QJsonArray driverArray;
        for (const DriverInfo &driverInfo: hardwareInfo.driverNameList) {
            QJsonObject driverObject;
            driverObject["driverName"] = driverInfo.driverName;
            driverObject["arch"] = driverInfo.arch;
            driverObject["version"] = driverInfo.version;
            driverObject["priority"] = driverInfo.priority;
            driverObject["section"] = driverInfo.section;
            driverObject["source"] = driverInfo.source;
            driverObject["origin"] = driverInfo.origin;
            driverObject["maintainer"] = driverInfo.maintainer;
            driverObject["bugs"] = driverInfo.bugs;
            driverObject["installedSize"] = driverInfo.installedSize;
            driverObject["depends"] = driverInfo.depends;
            driverObject["recommends"] = driverInfo.recommends;
            driverObject["filename"] = driverInfo.filename;
            driverObject["size"] = driverInfo.size;
            driverObject["modaliases"] = QJsonArray::fromStringList(driverInfo.modaliases);
            driverArray.append(driverObject);
        }

        jsonObject["driverNameList"] = driverArray;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<BluetoothInfo> bluetoothHardwareList) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: bluetoothHardwareList) {
        QJsonObject jsonObject;
        jsonObject["type"] = static_cast<int>(hardwareInfo.type);
        jsonObject["vendor"] = hardwareInfo.vendor;
        jsonObject["model"] = hardwareInfo.model;
        jsonObject["serial"] = hardwareInfo.serial;
        jsonObject["name"] = hardwareInfo.name;
        jsonObject["driver_now"] = hardwareInfo.driver_now;
        jsonObject["driver_version_now"] = hardwareInfo.driver_version_now;
        jsonObject["sysPath"] = hardwareInfo.sysPath;
        jsonObject["devicePath"] = hardwareInfo.devicePath;
        jsonObject["busNumber"] = hardwareInfo.busNumber;
        jsonObject["deviceNumber"] = hardwareInfo.deviceNumber;
        jsonObject["VID"] = hardwareInfo.VID;
        jsonObject["PID"] = hardwareInfo.PID;
        jsonObject["upgradeable"] = hardwareInfo.upgradeable;
        jsonObject["address"] = hardwareInfo.address;
        jsonObject["alcmtu"] = hardwareInfo.alcmtu;
        jsonObject["bus"] = hardwareInfo.bus;
        jsonObject["dev_version"] = hardwareInfo.dev_version;
        jsonObject["features"] = hardwareInfo.features;
        jsonObject["link_mode"] = hardwareInfo.link_mode;
        jsonObject["link_policy"] = hardwareInfo.link_policy;
        jsonObject["dev_name"] = hardwareInfo.dev_name;
        jsonObject["packettype"] = hardwareInfo.packettype;
        jsonObject["scomtu"] = hardwareInfo.scomtu;
        jsonObject["speed"] = hardwareInfo.speed;
        // modinfo 输出内容
        QJsonArray modeinfoArray;
        QJsonObject modeinfoObject;
        modeinfoObject["driverName"] = hardwareInfo.modeinfo.driverName;
        modeinfoObject["filename"] = hardwareInfo.modeinfo.filename;
        modeinfoObject["license"] = hardwareInfo.modeinfo.license;
        modeinfoObject["description"] = hardwareInfo.modeinfo.description;
        modeinfoObject["author"] = hardwareInfo.modeinfo.author;
        modeinfoObject["version"] = hardwareInfo.modeinfo.version;
        modeinfoObject["srcversion"] = hardwareInfo.modeinfo.srcversion;
        QString alias = "";
        for (int i = 0; i < hardwareInfo.modeinfo.alias.length(); ++i) {
            alias += hardwareInfo.modeinfo.alias[i];
            alias += " ";
        }
        modeinfoObject["alias"] = alias;
        modeinfoObject["depends"] = hardwareInfo.modeinfo.depends;
        modeinfoObject["softdep"] = hardwareInfo.modeinfo.softdep;
        modeinfoObject["vermagic"] = hardwareInfo.modeinfo.vermagic;
        modeinfoArray.append(modeinfoObject);
        jsonObject["modeinfo"] = modeinfoArray;

        QJsonArray driverArray;
        for (const DriverInfo &driverInfo: hardwareInfo.driverNameList) {
            QJsonObject driverObject;
            driverObject["driverName"] = driverInfo.driverName;
            driverObject["arch"] = driverInfo.arch;
            driverObject["version"] = driverInfo.version;
            driverObject["priority"] = driverInfo.priority;
            driverObject["section"] = driverInfo.section;
            driverObject["source"] = driverInfo.source;
            driverObject["origin"] = driverInfo.origin;
            driverObject["maintainer"] = driverInfo.maintainer;
            driverObject["bugs"] = driverInfo.bugs;
            driverObject["installedSize"] = driverInfo.installedSize;
            driverObject["depends"] = driverInfo.depends;
            driverObject["recommends"] = driverInfo.recommends;
            driverObject["filename"] = driverInfo.filename;
            driverObject["size"] = driverInfo.size;
            driverObject["modaliases"] = QJsonArray::fromStringList(driverInfo.modaliases);
            driverArray.append(driverObject);
        }

        jsonObject["driverNameList"] = driverArray;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<SoundCardInfo> soundCardHardwareList) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: soundCardHardwareList) {
        QJsonObject jsonObject;
        jsonObject["type"] = static_cast<int>(hardwareInfo.type);
        jsonObject["vendor"] = hardwareInfo.vendor;
        jsonObject["model"] = hardwareInfo.model;
        jsonObject["serial"] = hardwareInfo.serial;
        jsonObject["name"] = hardwareInfo.name;
        jsonObject["driver_now"] = hardwareInfo.driver_now;
        jsonObject["driver_version_now"] = hardwareInfo.driver_version_now;
        jsonObject["sysPath"] = hardwareInfo.sysPath;
        jsonObject["devicePath"] = hardwareInfo.devicePath;
        jsonObject["busNumber"] = hardwareInfo.busNumber;
        jsonObject["deviceNumber"] = hardwareInfo.deviceNumber;
        jsonObject["VID"] = hardwareInfo.VID;
        jsonObject["PID"] = hardwareInfo.PID;
        jsonObject["upgradeable"] = hardwareInfo.upgradeable;
        jsonObject["bus"] = hardwareInfo.bus;
        jsonObject["clock"] = hardwareInfo.clock;
        jsonObject["width"] = hardwareInfo.width;
        // modinfo 输出内容
        QJsonArray modeinfoArray;
        QJsonObject modeinfoObject;
        modeinfoObject["driverName"] = hardwareInfo.modeinfo.driverName;
        modeinfoObject["filename"] = hardwareInfo.modeinfo.filename;
        modeinfoObject["license"] = hardwareInfo.modeinfo.license;
        modeinfoObject["description"] = hardwareInfo.modeinfo.description;
        modeinfoObject["author"] = hardwareInfo.modeinfo.author;
        modeinfoObject["version"] = hardwareInfo.modeinfo.version;
        modeinfoObject["srcversion"] = hardwareInfo.modeinfo.srcversion;
        QString alias = "";
        for (int i = 0; i < hardwareInfo.modeinfo.alias.length(); ++i) {
            alias += hardwareInfo.modeinfo.alias[i];
            alias += " ";
        }
        modeinfoObject["alias"] = alias;
        modeinfoObject["depends"] = hardwareInfo.modeinfo.depends;
        modeinfoObject["softdep"] = hardwareInfo.modeinfo.softdep;
        modeinfoObject["vermagic"] = hardwareInfo.modeinfo.vermagic;
        modeinfoArray.append(modeinfoObject);
        jsonObject["modeinfo"] = modeinfoArray;

        QJsonArray driverArray;
        for (const DriverInfo &driverInfo: hardwareInfo.driverNameList) {
            QJsonObject driverObject;
            driverObject["driverName"] = driverInfo.driverName;
            driverObject["arch"] = driverInfo.arch;
            driverObject["version"] = driverInfo.version;
            driverObject["priority"] = driverInfo.priority;
            driverObject["section"] = driverInfo.section;
            driverObject["source"] = driverInfo.source;
            driverObject["origin"] = driverInfo.origin;
            driverObject["maintainer"] = driverInfo.maintainer;
            driverObject["bugs"] = driverInfo.bugs;
            driverObject["installedSize"] = driverInfo.installedSize;
            driverObject["depends"] = driverInfo.depends;
            driverObject["recommends"] = driverInfo.recommends;
            driverObject["filename"] = driverInfo.filename;
            driverObject["size"] = driverInfo.size;
            driverObject["modaliases"] = QJsonArray::fromStringList(driverInfo.modaliases);
            driverArray.append(driverObject);
        }

        jsonObject["driverNameList"] = driverArray;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<NetworkCardInfo> networkCardHardwareList) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: networkCardHardwareList) {
        QJsonObject jsonObject;
        jsonObject["type"] = static_cast<int>(hardwareInfo.type);
        jsonObject["vendor"] = hardwareInfo.vendor;
        jsonObject["model"] = hardwareInfo.model;
        jsonObject["serial"] = hardwareInfo.serial;
        jsonObject["name"] = hardwareInfo.name;
        jsonObject["driver_now"] = hardwareInfo.driver_now;
        jsonObject["driver_version_now"] = hardwareInfo.driver_version_now;
        jsonObject["sysPath"] = hardwareInfo.sysPath;
        jsonObject["devicePath"] = hardwareInfo.devicePath;
        jsonObject["busNumber"] = hardwareInfo.busNumber;
        jsonObject["deviceNumber"] = hardwareInfo.deviceNumber;
        jsonObject["VID"] = hardwareInfo.VID;
        jsonObject["PID"] = hardwareInfo.PID;
        jsonObject["upgradeable"] = hardwareInfo.upgradeable;
        jsonObject["isWireless"] = hardwareInfo.isWireless;
        jsonObject["ip_addr"] = hardwareInfo.ip_addr;
        jsonObject["MAC_addr"] = hardwareInfo.MAC_addr;
        jsonObject["net_musk"] = hardwareInfo.net_musk;
        jsonObject["send_bytes"] = hardwareInfo.send_bytes;
        jsonObject["recv_bytes"] = hardwareInfo.recv_bytes;
        jsonObject["MTU"] = hardwareInfo.MTU;
        jsonObject["size"] = hardwareInfo.size;
        jsonObject["bus"] = hardwareInfo.bus;
        // modinfo 输出内容
        QJsonArray modeinfoArray;
        QJsonObject modeinfoObject;
        modeinfoObject["driverName"] = hardwareInfo.modeinfo.driverName;
        modeinfoObject["filename"] = hardwareInfo.modeinfo.filename;
        modeinfoObject["license"] = hardwareInfo.modeinfo.license;
        modeinfoObject["description"] = hardwareInfo.modeinfo.description;
        modeinfoObject["author"] = hardwareInfo.modeinfo.author;
        modeinfoObject["version"] = hardwareInfo.modeinfo.version;
        modeinfoObject["srcversion"] = hardwareInfo.modeinfo.srcversion;
        QString alias = "";
        for (int i = 0; i < hardwareInfo.modeinfo.alias.length(); ++i) {
            alias += hardwareInfo.modeinfo.alias[i];
            alias += " ";
        }
        modeinfoObject["alias"] = alias;
        modeinfoObject["depends"] = hardwareInfo.modeinfo.depends;
        modeinfoObject["softdep"] = hardwareInfo.modeinfo.softdep;
        modeinfoObject["vermagic"] = hardwareInfo.modeinfo.vermagic;
        modeinfoArray.append(modeinfoObject);
        jsonObject["modeinfo"] = modeinfoArray;

        QJsonArray driverArray;
        for (const DriverInfo &driverInfo: hardwareInfo.driverNameList) {
            QJsonObject driverObject;
            driverObject["driverName"] = driverInfo.driverName;
            driverObject["arch"] = driverInfo.arch;
            driverObject["version"] = driverInfo.version;
            driverObject["priority"] = driverInfo.priority;
            driverObject["section"] = driverInfo.section;
            driverObject["source"] = driverInfo.source;
            driverObject["origin"] = driverInfo.origin;
            driverObject["maintainer"] = driverInfo.maintainer;
            driverObject["bugs"] = driverInfo.bugs;
            driverObject["installedSize"] = driverInfo.installedSize;
            driverObject["depends"] = driverInfo.depends;
            driverObject["recommends"] = driverInfo.recommends;
            driverObject["filename"] = driverInfo.filename;
            driverObject["size"] = driverInfo.size;
            driverObject["modaliases"] = QJsonArray::fromStringList(driverInfo.modaliases);
            driverArray.append(driverObject);
        }

        jsonObject["driverNameList"] = driverArray;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(PcInfo pcinfo) {
    QJsonArray jsonArray;

    QJsonObject jsonObject;
    jsonObject["hostname"] = pcinfo.hostname;
    jsonObject["sysversion"] = pcinfo.sysversion;
    jsonObject["kernelversion"] = pcinfo.kernelversion;
    jsonObject["arch"] = pcinfo.arch;
    jsonObject["sysbits"] = pcinfo.sysbits;
    jsonObject["manufacturer"] = pcinfo.manufacturer;
    jsonObject["version"] = pcinfo.version;
    jsonObject["model"] = pcinfo.model;
    jsonObject["serial"] = pcinfo.serial;
    jsonObject["yearversion"] = pcinfo.yearversion;

    jsonArray.append(jsonObject);

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(CpuInfo cpuinfo) {
    QJsonArray jsonArray;

    QJsonObject jsonObject;
    jsonObject["vendor"] = cpuinfo.vendor;
    jsonObject["model"] = cpuinfo.model;
    jsonObject["arch"] = cpuinfo.arch;
    jsonObject["cores"] = cpuinfo.cores;
    jsonObject["onlinecores"] = cpuinfo.onlinecores;
    jsonObject["threadprecore"] = cpuinfo.threadprecore;
    jsonObject["maxMHz"] = cpuinfo.maxMHz;
    jsonObject["L1dcache"] = cpuinfo.L1dcache;
    jsonObject["L1icache"] = cpuinfo.L1icache;
    jsonObject["L2cache"] = cpuinfo.L2cache;
    jsonObject["L3cache"] = cpuinfo.L3cache;
    jsonObject["L4cache"] = cpuinfo.L4cache;

    jsonArray.append(jsonObject);

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<MemoryInfo> memoryInfoList) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: memoryInfoList) {
        QJsonObject jsonObject;
        jsonObject["locator"] = hardwareInfo.locator;
        jsonObject["totalwidth"] = hardwareInfo.totalwidth;
        jsonObject["size"] = hardwareInfo.size;
        jsonObject["serial"] = hardwareInfo.serial;
        jsonObject["verdor"] = hardwareInfo.verdor;
        jsonObject["datawidth"] = hardwareInfo.datawidth;
        jsonObject["type"] = hardwareInfo.type;
        jsonObject["speed"] = hardwareInfo.speed;
        jsonObject["configuredspeed"] = hardwareInfo.configuredspeed;
        jsonObject["banklocator"] = hardwareInfo.banklocator;
        jsonObject["partnumber"] = hardwareInfo.partnumber;
        jsonObject["displayname"] = hardwareInfo.displayname;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(BaseboardInfo baseboardinfo) {
    QJsonArray jsonArray;

    QJsonObject jsonObject;
    jsonObject["vendor"] = baseboardinfo.vendor;
    jsonObject["model"] = baseboardinfo.model;
    jsonObject["version"] = baseboardinfo.version;
    jsonObject["chassishandle"] = baseboardinfo.chassishandle;
    jsonObject["biosvendor"] = baseboardinfo.biosvendor;
    jsonObject["biosversion"] = baseboardinfo.biosversion;
    jsonObject["chipset"] = baseboardinfo.chipset;
    jsonObject["serial_num"] = baseboardinfo.serial_num;
    jsonObject["publish_date"] = baseboardinfo.publish_date;

    jsonArray.append(jsonObject);

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<DiskInfo> diskInfolist) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: diskInfolist) {
        QJsonObject jsonObject;
        jsonObject["model"] = hardwareInfo.model;
        jsonObject["vendor"] = hardwareInfo.vendor;
        jsonObject["name"] = hardwareInfo.name;
        jsonObject["size"] = hardwareInfo.size;
        jsonObject["type"] = hardwareInfo.type;
        jsonObject["serial"] = hardwareInfo.serial;
        jsonObject["frimwareversion"] = hardwareInfo.frimwareversion;
        jsonObject["is_main"] = hardwareInfo.is_main;
        jsonObject["interface"] = hardwareInfo.interface;
        jsonObject["ufs"] = hardwareInfo.ufs;
        jsonObject["ufsrevision"] = hardwareInfo.ufsrevision;
        jsonObject["rota"] = hardwareInfo.rota;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<MonitorInfo> monitorInfolist) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: monitorInfolist) {
        QJsonObject jsonObject;
        jsonObject["vendor"] = hardwareInfo.vendor;
        jsonObject["name"] = hardwareInfo.name;
        jsonObject["size"] = hardwareInfo.size;
        jsonObject["ratio"] = hardwareInfo.ratio;
        jsonObject["resolution"] = hardwareInfo.resolution;
        jsonObject["max_resolution"] = hardwareInfo.max_resolution;
        jsonObject["is_main"] = hardwareInfo.is_main;
        jsonObject["gamma"] = hardwareInfo.gamma;
        jsonObject["interface"] = hardwareInfo.interface;
        jsonObject["model"] = hardwareInfo.model;
        jsonObject["visible_area"] = hardwareInfo.visible_area;
        jsonObject["year"] = hardwareInfo.year;
        jsonObject["week"] = hardwareInfo.week;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<KeyboardInfo> keyboardInfolist) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: keyboardInfolist) {
        QJsonObject jsonObject;
        jsonObject["vendor"] = hardwareInfo.vendor;
        jsonObject["name"] = hardwareInfo.name;
        jsonObject["devtype"] = hardwareInfo.devtype;
        jsonObject["interface"] = hardwareInfo.interface;
        jsonObject["model"] = hardwareInfo.model;
        jsonObject["driver"] = hardwareInfo.driver;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<MouseInfo> mouseInfolist) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: mouseInfolist) {
        QJsonObject jsonObject;
        jsonObject["vendor"] = hardwareInfo.vendor;
        jsonObject["name"] = hardwareInfo.name;
        jsonObject["devtype"] = hardwareInfo.devtype;
        jsonObject["interface"] = hardwareInfo.interface;
        jsonObject["model"] = hardwareInfo.model;
        jsonObject["driver"] = hardwareInfo.driver;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<CameraInfo> cameraInfolist) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: cameraInfolist) {
        QJsonObject jsonObject;
        jsonObject["vendor"] = hardwareInfo.vendor;
        jsonObject["model"] = hardwareInfo.model;
        jsonObject["name"] = hardwareInfo.name;
        jsonObject["interface"] = hardwareInfo.interface;
        jsonObject["driver"] = hardwareInfo.driver;
        jsonObject["version"] = hardwareInfo.version;
        jsonObject["businfo"] = hardwareInfo.businfo;
        jsonObject["speed"] = hardwareInfo.speed;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<BatteryInfo> batteryInfolist) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: batteryInfolist) {
        QJsonObject jsonObject;
        jsonObject["vendor"] = hardwareInfo.vendor;
        jsonObject["model"] = hardwareInfo.model;
        jsonObject["serailnum"] = hardwareInfo.serailnum;
        jsonObject["state"] = hardwareInfo.state;
        jsonObject["energy"] = hardwareInfo.energy;
        jsonObject["energy_full"] = hardwareInfo.energy_full;
        jsonObject["time_to_empty"] = hardwareInfo.time_to_empty;
        jsonObject["percentage"] = hardwareInfo.percentage;
        jsonObject["used_times"] = hardwareInfo.used_times;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<CDInfo> cdInfolist) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: cdInfolist) {
        QJsonObject jsonObject;
        jsonObject["vendor"] = hardwareInfo.vendor;
        jsonObject["model"] = hardwareInfo.model;
        jsonObject["businfo"] = hardwareInfo.businfo;
        jsonObject["name"] = hardwareInfo.name;
        jsonObject["version"] = hardwareInfo.version;
        jsonObject["driver"] = hardwareInfo.driver;
        jsonObject["devicenum"] = hardwareInfo.devicenum;
        jsonObject["speed"] = hardwareInfo.speed;
        jsonObject["serialnum"] = hardwareInfo.serialnum;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

QString getDriver::convertClassToString(QList<FanInfo> fanInfolist) {
    QJsonArray jsonArray;

    for (const auto &hardwareInfo: fanInfolist) {
        QJsonObject jsonObject;
        jsonObject["name"] = hardwareInfo.name;
        jsonObject["speed"] = hardwareInfo.speed;

        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}

void getDriver::addHWinfo(bool flag, HardWareInfo hwInfo) {
    QList<HardWareInfo> tempHwInfo;
    switch (hwInfo.type) {
    case DeviceType::SCANNER:
        if (!flag){
            m_scannerHardwares.append(hwInfo);
            tempHwInfo.append(hwInfo);
        }else{
            for (int i = 0; i < m_scannerHardwares.length(); ++i) {
                if (m_scannerHardwares[i].PID == hwInfo.PID && m_scannerHardwares[i].VID == hwInfo.VID){
                    m_scannerHardwares[i].isOnline = true;
                    tempHwInfo.append(m_scannerHardwares[i]);
                }
            }
        }
        emit refreshInfo(hwInfo.type, convertClassToString(m_scannerHardwares));
        break;
    case DeviceType::PRINTER:
        if (!flag) {
            m_printerHardwares.append(hwInfo);
            tempHwInfo.append(hwInfo);
        }else{
            for (int i = 0; i < m_printerHardwares.length(); ++i) {
                if (m_printerHardwares[i].PID == hwInfo.PID && m_printerHardwares[i].VID == hwInfo.VID){
                    m_printerHardwares[i].isOnline = true;
                    tempHwInfo.append(m_printerHardwares[i]);
                }
            }
        }
        emit refreshInfo(hwInfo.type, convertClassToString(m_printerHardwares));
        break;
    default:
        break;
    }

    emit addDevInfo(convertClassToString(tempHwInfo));

}

void getDriver::removeHWinfo(HardWareInfo hwInfo) {
    QList<HardWareInfo> tempHwInfo;

    for (int i = 0; i < m_scannerHardwares.length(); ++i) {
        if(m_scannerHardwares.at(i).PID == hwInfo.PID && m_scannerHardwares.at(i).VID == hwInfo.VID){
            m_scannerHardwares[i].isOnline = false;
            tempHwInfo.append(m_scannerHardwares[i]);
            emit refreshInfo(m_scannerHardwares[i].type, convertClassToString(m_scannerHardwares));
        }
    }

    for (int i = 0; i < m_printerHardwares.length(); ++i) {
        if(m_printerHardwares.at(i).PID == hwInfo.PID && m_printerHardwares.at(i).VID == hwInfo.VID){
            m_printerHardwares[i].isOnline = false;
            tempHwInfo.append(m_printerHardwares[i]);
            emit refreshInfo(m_printerHardwares[i].type, convertClassToString(m_printerHardwares));
        }
    }
    emit removeDevInfo(convertClassToString(tempHwInfo));

}

QList<HardWareInfo> getDriver::getPrinterScanenrHw() {
    QList<HardWareInfo> hwinfo;
    hwinfo.append(m_scannerHardwares);
    hwinfo.append(m_printerHardwares);
    return hwinfo;
}

getDriver *getDriver::getInstance() {
    return instance;
}

static QString getPackageVersion(const QString &packageName)
{
    QString res{QString()};
    if (packageName.isEmpty()) {
        return res;
    }
    QString cmd = QString("/usr/bin/dpkg-query -W -f='${Version}\\n' %1").arg(packageName);
    res = getRetFromCommand(cmd.split(" "));
    return  res;
}

static QString getRetFromCommand(const QStringList &command)
{
    QProcess proc;
    QStringList options;
    options << "-c"<< command.join(" ");
    proc.closeWriteChannel();
    proc.start("bash", options);
    if (!proc.waitForFinished())
        return "";
    QString res = QString(proc.readAll());
    proc.close();
    if(res.right(1) == "\n")
        res.chop(1);
    return res;
}

static bool checkInstallable(const QString name, const QString version){
    QStringList command = QStringList{"apt-cache", "show", name};
    QString res = getRetFromCommand(command);
    if (res.isEmpty()){
        return false;
    }
    QStringList installabelVersionList;
    QStringList resList = res.split("\n");

    for(int i = 0; i < resList.size(); i++){
        if (resList[i].trimmed().startsWith("Version:")){
            installabelVersionList.append(resList[i].split(":")[1].trimmed());
        }
    }
    if (installabelVersionList.contains(version)){
        return true;
    }
    return false;
}
