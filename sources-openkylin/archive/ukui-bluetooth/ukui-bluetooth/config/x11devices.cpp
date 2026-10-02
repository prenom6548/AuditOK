/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "x11devices.h"

#include <ukui-log4qt.h>

extern "C"{
    #include <X11/Xlib.h>
    #include <X11/extensions/XInput.h>
}


x11Devices::x11Devices(QObject *parent) : QObject(parent)
{

}


unsigned int x11Devices::getSystemCurrentMouseAndTouchPadDevCount()
{
    unsigned int count = 0 ;
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        // 错误处理
        //KyWarning() << "display error!";
        XCloseDisplay(display);
        exit(1);
    }

    XDeviceInfo *devices;
    int num_devices;
    devices = XListInputDevices(display, &num_devices);
    if (devices == NULL) {
        //KyWarning() << "devices error!";
        XCloseDisplay(display);
        exit(1);
    }

    Atom mouse_prop = XInternAtom(display,STR_MOUSE,false);
    Atom touchPad_prop = XInternAtom(display,STR_TOUCHPAD,false);



    for (int i = 0; i < num_devices; i++) {
        if (devices[i].type == mouse_prop || devices[i].type == touchPad_prop) {
            QString dev_name = QString(devices[i].name);
            KyDebug() << "dev_name:"  <<dev_name ;
            KyDebug() << "mouse/touchPad"  <<" id: " <<  devices[i].id << "devices name:" << devices[i].name ;
            if (dev_name.contains(STR_MOUSE,Qt::CaseInsensitive)
                    || dev_name.contains(STR_TOUCHPAD,Qt::CaseInsensitive)
                    || dev_name.contains(STR_TRACKPOINT,Qt::CaseInsensitive)
                    || dev_name.contains("MS",Qt::CaseInsensitive)
                    )
                count ++;
        }
    }
    //KyDebug() << "devices count:" << count;

    XFreeDeviceList(devices);
    XCloseDisplay(display);

    return count;
}


unsigned int x11Devices::getSystemCurrentKeyBoardDevCount()
{
    unsigned int count = 0 ;
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        // 错误处理
        //KyWarning() << "display error!";
        XCloseDisplay(display);
        exit(1);
    }

    XDeviceInfo *devices;
    int num_devices;
    devices = XListInputDevices(display, &num_devices);
    if (devices == NULL) {
        //KyWarning() <<  "devices error!";
        XCloseDisplay(display);
        exit(1);
    }

    Atom keyBoard_prop = XInternAtom(display,STR_KEYBOARD,false);

    //KyDebug() << "keyBoard_prop:" << keyBoard_prop;
    for (int i = 0; i < num_devices; i++) {

        if(devices[i].type == keyBoard_prop)
        {
            QString dev_name = QString(devices[i].name);
            KyDebug() << "dev_name:"  <<dev_name ;
            KyDebug() << "keyBoard" <<" id: " <<  devices[i].id << "devices name:" << devices[i].name ;
            if (dev_name.contains(STR_KEYBOARD,Qt::CaseInsensitive)
                    )
                count++;
        }
    }
    //KyDebug() << "devices count:" << count;

    XFreeDeviceList(devices);
    XCloseDisplay(display);

    return count;
}
