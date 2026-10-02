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
#include "usbhotplugthread.h"
#include "usb.h"
#include <QDebug>
#include <libudev.h>
#include "globalsignal.h"
#include <QList>
#include <QProcess>
#include <QMessageBox>
#include "saneobject.h"
#include "mainwidget.cpp"

static SANE_Device *g_saneDeviceusb = nullptr;

void UsbHotplugThread::run()
{
    qDebug() << "read device list complete!";

    USBdeviceList = getConnectedScanner();
    USBdeviceList.append(getConnectedScanner2());
    connect(g_user_signal, &GlobalUserSignal::setExitFlagTrueSignal, this, &UsbHotplugThread::setExitFlagTrue);
    connect(g_user_signal, &GlobalUserSignal::setExitFlagFalseSignal, this, &UsbHotplugThread::setExitFlagFalse);
    do {
        /* Netlink message buffer */
        char buf[UEVENT_BUFFER_SIZE * 2] = {0};
        recv(g_sane_object->hotplug_sock, &buf, sizeof(buf), 0);

        QString recvData = QString(buf);
        if (recvData.contains("add@", Qt::CaseInsensitive)
                && recvData.contains("devices", Qt::CaseInsensitive)
                && recvData.contains("usb", Qt::CaseInsensitive)
                && recvData.endsWith(":1.0", Qt::CaseInsensitive)){
            // ":1.0" means each usb device first directory, which is must
            if(usbDeviceIdentify(recvData)){
                while(1){
                    if(!g_sane_object->usbRemoveSlotIsRunning){
                        usbAddSlot(recvData);
                        break;
                    }
                }
            }
        } else if (recvData.contains("remove@", Qt::CaseInsensitive)
                   && recvData.contains("devices", Qt::CaseInsensitive)
                   && recvData.contains("usb", Qt::CaseInsensitive)
                   && recvData.endsWith(":1.0", Qt::CaseInsensitive)){
            if(isRemoveScanner(recvData)){
                while(1){
                    if(!g_sane_object->usbAddSlotIsRunning){
                        removeToDeviceList(recvData);
                        usbRemoveSlot();
                        break;
                    }
                }
            }
        }
    } while (! exitWindowFlag);
    quit();
}

void UsbHotplugThread::setExitFlagTrue()
{
    exitWindowFlag = true;
}

void UsbHotplugThread::setExitFlagFalse()
{
    exitWindowFlag = false;
}

void UsbHotplugThread::usbAddSlot(QString qstr){
    g_sane_object->usbAddSlotIsRunning = true;
    qDebug() << "新的usb插入了";

    while(1){
        // 不在运行时再对设备列表进行刷新
        if(!g_sane_object->isOnScanning){
            g_sane_object->onDetection = true;
            // 正在查询扫描仪设备，请稍等...
            QString msg = tr("Querying scanner device. Please waitting...");
            g_user_signal->warnMsg(msg);
            g_user_signal->setScanIconDisableSignal();

            if(g_user_signal->getIsFailPage()){
                return;
            }

            initSane();

            QList<SANE_Device> current = GlobalUserSignal::getInstance()->getDeviceList();
            if(current.size() != 0){
                g_user_signal->updateConnectSuccessTextSignal(true);
            }else{
                emit g_user_signal->hotPlugScanCompleteSignal();
            }

            //todo:更新UI设备列表
            qDebug() << "a new scanner may joined!";

            g_sane_object->setSaneHaveHandle(false);
            g_user_signal->updateSetting();

            addToDeviceList(qstr);
            g_user_signal->warnMsg(tr("New Scanner has been Connected."));
            break;
        }
    }
}

void UsbHotplugThread::usbRemoveSlot(){
    g_sane_object->usbRemoveSlotIsRunning = true;
    qDebug() << "usb拔出了";
    // 检测到有扫描仪断开链接，如果您断开的是正在扫描的扫描仪，请点击取消扫描或者等待扫描仪报错信息。
    QString msg = tr("A scanner is disconnected. If you disconnect the scanner is on scanning, click Cancel Scan or wait for the scanner to report an error message.");
    g_user_signal->warnMsg(msg);
    while(1){
        if(!g_sane_object->isOnScanning){
            g_sane_object->onDetection = true;
            // 扫描仪断开链接，刷新扫描仪列表，请稍等...
            QString msg = tr("Scanner is disconnect，refreshing scanner list. Please waitting...");
            g_user_signal->warnMsg(msg);
            g_user_signal->setScanIconDisableSignal();

            initSane();
            QList<SANE_Device> current = GlobalUserSignal::getInstance()->getDeviceList();
            if(current.size() == 0){
                g_user_signal->updateConnectSuccessTextSignal(false);
                emit g_user_signal->hotPlugScanCompleteSignal();
            }
            //todo:禁止操作右边设置栏
            g_sane_object->setSaneStatus(false);
            g_sane_object->setSaneHaveHandle(false);

            g_user_signal->updateSetting();
            msg = tr("Scanner list refresh complete.");
            g_user_signal->warnMsg(msg);
            break;
        }
    }
}

bool UsbHotplugThread::usbDeviceIdentify(QString qstr)
{
    return usbDeviceAdd(qstr);
}

bool UsbHotplugThread::usbDeviceAdd(QString qstr)
{
    QString path = QString("/sys") + qstr.right(qstr.size() - 1 - qstr.indexOf('@'));
    QString deviceType = getDeviceTypeFromPath(path);

    if(deviceType == "ff" || deviceType == "06"){
        return  true;
    }else{
        return false;
    }
}

bool UsbHotplugThread::usbDeviceRemove(QString qstr){
    QString path = QString("/sys") + qstr.right(qstr.size() - 1 - qstr.indexOf('@'));

}

QStringList UsbHotplugThread::getConnectedScanner()
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
    udev_enumerate_add_match_sysattr(enumerate, "bInterfaceClass", "ff");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
        if (!dev)
        {
            // printf("Unable to find parent usb device.");
            continue;
        }
        res.append(udev_device_get_devpath(dev));
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);

    udev_unref(udev);
    res = res.toSet().toList();
    return res;
}

QStringList UsbHotplugThread::getConnectedScanner2()
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
    udev_enumerate_add_match_sysattr(enumerate, "bInterfaceClass", "06");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
        if (!dev)
        {
            // printf("Unable to find parent usb device.");
            continue;
        }
        res.append(udev_device_get_devpath(dev));
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);

    udev_unref(udev);
    res = res.toSet().toList();
    return res;
}

QString UsbHotplugThread::getDeviceTypeFromPath(QString path)
{
    QString res;
    QString bInterfaceClass;
    QStringList bInterfaceClassPathList = getRetFromCommand(QStringList{"find", path ,"-name", "bInterfaceClass"}).split("\n");
    for (int i = 0; i < bInterfaceClassPathList.size(); i++) {
        bInterfaceClass = getRetFromCommand(QStringList{"cat", bInterfaceClassPathList.at(i)});
        if (bInterfaceClass != "ff" && bInterfaceClass != "06"){
            continue;
        }
        res = bInterfaceClass;
    }
    return res;
}

QString UsbHotplugThread::getRetFromCommand(QStringList command)
{
    QProcess proc;
    QStringList options;
    options << "-c"<< command.join(" ");
    proc.closeWriteChannel();
    proc.start("bash", options);
    if (!proc.waitForFinished())
        return "";
    QString res = QString(proc.readAll());
    proc.close();
    if(res.right(1) == "\n")
        res.chop(1);
    return res;
}

void UsbHotplugThread::addToDeviceList(QString path)
{
    QString newPath = "";
    QStringList tempList = path.replace("add@", "").split("/");
    for(int i = 1; i < tempList.length() - 1; i++){
        newPath.append("/");
        newPath.append(tempList.at(i));
    }
    USBdeviceList.append(newPath);
}

void UsbHotplugThread::removeToDeviceList(QString path)
{
    QString newPath = "";
    QStringList tempList = path.replace("remove@", "").split("/");
    for(int i = 1; i < tempList.length() - 1; i++){
        newPath.append("/");
        newPath.append(tempList.at(i));
    }
    USBdeviceList.removeOne(newPath);
}

bool UsbHotplugThread::isRemoveScanner(QString path)
{
    QString newPath = "";
    QStringList tempList = path.replace("remove@", "").split("/");
    for(int i = 1; i < tempList.length() - 1; i++){
        newPath.append("/");
        newPath.append(tempList.at(i));
    }

    if(USBdeviceList.contains(newPath)){
        return true;
    }else{
        return false;
    }
}

static void authCallback (SANE_String_Const resource, SANE_Char *username, SANE_Char *password)
{
    //    qDebug() << "auth_callback" << resource << username << password;
}

static SANE_Status saneGetDevices(const SANE_Device ***device_list)
{
    SANE_Status status = SANE_STATUS_GOOD;

    g_sane_object->saneClose();
    g_sane_object->saneExit();

    SANE_Int version_code = 0;

    sane_init(&version_code, authCallback);

    qDebug() << "Get all scan devices, please waiting ...";

    memset(device_list, 0x00, sizeof(device_list));
    status = sane_get_devices(device_list, SANE_FALSE);

    if (status) {
        qDebug() << "status = " << sane_strstatus(status);
    }

    return status;
}

void UsbHotplugThread::initSane(){
    QStringList names;
    SANE_Status sane_status;
    QString name;
    //获取扫描设备列表

    sane_status = saneGetDevices(&g_deviceListusb);
    GlobalUserSignal::getInstance()->setDeviceList(g_deviceListusb);
    g_sane_object->g_deviceList = g_deviceListusb;
    if (sane_status) {
        qDebug() << "Cannot get scan devices, sane_status = " << sane_status;
        return;
    }

    qDebug() << "scan device tag!";
    for (int i = 0; g_deviceListusb[i]; ++i) {
        qDebug() << "mark-Name-usb: " << g_deviceListusb[i]->name
                 << "mark-Vendor-usb: " << g_deviceListusb[i]->vendor
                 << "mark-Model-usb: " << g_deviceListusb[i]->model
                 << "mark-Type-usb: " << g_deviceListusb[i]->type;

        name = QString("%1 %2 %3").arg(g_deviceListusb[i]->vendor).arg(g_deviceListusb[i]->model).arg(g_deviceListusb[i]->name);
        names << name;
    }
    g_sane_object->setSaneNames(names);
}
