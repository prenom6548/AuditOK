#include "drivermanagerservicedbus.h"
#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileInfo>
#include "../dbus_args.h"
#include "include/globalsignal.h"
// #include <libkydevicesec_netcard.h>

DriverManagerServiceTool::DriverManagerServiceTool(QObject *parent) : QObject(parent) {
    // 启动USB监听线程
    m_usbListenerThread = new USBListenerThread();
    m_usbListenerThread->start();
    connect(m_usbListenerThread, &USBListenerThread::deviceChange, this, &DriverManagerServiceTool::checkRedetectType);

    m_usbHotplugServer = new UsbHotplugServer();
    m_hotplugDriverFinder = new HotplugDevDriverFinder();

    getdriver = DRIVER_GETTER;

    getPcInfo();
    getCpuInfo();
    getMemoryInfo();
    getBaseboardInfo();
    getDiskInfo();
    getMonitorInfo();
    getKeyboardInfo();
    getMouseInfo();
    getCameraInfo();
    getBatteryInfo();
    getCDInfo();
    getFanInfo();
    getGraphicCardDriver();
    getScannerDriver();
    getPrinterDriver();
    getBluetoothInfo();
    getSoundcardInfo();
    getNetWorkcardInfo();
    getInputDeviceDriver();
    getVideoDeviceDriver();
    getOtherDeviceDriver();
    m_driverClient = new DrivermanagerClient();
    initConnect();
    initFinish = true;
    qDebug() << "init finished";
}

void DriverManagerServiceTool::initConnect() {
    connect(getdriver, &getDriver::refreshInfo, this, &DriverManagerServiceTool::updateHWinfo);
    connect(getdriver, &getDriver::addDevInfo, [=](const QString devString) {
        m_usbHotplugServer->emitSingal(true, devString);
    });
    connect(getdriver, &getDriver::removeDevInfo, [=](const QString devString) {
        m_usbHotplugServer->emitSingal(false, devString);
    });
    connect(g_signal, &GlobalSignal::installSuccessSignal, this, &DriverManagerServiceTool::updateInfo);
}

void DriverManagerServiceTool::updateHWinfo(DeviceType type, QString hwInfo)
{
    switch (type) {
    case DeviceType::SCANNER:
        scannerDriverInfo = hwInfo;
        break;
    case DeviceType::PRINTER:
        printerDriverInfo = hwInfo;
        break;
    case DeviceType::VideoOrAudio:
        videoDevInfo = hwInfo;
        break;
    case DeviceType::InputDev:
        inputDevInfo = hwInfo;
        break;
    case DeviceType::Other:
        otherDevInfo = hwInfo;
        break;
    default:
        break;
    }
}

QString DriverManagerServiceTool::getGraphicCardDriver()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    if (graphicCardInfo == "") {
        QString result = getdriver->getGraphicCardDriver();
        graphicCardInfo = result;
        return result;
    } else {
        return graphicCardInfo;
    }
}

QString DriverManagerServiceTool::getScannerDriver()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getScannerDriver();
    scannerDriverInfo = result;
    return result;
}

QString DriverManagerServiceTool::getPrinterDriver()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getPrinterDriver();
    printerDriverInfo = result;
    return result;

}

QString DriverManagerServiceTool::getBluetoothInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getBluetoothInfo();
    bluetoothInfo = result;
    return result;
}

QString DriverManagerServiceTool::getSoundcardInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    if (soundCardInfo == "") {
        QString result = getdriver->getSoundcardInfo();
        soundCardInfo = result;
        return result;
    } else {
        return soundCardInfo;
    }
}

QString DriverManagerServiceTool::getNetWorkcardInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getNetWorkcardInfo();
    return result;
}

QString DriverManagerServiceTool::getInputDeviceDriver()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    if (inputDevInfo == "") {
        QString result = getdriver->getInputDeviceDriver();
        inputDevInfo = result;
        return result;
    } else {
        return inputDevInfo;
    }
}

QString DriverManagerServiceTool::getVideoDeviceDriver()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    if (videoDevInfo == "") {
        QString result = getdriver->getVideoDeviceDriver();
        videoDevInfo = result;
        return result;
    } else {
        return videoDevInfo;
    }
}

QString DriverManagerServiceTool::getOtherDeviceDriver()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    if (otherDevInfo == "") {
        QString result = getdriver->getOtherDeviceDriver();
        otherDevInfo = result;
        return result;
    } else {
        return otherDevInfo;
    }
}

QString DriverManagerServiceTool::getPcInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getPcInfo();
    pcInfo = result;
    return result;
}

QString DriverManagerServiceTool::getCpuInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    if (cpuInfo == "") {
        QString result = getdriver->getCpuInfo();
        cpuInfo = result;
        return result;
    } else {
        return cpuInfo;
    }
}

QString DriverManagerServiceTool::getMemoryInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    if (memoryInfo == "") {
        QString result = getdriver->getMemoryInfo();
        memoryInfo = result;
        return result;
    } else {
        return memoryInfo;
    }
}

QString DriverManagerServiceTool::getBaseboardInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    if (baseboardInfo == "") {
        QString result = getdriver->getBaseboardInfo();
        baseboardInfo = result;
        return result;
    } else {
        return baseboardInfo;
    }
}

QString DriverManagerServiceTool::getDiskInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getDiskInfo();
    return result;
}

QString DriverManagerServiceTool::getMonitorInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getMonitorInfo();
    return result;
}

QString DriverManagerServiceTool::getKeyboardInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getKeyboardInfo();
    return result;
}

QString DriverManagerServiceTool::getMouseInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getMouseInfo();
    return result;
}

QString DriverManagerServiceTool::getCameraInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getCameraInfo();
    return result;
}

QString DriverManagerServiceTool::getBatteryInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getBatteryInfo();
    return result;
}

QString DriverManagerServiceTool::getFanInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getFanInfo();
    return result;

}


QString DriverManagerServiceTool::getCDInfo()
{
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    QString result = getdriver->getCDInfo();
    return result;
}

void
DriverManagerServiceTool::checkRedetectType(QString action, QString vid, QString pid, QString vendor, QString model, QString devtype)
{
    HardWareInfo hwInfo;
    hwInfo.vendor = vendor;
    hwInfo.model = model;
    hwInfo.PID = pid;
    hwInfo.VID = vid;

    // 判断是不是第一次接入，如果第一次接入则需请求服务器，否则仅需更改参数即可
    QList<HardWareInfo> hwList = DRIVER_GETTER->getPrinterScanenrHw();   // 所有的打印、扫描设备
    bool flag = false; // 用于标识新插入的设备是否是已经请求过服务器的设备
    for (int i = 0; i < hwList.length(); ++i) {
        if (hwList.at(i).PID == pid && hwList.at(i).VID == vid) {
            hwInfo = hwList[i];
//            flag = true;
            break;
        }
    }
    if (!flag && action == "add"){
        m_hotplugDriverFinder->dowork(hwInfo, vid, pid, vendor, model, devtype);
    }

    if (!hwInfo.PID.isEmpty() && !hwInfo.VID.isEmpty()) {
        if (action == "add") {
            getdriver->addHWinfo(flag, hwInfo);
        }
        if (action == "remove") {
            getdriver->removeHWinfo(hwInfo);
        }
    }
}

void DriverManagerServiceTool::redetectGraphicCard()
{
    if (initFinish && !isAllowedCaller()) {
        return;
    }
    graphicCardInfo = "";
    getGraphicCardDriver();
}

void DriverManagerServiceTool::redetectScanner()
{
    if (initFinish && !isAllowedCaller()) {
        return;
    }
    scannerDriverInfo = "";
    getScannerDriver();
}

void DriverManagerServiceTool::redetectPrinter()
{
    if (initFinish && !isAllowedCaller()) {
        return;
    }
    printerDriverInfo = "";
    getPrinterDriver();
}

void DriverManagerServiceTool::redetectInputDev()
{
    if (initFinish && !isAllowedCaller()) {
        return;
    }
    inputDevInfo = "";
    getInputDeviceDriver();
}

void DriverManagerServiceTool::redetectVideoDev()
{
    if (initFinish && !isAllowedCaller()) {
        return;
    }
    videoDevInfo = "";
    getVideoDeviceDriver();
}

void DriverManagerServiceTool::redetectOtherDev()
{
    if (initFinish && !isAllowedCaller()) {
        return;
    }
    otherDevInfo = "";
    getOtherDeviceDriver();
}

void DriverManagerServiceTool::redetectBluetoothDev() {
    if (initFinish && !isAllowedCaller()) {
        return;
    }
    bluetoothInfo = "";
}

bool DriverManagerServiceTool::isAllowedCaller()
{
    return true;
    QString clientService = message().service();
    QDBusConnectionInterface *intf = connection().interface();
    unsigned int pid = intf->servicePid(clientService).value();
    /*check caller environ*/
    if (!kom::KomService::checkEnviron(pid)) {
        sendErrorReply(QDBusError::ErrorType::Failed, QString("[%2] dbus method control,env forbidden").arg((pid)));
        return false;
    }
    /*check caller white list*/
    QFileInfo file(QString("/proc/%1/exe").arg(pid));
    QString execPath = "";
    if (file.exists()) {
        execPath = file.canonicalFilePath();
        foreach (const QString &whitePath, kom::KomService::programWhiteList) {
            if (execPath == whitePath) {
                return true;
            }
        }
    }
    sendErrorReply(QDBusError::ErrorType::Failed,
                   QString("[%2] dbus method control,operation not permitted").arg((execPath)));
    return false;
}

void DriverManagerServiceTool::updateInfo() {
    scannerDriverInfo = DRIVER_GETTER->convertClassToString(DRIVER_GETTER->m_scannerHardwares);
    printerDriverInfo = DRIVER_GETTER->convertClassToString(DRIVER_GETTER->m_printerHardwares);
}

void DriverManagerServiceTool::disableWirelessNetworkCard() {
    if (initFinish && !isAllowedCaller()) {
        return;
    }
    //int res = kdk_device_set_netcard_status(KDK_NET_WIRELESS, KDK_NET_DISABLE);
    //qDebug() << "disableWirelessNetworkCard Result:" << res;
}

void DriverManagerServiceTool::enableWirelessNetworkCard() {
    if (initFinish && !isAllowedCaller()) {
        return;
    }
    //int res = kdk_device_set_netcard_status(KDK_NET_WIRELESS, KDK_NET_ENABLE);
    //qDebug() << "enableWirelessNetworkCard Result:" << res;
}

void DriverManagerServiceTool::enableWiredNetworkCard() {
    if (initFinish && !isAllowedCaller()) {
        return;
    }
    //int res = kdk_device_set_netcard_status(KDK_NET_ETHERNET, KDK_NET_ENABLE);
    //qDebug() << "enableWiredNetworkCard Result:" << res;
}

void DriverManagerServiceTool::disableWiredNetworkCard() {
    if (initFinish && !isAllowedCaller()) {
        return;
    }
    //int res = kdk_device_set_netcard_status(KDK_NET_ETHERNET, KDK_NET_DISABLE);
    //qDebug() << "disableWiredNetworkCard Result:" << res;
}

QString DriverManagerServiceTool::getWirelessNetworkCardStatus() {
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    //int status = kdk_device_get_netcard_status(KDK_NET_WIRELESS);
    //if(status == 0){
    //    return "disabled";
    //} else if (status == 1) {
    //    return "enabled";
    //}
    return  "";
}

QString DriverManagerServiceTool::getWiredNetworkCardStatus() {
    if (initFinish && !isAllowedCaller()) {
        return "";
    }
    //int status = kdk_device_get_netcard_status(KDK_NET_ETHERNET);
    //if(status == 0){
    //    return "disabled";
    //} else if (status == 1) {
    //    return "enabled";
    //}
    return  "";
}
