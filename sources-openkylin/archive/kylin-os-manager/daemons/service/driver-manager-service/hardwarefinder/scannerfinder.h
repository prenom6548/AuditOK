#ifndef SCANNERFINDER_H
#define SCANNERFINDER_H

#include "devicefinder.h"

class scannerFinder: public deviceFinder
{
    Q_OBJECT
public:
    scannerFinder();
    ~scannerFinder();

    QList<HardWareInfo> getUsbDeviceList();
    void dowork();

private:
    QList<HardWareInfo> m_usbInfoList;
    void findUsbDev();
};

#endif // SCANNERFINDER_H
