#ifndef PRINTERFINDER_H
#define PRINTERFINDER_H

#include "devicefinder.h"

class printerFinder: public deviceFinder
{
public:
    printerFinder();
    ~printerFinder();

    QList<HardWareInfo> getUsbDeviceList();
    void dowork();

private:
    QList<HardWareInfo> m_usbInfoList;
    void findUsbDev();
};

#endif // PRINTERFINDER_H
