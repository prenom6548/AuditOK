/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "usbFinder.h"
#include <libusb-1.0/libusb.h>
#include <cstring>
#include <cstdio>
#include <QDebug>
#include "common.h"
#include <libudev.h>
#include <glib.h>


void
parse_device_id (const char *device_id,
                 struct device_id_s *id)
{
    QString s = QString(device_id);

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

            id->mfg = ss.split(":").at(1);
        }

        else if (id->mdl.isEmpty()
                 && (ss.indexOf("MODEL") == 0 || ss.indexOf("MDL") == 0)
                 && (ss.contains(":"))) {

            id->mdl = ss.split(":").at(1);
        }

        else if (id->sern.isEmpty()
                 && (ss.indexOf("SERIALNUMBER") == 0 || ss.indexOf("SERN") == 0 || ss.indexOf("SN") == 0)
                 && (ss.contains(":"))) {

            id->sern = ss.split(":").at(1);
        }

        else if (id->cls.isEmpty()
                 && (ss.indexOf("CLS") == 0 || ss.indexOf("CLASS") == 0)
                 && (ss.contains(":"))) {

            id->cls = ss.split(":").at(1);
        }
    }
    // 特殊打印机处理
    specialDeviceCheck(id->mfg, id->mdl);
    return ;
}

static char *
get_ieee1284_id_from_child (struct udev *udev, struct udev_device *parent)
{
    struct udev_enumerate *udev_enum;
    struct udev_list_entry *item, *first = NULL;
    char *device_id = NULL;

    udev_enum = udev_enumerate_new (udev);
    if (!udev_enum) {
        qDebug ("udev_enumerate_new failed");
        exit (1);
    }

    if (udev_enumerate_add_match_parent (udev_enum, parent) < 0) {
        udev_enumerate_unref (udev_enum);
        qDebug ("uname to add parent match");
        exit (1);
    }

    if (udev_enumerate_scan_devices (udev_enum) < 0) {
        udev_enumerate_unref (udev_enum);
        qDebug ("udev_enumerate_scan_devices failed");
        exit (1);
    }

    first = udev_enumerate_get_list_entry (udev_enum);
    udev_list_entry_foreach (item, first) {
        const char *ieee1284_id = NULL;
        struct udev_device *dev;
        //        qDebug() << udev_list_entry_get_name (item);
        dev = udev_device_new_from_syspath (udev,
                                            udev_list_entry_get_name (item));
        if (dev == NULL)
            continue;

        ieee1284_id = udev_device_get_sysattr_value (dev, "ieee1284_id");
        if (ieee1284_id)
            device_id = g_strdup (ieee1284_id);

        udev_device_unref (dev);
        if (device_id)
            break;
    }

    udev_enumerate_unref (udev_enum);
    return device_id;
}


// /dev/bus/usb/001/067
void
devpath_from_usb_devaddr (const QString &devaddr, DeviceInformation &info)
{
    if (devaddr.isEmpty()) {
        return ;
    }

    struct udev *udev = nullptr;
    udev = udev_new ();

    struct udev_enumerate *udev_enum = nullptr;
    udev_enum = udev_enumerate_new (udev);
    if (udev_enum == nullptr) {
        qDebug() << "udev_enumerate_new failed";
        udev_unref (udev);
        return ;
    }

    if (udev_enumerate_add_match_property (udev_enum, "DEVNAME", QTC(devaddr)) < 0) {
        udev_unref (udev);
        udev_enumerate_unref (udev_enum);
        qDebug() << "udev_enumerate_add_match_property failed";
        return ;
    }

    if (udev_enumerate_scan_devices (udev_enum) < 0) {
        udev_unref (udev);
        udev_enumerate_unref (udev_enum);
        qDebug() << "udev_enumerate_scan_devices failed";
        return ;
    }

    struct udev_list_entry *first = nullptr;
    first = udev_enumerate_get_list_entry (udev_enum);
    if (first == nullptr) {
        udev_unref (udev);
        udev_enumerate_unref (udev_enum);
        qDebug ("no device named %s found", QTC(devaddr));
        return ;
    }

    struct udev_device *device;
    device = udev_device_new_from_syspath (udev,
                                           udev_list_entry_get_name (first));
    if (device == nullptr) {
        udev_unref (udev);
        udev_enumerate_unref (udev_enum);
        qDebug ("unable to examine device");
        return ;
    }


    info.serial = QString(udev_device_get_sysattr_value (device, "serial"));
    info.vendor = QString(udev_device_get_sysattr_value (device, "manufacturer")).trimmed();
    info.model  = QString(udev_device_get_sysattr_value (device, "product")).trimmed();

    device_id_s id;
    parse_device_id(get_ieee1284_id_from_child (udev, device), &id);
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



    //    qDebug() << get_ieee1284_id_from_child (udev, device);
    //    qDebug() << udev_device_get_devpath (device);
    //    qDebug() << udev_device_get_property_value(device, "SUBSYSTEM");
    //    qDebug() << udev_device_get_property_value(device, "DEVNAME");
    //    qDebug() << udev_device_get_property_value(device, "ID_VENDOR");
    //    qDebug() << udev_device_get_property_value(device, "ID_MODEL");
    //    qDebug() << udev_device_get_property_value(device, "ID_SERIAL");
    //    qDebug() << udev_device_get_property_value(device, "ID_BUS");
    //    qDebug() << udev_device_get_property_value(device, "ID_PATH");
    //    qDebug() << udev_device_get_property_value(device, "ID_USB_DRIVER");
    //    qDebug() << udev_device_get_sysattr_value (device, "serial");
    //    qDebug() << udev_device_get_sysattr_value (device, "ieee1284_id");
    //    qDebug() << udev_device_get_sysattr_value (device, "idVendor");
    //    qDebug() << udev_device_get_sysattr_value (device, "idProduct");
    //    qDebug() << udev_device_get_sysattr_value (device, "manufacturer");
    //    qDebug() << udev_device_get_sysattr_value (device, "product");
}



bool usbClassCheck(int bInterfaceClass, int bInterfaceSubClass)
{
    switch (bInterfaceClass) {
    case LIBUSB_CLASS_PER_INTERFACE:
        return true;
    case LIBUSB_CLASS_IMAGE:
        return true;
    case LIBUSB_CLASS_VENDOR_SPEC:
        return true;
    default:
        return false;
    }
    return false;
}

usbFinder::usbFinder()
{

}

usbFinder::~usbFinder()
{

}

void usbFinder::dowork()
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
        return ;
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

        DeviceInformation info;
        info.connectType = ConnectType::InfoFrom_Invalid;

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
                    qDebug("%x %x %x %x\n", altptr->bInterfaceClass, altptr->bInterfaceSubClass, devdesc.idVendor, devdesc.idProduct);
                    if (!usbClassCheck(altptr->bInterfaceClass, altptr->bInterfaceSubClass)) {
                        continue ;
                    }

                    info.VID = QString("%1").arg(devdesc.idVendor, 4, 16, QLatin1Char('0'));
                    info.PID = QString("%1").arg(devdesc.idProduct, 4, 16, QLatin1Char('0'));
                    info.busNumber = QString("%1").arg(libusb_get_bus_number(device), 3, 10, QLatin1Char('0'));
                    info.deviceNumber = QString("%1").arg(libusb_get_device_address(device), 3, 10, QLatin1Char('0'));
                    handle = NULL;
                    qDebug() << info;
                    QString devaddr = QString("/dev/bus/usb/%1/%2").arg(info.busNumber).arg(info.deviceNumber);
                    devpath_from_usb_devaddr(QTC(devaddr), info);
                    qDebug() << info;
                    if (info.vendor.size() && info.model.size())
                        info.connectType = ConnectType::InfoFrom_USB;

                }
            }
        }
        if (info.connectType == ConnectType::InfoFrom_USB)
            m_usbInfoList.append(info);
    }

}


QList<DeviceInformation> usbFinder::getList()
{
    return  m_usbInfoList;
}
