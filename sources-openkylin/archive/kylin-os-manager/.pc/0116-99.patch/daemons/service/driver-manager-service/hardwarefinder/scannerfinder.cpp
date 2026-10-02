#include "scannerfinder.h"
#include <libusb-1.0/libusb.h>
#include <cstring>
#include <QDebug>
#include <libudev.h>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QSysInfo>
#include <QDataStream>
#include <QProcess>

void devpath_from_usb_devaddr_scanner(const QString &devaddr, HardWareInfo &info);

static bool usbClassCheck(int bInterfaceClass, int bInterfaceSubClass);

scannerFinder::scannerFinder() {
}

scannerFinder::~scannerFinder() {

}

QList<HardWareInfo> scannerFinder::getUsbDeviceList() {
    return m_usbInfoList;
}

void scannerFinder::dowork() {
    m_usbInfoList.clear();
    findUsbDev();
}

void scannerFinder::findUsbDev() {
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

    for (int i = 0; i < numdevs; i++) {
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
                for (altset = 0, altptr = ifaceptr->altsetting; altset < ifaceptr->num_altsetting; altset++, altptr++) {
                    //                    qDebug("%x %x %x %x\n", altptr->bInterfaceClass, altptr->bInterfaceSubClass, devdesc.idVendor, devdesc.idProduct);
                    if (!usbClassCheck(altptr->bInterfaceClass, altptr->bInterfaceSubClass)) {
                        continue;
                    }

                    info.VID = QString("%1").arg(devdesc.idVendor, 4, 16, QLatin1Char('0'));
                    info.PID = QString("%1").arg(devdesc.idProduct, 4, 16, QLatin1Char('0'));
                    info.busNumber = QString("%1").arg(libusb_get_bus_number(device), 3, 10, QLatin1Char('0'));
                    info.deviceNumber = QString("%1").arg(libusb_get_device_address(device), 3, 10, QLatin1Char('0'));
                    handle = NULL;
                    QString devaddr = QString("/dev/bus/usb/%1/%2").arg(info.busNumber).arg(info.deviceNumber);
                    devpath_from_usb_devaddr_scanner(qstringTochar(devaddr), info);
                }
            }
        }
        if (info.vendor.size() && info.model.size())
            m_usbInfoList.append(info);
    }
}

bool usbClassCheck(int bInterfaceClass, int bInterfaceSubClass) {
    switch (bInterfaceClass) {
        case LIBUSB_CLASS_IMAGE:
            return true;
        case LIBUSB_CLASS_VENDOR_SPEC:
            return true;
        default:
            return false;
    }
    return false;
}

void devpath_from_usb_devaddr_scanner(const QString &devaddr, HardWareInfo &info) {
    if (devaddr.isEmpty()) {
        return;
    }

    struct udev *udev = nullptr;
    udev = udev_new();

    struct udev_enumerate *udev_enum = nullptr;
    udev_enum = udev_enumerate_new(udev);
    if (udev_enum == nullptr) {
        qDebug() << "udev_enumerate_new failed";
        udev_unref(udev);
        return;
    }

    if (udev_enumerate_add_match_property(udev_enum, "DEVNAME", qstringTochar(devaddr)) < 0) {
        udev_unref(udev);
        udev_enumerate_unref(udev_enum);
        qDebug() << "udev_enumerate_add_match_property failed";
        return;
    }

    if (udev_enumerate_scan_devices(udev_enum) < 0) {
        udev_unref(udev);
        udev_enumerate_unref(udev_enum);
        qDebug() << "udev_enumerate_scan_devices failed";
        return;
    }

    struct udev_list_entry *first = nullptr;
    first = udev_enumerate_get_list_entry(udev_enum);
    if (first == nullptr) {
        udev_unref(udev);
        udev_enumerate_unref(udev_enum);
        qDebug("no device named %s found", qstringTochar(devaddr));
        return;
    }

    struct udev_device *device;
    device = udev_device_new_from_syspath(udev, udev_list_entry_get_name(first));
    if (device == nullptr) {
        udev_unref(udev);
        udev_enumerate_unref(udev_enum);
        qDebug("unable to examine device");
        return;
    }


    info.serial = QString(udev_device_get_sysattr_value(device, "serial"));
    info.vendor = QString(udev_device_get_sysattr_value(device, "manufacturer"));
    info.model = QString(udev_device_get_sysattr_value(device, "product"));

    device_id_s id;
    parse_device_id(get_ieee1284_id_from_child(udev, device), &id);
    info.deviceId = id.full_device_id;
    if (info.vendor.isEmpty()) {
        info.vendor = id.mfg;
    }
    if (info.model.isEmpty()) {
        info.model = id.mdl;
    }
    if (info.serial.isEmpty()) {
        info.serial = id.sern;
    }

    if (id.cls == "SCANNER") {
        info.type = DeviceType::SCANNER;
    }

    specialDeviceCheck(info.vendor, info.model);
}