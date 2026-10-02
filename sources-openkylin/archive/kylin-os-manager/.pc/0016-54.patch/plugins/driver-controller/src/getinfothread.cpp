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
