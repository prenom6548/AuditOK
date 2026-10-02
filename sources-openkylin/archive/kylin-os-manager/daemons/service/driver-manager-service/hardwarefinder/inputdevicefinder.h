#ifndef INPUTDEVICEFINDER_H
#define INPUTDEVICEFINDER_H

#include "devicefinder.h"

class inputDeviceFinder: public deviceFinder
{
    Q_OBJECT
public:
    inputDeviceFinder();
    ~inputDeviceFinder();

    QList<HardWareInfo> getUsbDeviceList();
    void dowork();

private:
    QList<HardWareInfo> m_usbInfoList;
    void findUsbDev();
};

#endif // INPUTDEVICEFINDER_H
