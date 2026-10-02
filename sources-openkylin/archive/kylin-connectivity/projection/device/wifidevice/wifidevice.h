#ifndef WIFIDEVICE_H
#define WIFIDEVICE_H

#include "basedevice.h"

class WifiDevice : public BaseDevice
{
    Q_OBJECT
public:
    WifiDevice(DeviceParams params);

    virtual ~WifiDevice();
};

#endif // WIFIDEVICE_H
