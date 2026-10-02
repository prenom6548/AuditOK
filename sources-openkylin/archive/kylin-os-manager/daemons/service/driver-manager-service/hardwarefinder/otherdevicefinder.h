#ifndef OTHERDEVICEFINDER_H
#define OTHERDEVICEFINDER_H

#include "devicefinder.h"

class OtherDeviceFinder: public deviceFinder
{
    Q_OBJECT
public:
    OtherDeviceFinder();
    ~OtherDeviceFinder();

    QList<HardWareInfo> getUsbDeviceList();
    void dowork();

private:
    QList<HardWareInfo> m_usbInfoList;
    void findUsbDev();
};

#endif // OTHERDEVICEFINDER_H
