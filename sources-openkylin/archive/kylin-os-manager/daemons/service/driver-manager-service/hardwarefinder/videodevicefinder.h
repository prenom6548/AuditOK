#ifndef VIDEODEVICEFINDER_H
#define VIDEODEVICEFINDER_H

#include "devicefinder.h"

class videoDeviceFinder: public deviceFinder
{
    Q_OBJECT
public:
    videoDeviceFinder();
    ~videoDeviceFinder();

    QList<HardWareInfo> getUsbDeviceList();
    void dowork();

private:
    QList<HardWareInfo> m_usbInfoList;
    void findUsbDev();

};

#endif // VIDEODEVICEFINDER_H
