#include "getinfothread.h"
#include <QDebug>
#include "hardwareinfogetter.h"

void GetInfoThread::run() {
    qDebug() << "get hardware info...";
    g_infogetter->getPcInfo();
    g_infogetter->getCpuInfo();
    g_infogetter->getMemoryInfo();
    g_infogetter->getBaseboardInfo();
    g_infogetter->getDiskInfo();
    g_infogetter->getMonitorInfo();
    g_infogetter->getKeyboardInfo();
    g_infogetter->getMouseInfo();
    g_infogetter->getCDInfo();
    g_infogetter->getCameraInfo();
    g_infogetter->getFanInfo();
    g_infogetter->getBatteryInfo();

    g_infogetter->getGraphicHardwares();
    g_infogetter->getScannerHardwares();
    g_infogetter->getPrinterHardwares();
    g_infogetter->getBluetoothHardwares();
    g_infogetter->getSoundcardHardwares();
    g_infogetter->getNetworkHardwares();
    g_infogetter->getInputHardwares();
    g_infogetter->getVideoHardwares();
    g_infogetter->getOtherHardwares();
}

void RefreshScannerInfo::run() {
    qDebug() << "refresh Scanner info...";
    g_infogetter->refreshServerInfo(DeviceType::SCANNER);
}

void RefreshGraphicCardInfo::run() {
    qDebug() << "refresh GraphicCard info...";
    g_infogetter->refreshServerInfo(DeviceType::GraphicCard);
}

void RefreshPrinterInfo::run() {
    qDebug() << "refresh Printer info...";
    g_infogetter->refreshServerInfo(DeviceType::PRINTER);
}

void RefreshInputDevInfo::run() {
    qDebug() << "refresh InputDev info...";
    g_infogetter->refreshServerInfo(DeviceType::InputDev);
}

void RefreshVideoDevInfo::run() {
    qDebug() << "refresh VideoDev info...";
    g_infogetter->refreshServerInfo(DeviceType::VideoOrAudio);
}

void RefreshOtherDevInfo::run() {
    qDebug() << "refresh OtherDev info...";
    g_infogetter->refreshServerInfo(DeviceType::Other);
}

void RefreshDiskvInfo::run() {
    qDebug() << "refresh Disk info...";
    g_infogetter->refreshDiskInfo();
}

void RefreshMonitorInfo::run() {
    qDebug() << "refresh Monitor info...";
    g_infogetter->refreshMonitorInfo();
}

void RefreshKeyboardInfo::run() {
    qDebug() << "refresh Keyboard info...";
    g_infogetter->refreshKeyboardInfo();
}

void RefreshMouseInfo::run() {
    qDebug() << "refresh Mouse info...";
    g_infogetter->refreshMouseInfo();
}

void RefreshBluetoothInfo::run() {
    qDebug() << "refresh Bluetooth info...";
    g_infogetter->refreshBluetoothInfo();
}

void RefreshNetworkInfo::run() {
    qDebug() << "refresh Network info...";
    g_infogetter->refreshNetworkInfo();
}

void RefreshCdInfo::run() {
    qDebug() << "refresh CD info...";
    g_infogetter->refreshCdInfo();
}
