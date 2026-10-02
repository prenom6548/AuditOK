/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2025 Tianjin KYLIN Information Technology Co., Ltd.
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


#include <QX11Info>
#include <QDBusInterface>

#include "usd_base_class.h"
#include "touch_calibrate_x.h"

TouchCalibrateX::TouchCalibrateX()
{
    m_display = QX11Info::display();
}

void TouchCalibrateX::getScreenList()
{
    int event_base, error_base, major, minor;
    Window  root;
    XRRScreenResources *res;

    if (!XRRQueryExtension(m_display, &event_base, &error_base) ||
        !XRRQueryVersion(m_display, &major, &minor)) {
        SYS_LOG(LOG_ERR, "RandR extension missing.");
        return;
    }
    root = DefaultRootWindow(m_display);
    if (major >= 1 && minor >= 5) {
        res = XRRGetScreenResources(m_display, root);
        if (!res) {
            SYS_LOG(LOG_ERR, "get screen resources failed");
            return;
        }

        for (int o = 0; o < res->noutput; ++o) {
            XRROutputInfo *output = XRRGetOutputInfo(m_display, res, res->outputs[o]);
            if (!output){
                SYS_LOG(LOG_ERR, "could not get output.");
                continue;
            }
            if (output->connection == RR_Connected) {
                ScreenInfoPtr screen(new ScreenInfo);
                screen->name = QString::fromLatin1(output->name);
                screen->width = output->mm_width;
                screen->height = output->mm_height;
                m_screenInfoMap.insert(screen->name, screen);
                SYS_LOG(LOG_DEBUG, "%s  width : %d height : %d",
                        screen->name.toLatin1().data(), screen->width, screen->height);
            }
            XRRFreeOutputInfo(output);
        }
        XRRFreeScreenResources(res);
    }
}

void TouchCalibrateX::getTouchDeviceList()
{
    int ndevices;
    XDeviceInfo* deviceList = XListInputDevices(m_display ,&ndevices);
    for (int i = 0 ; i < ndevices ; ++i) {
        XDeviceInfo device = deviceList[i];
        if (device.type == XInternAtom(m_display, XI_TOUCHSCREEN, False)) {
            addTouchDevice(device, m_touchScreenList);
        } else if (device.type == XInternAtom(m_display, XI_TABLET, False) ||
                   device.type == XInternAtom(m_display, "STYLUS", False)) {
            addTouchDevice(device, m_tabletList);
        }
    }
    XFreeDeviceList(deviceList);
}

void TouchCalibrateX::calibrateTouchScreen()
{
    //首先配置文件映射
    Q_FOREACH (const TouchConfigPtr& touchConfig, m_touchConfigList) {
        Q_FOREACH (const TouchDevicePtr& touch , m_touchScreenList) {
            if (touch->name == touchConfig->sTouchName) {
                if (touchConfig->product.isValid) { // 检查配置里product id 是否有效，无效则不进行判断
                    if (touchConfig->product != touch->product) {
                        continue;
                    }
                }
                const ScreenInfoPtr& screen = m_screenInfoMap.value(touchConfig->sMonitorName);
                if (screen.data()) {
                    calibrateDevice(touch->id, screen->name);
                    touch->isMapped = true;
                    screen->isMapped = true;
                }
            }
        }
    }
    autoMaticMapping(m_touchScreenList, m_screenInfoMap);
}

void TouchCalibrateX::calibrateDevice(QVariant id, const QString &output)
{
    QStringList arguments;
    arguments << "--map-to-output" << QString::number(id.toInt()) << output;
    QProcess process;
    process.setProgram("xinput");
    process.setArguments(arguments);
    if (!process.startDetached()) {
        SYS_LOG(LOG_DEBUG, "xinput map to output failed");
    }
    SYS_LOG(LOG_DEBUG, "xinput touch device map to output [%d : %s]", id, output.toLatin1().data());
}

QString TouchCalibrateX::getDeviceNode(int id)
{
    QString node;
    Atom  prop;
    Atom act_type;
    int  act_format;
    unsigned long nitems, bytes_after;
    unsigned char *data;
    prop = XInternAtom(m_display, XI_PROP_DEVICE_NODE, False);
    if (!prop) {
        return node;
    }

    if (XIGetProperty(m_display, id, prop, 0, 1000, False,
                      AnyPropertyType, &act_type, &act_format, &nitems, &bytes_after, &data) == Success) {
         node = QString::fromLatin1(reinterpret_cast<char*>(data));
         XFree(data);
    }
    return node;
}

QVariantList TouchCalibrateX::getDeviceProductId(int id)
{
    QVariantList valueList;
    Atom           real_type;
    int            real_format;
    unsigned long  nitems;
    unsigned long  bytes_after;
    unsigned char* data = nullptr;
    unsigned char* dataPtr = nullptr;

    Atom prop = XInternAtom(QX11Info::display(), XI_PROP_PRODUCT_ID, False);

    if (Success == XIGetProperty(QX11Info::display(), id, prop, 0,
                                1000, False, AnyPropertyType,  &real_type,
                                &real_format, &nitems, &bytes_after, &data)) {
        dataPtr = data;
        for (int i = 0; i < nitems; ++i) {
            if (real_type == XA_INTEGER && real_format == 32) {
                valueList << *(reinterpret_cast<int32_t*>(dataPtr));
            }
            dataPtr += real_format / 8;
        }
        XFree(data);
    }
    return valueList;
}

void TouchCalibrateX::addTouchDevice(const XDeviceInfo &device, TouchDeviceList &touchList)
{
    const QString& node = getDeviceNode(device.id);
    const QVariantList& product = getDeviceProductId(device.id);
    if (!node.isEmpty()) {
        TouchDevicePtr dev = TouchDevicePtr(new TouchDevice);
        dev->id = device.id;
        dev->calibrateName = dev->id;
        dev->name = QString::fromLatin1(device.name);
        dev->node = node;
        getTouchSize(node.toLatin1().data(), dev->width, dev->height);
        if (product.count() > 1) {
            dev->product.isValid = true;
            dev->product.vid = product[0].toInt();
            dev->product.pid = product[1].toInt();
        }
        touchList.append(dev);
        SYS_LOG(LOG_DEBUG, "%s id : %d node: %s width : %d height : %d",
                dev->name.toLatin1().data(), dev->id, dev->node.toLatin1().data(), dev->width, dev->height);
    }
}
