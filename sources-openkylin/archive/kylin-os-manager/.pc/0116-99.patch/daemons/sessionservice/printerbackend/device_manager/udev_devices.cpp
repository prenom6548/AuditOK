#include <cups/cups.h>
#include <cups/http.h>
#include <errno.h>
#include <fcntl.h>
#include <libudev.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <syslog.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <glib.h>
#include <dirent.h>
#include <libudev.h>

#include <QString>
#include <QStringList>
#include <QDebug>

#include "udev_devices.h"
#include "device_information.h"
#include "common.h"
#include "lpinfo_browser.h"
#include "ukui_printer.h"


// usb-001-067
static char *
devpath_from_usb_devaddr (struct udev *udev, const char *devaddr)
{
    char *devname_ending = g_strdup (devaddr);
    char *devname;
    const char *devpath;
    struct udev_enumerate *udev_enum;
    struct udev_list_entry *first = NULL;
    struct udev_device *device;

    g_strdelimit (devname_ending, "-", '/');
    devname = g_strdup_printf("/dev/bus/%s", devname_ending);
    g_free (devname_ending);

    udev_enum = udev_enumerate_new (udev);
    if (udev_enum == NULL) {
        qDebug() << "udev_enumerate_new failed";
        exit (1);
    }

    if (udev_enumerate_add_match_property (udev_enum, "DEVNAME", devname) < 0) {
        udev_enumerate_unref (udev_enum);
        qDebug() << "udev_enumerate_add_match_property failed";
        exit (1);
    }

    if (udev_enumerate_scan_devices (udev_enum) < 0) {
        udev_enumerate_unref (udev_enum);
        qDebug() << "udev_enumerate_scan_devices failed";
        exit (1);
    }

    first = udev_enumerate_get_list_entry (udev_enum);
    if (first == NULL) {
        udev_enumerate_unref (udev_enum);
        qDebug ("no device named %s found", devname);
        exit (1);
    }

    device = udev_device_new_from_syspath (udev,
                                           udev_list_entry_get_name (first));
    if (device == NULL) {
        udev_enumerate_unref (udev_enum);
        qDebug ("unable to examine device");
        exit (1);
    }

    devpath = udev_device_get_devpath (device);
    udev_enumerate_unref (udev_enum);
    if (!devpath) {
        qDebug ("no devpath for device");
        exit (1);
    }

    g_free (devname);
    return g_strdup (devpath);
}

// 将device_id中的信息转成id
void
parse_device_id (const char *device_id,
		         struct device_id_s *id)
{
    QString s = QString(device_id);

    // len = strlen (device_id);
    if (s.size() == 0)
        return;

    if (s[s.size() - 1] == '\n')
        s.chop(1);


    id->full_device_id = s;

    QStringList ls = s.split(";");

    for (auto ss : ls) {
        if (id->mfg.isEmpty()
         && (ss.indexOf("MANUFACTURER") == 0 || ss.indexOf("MFG") == 0)
         && (ss.contains(":"))) {

            id->mfg = ss.split(":").at(1).trimmed();
        }

        else if (id->mdl.isEmpty()
         && (ss.indexOf("MODEL") == 0 || ss.indexOf("MDL") == 0)
         && (ss.contains(":"))) {

            id->mdl = ss.split(":").at(1).trimmed();
        }

        else if (id->sern.isEmpty()
         && (ss.indexOf("SERIALNUMBER") == 0 || ss.indexOf("SERN") == 0 || ss.indexOf("SN") == 0)
         && (ss.contains(":"))) {

            id->sern = ss.split(":").at(1).trimmed();
        }
    }
    // 特殊打印机处理
    specialDeviceCheck(id->mfg, id->mdl);
    return ;
}


void deviceIdToDeviceInfo(DeviceInformation &info)
{
    if (info.ieee1284ID.isEmpty()) {
        return ;
    }
    device_id_s id;
    parse_device_id(QTC(info.ieee1284ID), &id);
    if (info.vendor.isEmpty()) {
        info.vendor = id.mfg;
    }
    if (info.model.isEmpty()) {
        info.model = id.mdl;
    }
    if (info.serial.isEmpty()) {
        info.serial = id.sern;
    }
}

QString get_ieee1284_id_from_child (struct udev *udev, struct udev_device *parent)
{
    struct udev_enumerate *udev_enum;
    udev_enum = udev_enumerate_new (udev);
    if (!udev_enum) {
        qDebug ("udev_enumerate_new failed");
        return QString();
    }

    if (udev_enumerate_add_match_parent (udev_enum, parent) < 0) {
        udev_enumerate_unref (udev_enum);
        qDebug ("uname to add parent match");
        return QString();
    }

    if (udev_enumerate_scan_devices (udev_enum) < 0) {
        udev_enumerate_unref (udev_enum);
        qDebug ("udev_enumerate_scan_devices failed");
        return QString();
    }

    QString ieee1284_id{QString()};
    struct udev_list_entry *item, *first = nullptr;
    first = udev_enumerate_get_list_entry (udev_enum);
    udev_list_entry_foreach (item, first) {
        struct udev_device *dev;
        dev = udev_device_new_from_syspath (udev, udev_list_entry_get_name (item));
        if (dev == nullptr)
            continue;

        ieee1284_id = udev_device_get_sysattr_value (dev, "ieee1284_id");

        udev_device_unref (dev);
        if (ieee1284_id.size()) {
            break;
        }
    }

    udev_enumerate_unref (udev_enum);
    return ieee1284_id;
}

QString get_ieee1284_id_using_libusb (struct udev_device *dev)
{
    QList<UkuiUsbPrinter> printers;
    // magic function
    UkuiPrinterManager::getInstance().enumPrinters(printers);
    
    QString res;
    QString udevbusnum(udev_device_get_sysattr_value(dev, "busnum"));
    QString udevdevnum(udev_device_get_sysattr_value(dev, "devnum"));

    int got = 0;

    libusb_init(nullptr);

    int numdevs = 0;
    libusb_device **list = nullptr;
    numdevs = libusb_get_device_list(nullptr, &list);
    if (numdevs <= 0) {
        libusb_free_device_list(list, 1);
        libusb_exit(nullptr);
        return QString();
    }

    for (int i = 0; i < numdevs; i++) {

        struct libusb_device *device = list[i];

        QString busNumber = QString("%1").arg(libusb_get_bus_number(device));
        QString deviceNumber = QString("%1").arg(libusb_get_device_address(device));
        if (udevbusnum != busNumber || udevdevnum != deviceNumber) {
            continue;
        } 

        struct libusb_device_descriptor devdesc;
        if (libusb_get_device_descriptor(device, &devdesc) < 0) {
            continue;
        }

        if (!devdesc.bNumConfigurations || !devdesc.idVendor || !devdesc.idProduct) {
            continue;
        }

        for (int conf = 0; conf < devdesc.bNumConfigurations; conf++) {
            struct libusb_config_descriptor *confptr = NULL;
            if (libusb_get_config_descriptor(device, conf, &confptr) < 0)
                continue;
            const libusb_interface *ifaceptr;
            int iface;
            for (iface = 0, ifaceptr = confptr->interface; iface < confptr->bNumInterfaces; iface++, ifaceptr++) {

                if (got == 1) {
                    continue;
                }
                int altset;
                const struct libusb_interface_descriptor *altptr;
                struct libusb_device_handle *handle = NULL;
                
                int protocol   = 0;
                for (altset = 0, altptr = ifaceptr->altsetting; altset < ifaceptr->num_altsetting; altset++, altptr++) {

                    if ((altptr->bInterfaceProtocol != 1 && altptr->bInterfaceProtocol != 2) ||
                        altptr->bInterfaceProtocol < protocol) {
                        continue;
                    }

                    uint8_t endp;
                    uint8_t read_endp  = 0xff;
	                uint8_t write_endp = 0xff;
                    const struct libusb_endpoint_descriptor *endpptr = NULL;

                    for (endp = 0, endpptr = altptr->endpoint; endp < altptr->bNumEndpoints; endp ++, endpptr ++) {
                        if ((endpptr->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK) {
                            if (endpptr->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) {
                                read_endp = endp;
                            } else {
                                write_endp = endp;
                            }
                        }
                    }

                    if (write_endp != 0xff) {
                        protocol = altptr->bInterfaceProtocol;
                    }
                }

                if (protocol <= 0) {
                    continue;
                }

                int r = libusb_open(device, &handle);
                if (r < 0) {
                    qDebug ("failed to open device %d", r);
                    continue;
                }

                if (libusb_kernel_driver_active(handle, iface) == 1) {
                    auto r = libusb_detach_kernel_driver(handle, iface);
                    if (r < 0) {
                        qDebug("DEBUG: Failed to detach \"usblp\" module from %04x:%04x\n",
	                        devdesc.idVendor, devdesc.idProduct);
                        libusb_close(handle);
                        continue;
                    }
                }
                char ieee1284_id[1024];
                memset (ieee1284_id, 0, sizeof (ieee1284_id));
                if (libusb_control_transfer(handle,
                                            LIBUSB_REQUEST_TYPE_CLASS |
                                            LIBUSB_ENDPOINT_IN |
                                            LIBUSB_RECIPIENT_INTERFACE,
                                            0, conf,
                                            (iface << 8) | altset,
                                            (unsigned char *)ieee1284_id,
                                            sizeof (ieee1284_id),
                                            5000) < 0) {
                    
                    qDebug ("Failed to fetch Device ID");
                } else {
                    got = 1;
                    qDebug("%s\n", ieee1284_id + 2);
                    res = QString(ieee1284_id + 2);
                }
                libusb_close (handle);
            }
        }
    }

    if (numdevs >= 0) {
        libusb_free_device_list(list, 1);
    }
    libusb_exit(nullptr);
    if (got) {
        return res;
    }
    return QString();
}

QStringList find_matching_device_uris (const QString &deviceId)
{
    QStringList res{QStringList()};
    auto mp = LpinfoBrowser::findDirectPrinter();
    for (auto dev : mp) {
        if (dev.ieee1284ID == deviceId) {
            res.append(dev.uri);
        }
    }
    return res;
}

// devpath:/devices/pci0000:00/0000:00:14.0/usb1/1-2
QString device_id_from_devpath (struct udev *udev, const QString &devpath)
{
    QString device_id{QString()};
    QString syspath = "/sys" + devpath;

    struct udev_device *dev = udev_device_new_from_syspath (udev, QTC(syspath));
    if (dev == NULL) {
        udev_device_unref (dev);
        qDebug ("unable to access %s", syspath);
        return NULL;
    }

    qDebug ("device devpath is %s", udev_device_get_devpath(dev));
    qDebug ("device serial is %s", udev_device_get_sysattr_value (dev, "serial"));

    device_id = get_ieee1284_id_from_child (udev, dev);
    if (device_id.isEmpty()) {
        /* Use libusb to fetch the Device ID. */
        device_id = get_ieee1284_id_using_libusb (dev);
        qDebug() << device_id;
    }

    udev_device_unref (dev);
    return device_id;
}


QStringList list_printers_dev_path()
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    QStringList res;

    udev = udev_new();
    if (!udev) {
        qDebug("Can't create udev\n");
        return res;
    }

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "usb");
    // udev_enumerate_add_match_sysattr(enumerate, "bInterfaceClass", "07");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "usb_device");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        // dev = udev_device_get_parent_with_subsystem_devtype(
        //     dev,
        //     "usb",
        //     "usb_device");
        if (!dev)
        {
            // printf("Unable to find parent usb device.");
            continue;
        }
        // printf(" VID/PID: %s %s\n",
        //        udev_device_get_sysattr_value(dev, "idVendor"),
        //        udev_device_get_sysattr_value(dev, "idProduct"));
        // printf(" %s\n %s\n",
        //        udev_device_get_sysattr_value(dev, "manufacturer"),
        //        udev_device_get_sysattr_value(dev, "product"));
        // printf(" serial: %s\n",
        //        udev_device_get_sysattr_value(dev, "serial"));
        // printf("%s\n", udev_device_get_devpath(dev));
        // printf(" serial: %s\n",
        //        udev_device_get_sysattr_value(dev, "interface"));
        res.append(udev_device_get_devpath(dev));
        udev_device_unref(dev);
    }
    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);

    udev_unref(udev);
    res = res.toSet().toList();
    return res;
}

QString get_device_id_from_path(const QString &path)
{
    struct udev *udev;
    QString res = QString();

    if (path.isEmpty()) {
        return res;
    }

    udev = udev_new ();

    res = device_id_from_devpath (udev, path);
    udev_unref(udev);
    return res;
}
