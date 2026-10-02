#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "basedevice.h"

class UsbDevice : public BaseDevice
{
    Q_OBJECT
public:
    UsbDevice(DeviceParams params);

    virtual ~UsbDevice();
};

#endif // USBDEVICE_H
