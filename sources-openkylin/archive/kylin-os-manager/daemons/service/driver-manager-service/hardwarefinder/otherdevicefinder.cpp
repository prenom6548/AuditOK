#include "otherdevicefinder.h"
#include <libusb-1.0/libusb.h>
#include <libudev.h>
#include <QDebug>
#include <glib.h>

static bool usbClassCheck(int bInterfaceClass, int bInterfaceSubClass);

OtherDeviceFinder::OtherDeviceFinder()
{

}

OtherDeviceFinder::~OtherDeviceFinder()
{

}

QList<HardWareInfo> OtherDeviceFinder::getUsbDeviceList()
{
    return m_usbInfoList;

}

void OtherDeviceFinder::dowork()
{
    findUsbDev();
}

void OtherDeviceFinder::findUsbDev()
{
    int numdevs = 0;
    libusb_device **list = nullptr;
    struct libusb_device *device = nullptr;
    struct libusb_device_descriptor devdesc;
    libusb_init(nullptr);
    numdevs = libusb_get_device_list(nullptr, &list);
    if (numdevs <= 0) {
        libusb_free_device_list(list, 1);
        libusb_exit(nullptr);
        return;
    }
    qDebug("numdevs is %d\n", numdevs);

    for (int i = 0; i < numdevs; i++)
    {
        device = list[i];
        if (libusb_get_device_descriptor(device, &devdesc) < 0)
            continue;

        if (!devdesc.bNumConfigurations || !devdesc.idVendor || !devdesc.idProduct) {
            continue;
        }

        HardWareInfo info;
        for (int conf = 0; conf < devdesc.bNumConfigurations; conf++) {
            struct libusb_config_descriptor *confptr = NULL;
            if (libusb_get_config_descriptor(device, conf, &confptr) < 0)
                continue;
            const libusb_interface *ifaceptr;
            int iface;
            for (iface = 0, ifaceptr = confptr->interface;
                 iface < confptr->bNumInterfaces;
                 iface++, ifaceptr++) {
                int altset;
                const struct libusb_interface_descriptor *altptr;
                struct libusb_device_handle *handle = NULL;
                for (altset = 0, altptr = ifaceptr->altsetting; altset < ifaceptr->num_altsetting; altset ++, altptr ++) {
                    if (!usbClassCheck(altptr->bInterfaceClass, altptr->bInterfaceSubClass)) {
                        continue ;
                    }
                    info.VID = QString("%1").arg(devdesc.idVendor, 4, 16, QLatin1Char('0'));
                    info.PID = QString("%1").arg(devdesc.idProduct, 4, 16, QLatin1Char('0'));
                    info.busNumber = QString("%1").arg(libusb_get_bus_number(device), 3, 10, QLatin1Char('0'));
                    info.deviceNumber = QString("%1").arg(libusb_get_device_address(device), 3, 10, QLatin1Char('0'));
                    handle = NULL;
                    QString devaddr = QString("/dev/bus/usb/%1/%2").arg(info.busNumber).arg(info.deviceNumber);
                    devpath_from_usb_devaddr(qstringTochar(devaddr), info);
                }
            }
        }
        if (info.vendor.size() && info.model.size())
            info.type = DeviceType::Other;
            m_usbInfoList.append(info);
    }
}

bool usbClassCheck(int bInterfaceClass, int bInterfaceSubClass)
{
    switch (bInterfaceClass) {
    case LIBUSB_CLASS_PRINTER:
    case LIBUSB_CLASS_IMAGE:
    case LIBUSB_CLASS_HID:
    case LIBUSB_CLASS_AUDIO:
    case LIBUSB_CLASS_MASS_STORAGE:
    case LIBUSB_CLASS_HUB:
    case LIBUSB_CLASS_DATA:
        return false;
    default:
        return true;

    }
}
