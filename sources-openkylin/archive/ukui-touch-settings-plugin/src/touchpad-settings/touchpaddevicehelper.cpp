/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "touchpaddevicehelper.h"

#include <QDBusInterface>
#include <QX11Info>

extern "C" {
#include <X11/Xatom.h>
}

#include "settingscommon.h"

bool TouchpadDeviceHelper::hasTouchPad()
{
    if (SettingsCommon::isWaylandEnv()) {
        return hasTouchpadOnWayland();
    }
    else {
        return hasTouchpadOnX11();
    }
}

bool TouchpadDeviceHelper::hasTouchpadOnX11()
{
    XDeviceInfo* deviceInfo;
    int devicesNum;
    bool result;

    if (!supportXinputExtension()) {
        return false;
    }

    deviceInfo = XListInputDevices(QX11Info::display(), &devicesNum);
    if (deviceInfo == nullptr) {
        return false;
    }

    result = false;
    for (int i = 0; i < devicesNum; i++) {
        XDevice *device;

        device = deviceIsTouchpad(&deviceInfo[i]);
        if (device != nullptr) {
            result = true;
            break;
        }
    }
    if (deviceInfo != nullptr) {
        XFreeDeviceList(deviceInfo);
    }

    return result;
}

bool TouchpadDeviceHelper::supportXinputExtension()
{
    int op_code, event, error;

    return XQueryExtension(QX11Info::display(),
                           "XInputExtension",
                           &op_code,
                           &event,
                           &error);
}

XDevice* TouchpadDeviceHelper::deviceIsTouchpad (XDeviceInfo *deviceinfo)
{
    XDevice *device;
    if (deviceinfo->type != XInternAtom(QX11Info::display(), XI_TOUCHPAD, true)) {
        return nullptr;
    }

    device = XOpenDevice(QX11Info::display(), deviceinfo->id);
    if(device == nullptr) {
        qWarning() << "Can not open xinput device";
        return nullptr;
    }

    if (deviceHasProperty(device, "libinput Tapping Enabled") ||
        deviceHasProperty(device, "Synaptics Off")) {
        XCloseDevice(QX11Info::display(), device);
        return device;
    }
    XCloseDevice(QX11Info::display(), device);
    return nullptr;
}

bool TouchpadDeviceHelper::deviceHasProperty(XDevice *device, const char *propertyName)
{
    Atom realtype, prop;
    int realformat;
    unsigned long nitems, bytes_after;
    unsigned char* data;

    prop = XInternAtom(QX11Info::display(), propertyName, True);
    if (!prop) {
        return false;
    }

    auto status = XGetDeviceProperty(QX11Info::display(), device, prop, 0, 1, False,
                                     XA_INTEGER, &realtype, &realformat, &nitems,
                                     &bytes_after, &data);
    if ((status == Success) && (realtype != None)) {
        XFree(data);
        return true;
    }
    return false;
}

bool TouchpadDeviceHelper::hasTouchpadOnWayland()
{
    QStringList deviceList = getUKUIKwinDeviceList();
    bool isUKUI = true;
    if (deviceList.isEmpty()) {
        deviceList = getKDEKwinDeviceList();
        isUKUI = false;
    }

    if (deviceList.isEmpty()) {
        qWarning() << "Can not get device list from dbus of kde/ukui kwin";
        return false;
    }

    for (const QString& device : qAsConst(deviceList)) {
        if (deviceIsTouchpad(device, isUKUI))
            return true;
    }

    qDebug() << "Can not find touchpad on wayland, isUKUI:" << isUKUI;
    return false;
}

QStringList TouchpadDeviceHelper::getKDEKwinDeviceList()
{
    QDBusInterface iface("org.kde.KWin",
                         "/org/kde/KWin/InputDevice",
                         "org.kde.KWin.InputDeviceManager",
                         QDBusConnection::sessionBus(),
                         nullptr);
    if (!iface.isValid())
        return QStringList();

    QVariant reply = iface.property("devicesSysNames");
    return reply.toStringList();
}

QStringList TouchpadDeviceHelper::getUKUIKwinDeviceList()
{
    QDBusInterface iface("org.ukui.KWin",
                         "/org/ukui/KWin/InputDevice",
                         "org.ukui.KWin.InputDeviceManager",
                         QDBusConnection::sessionBus(),
                         nullptr);
    if (!iface.isValid())
        return QStringList();

    QVariant reply = iface.property("devicesSysNames");
    return reply.toStringList();
}

bool TouchpadDeviceHelper::deviceIsTouchpad(const QString& device, bool isUKUI)
{
    QString service, path, interface;
    if (isUKUI) {
        service = "org.ukui.KWin";
        path = "/org/ukui/KWin/InputDevice/"+ device;
        interface = "org.ukui.KWin.InputDevice";
    }
    else {
        service = "org.kde.KWin";
        path = "/org/kde/KWin/InputDevice/"+ device;
        interface = "org.kde.KWin.InputDevice";
    }

    QDBusInterface iface(service, path, interface, QDBusConnection::sessionBus(), nullptr);

    if (!iface.isValid()) {
        qWarning() << __FUNCTION__ << __LINE__ << "Can not get interface of dbus:"
                   << service << path << interface << "isUKUI" << isUKUI;
        return false;
    }

    return iface.property("touchpad").toBool();
}
