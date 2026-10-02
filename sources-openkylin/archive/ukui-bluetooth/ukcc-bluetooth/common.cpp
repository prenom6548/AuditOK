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

#include "common.h"

#include <ukui-log4qt.h>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusMessage>


extern "C"{
    #include <X11/Xlib.h>
    #include <X11/extensions/XInput.h>
}

#define  WLCOM_SERVICE           "com.kylin.Wlcom"
#define  WLCOM_OBJECT            "/com/kylin/Wlcom/Input"
#define  WLCOM_INTERFACE         "com.kylin.Wlcom.Input"
#define  WLCOM_METHED            "ListAllInputs"


Common::Common(QObject *parent) : QObject(parent)
{

}

static QStringList waylany_getdev(){
    QDBusInterface iface(WLCOM_SERVICE, WLCOM_OBJECT, WLCOM_INTERFACE, QDBusConnection::sessionBus());
    QStringList v;

    QDBusMessage reply = iface.call(WLCOM_METHED);
    if(reply.type() == QDBusMessage::ReplyMessage) {
        QVariant va = reply.arguments()[0];

        const QDBusArgument & dbusArg = va.value<QDBusArgument>();

        if (dbusArg.currentSignature() != "a(su)") {
            KyWarning() << "error data struct, " << dbusArg.currentSignature();
            return v;
        }

        dbusArg.beginArray();
        while (!dbusArg.atEnd()) {
            dbusArg.beginStructure();
            QString str;
            quint32 num;
            dbusArg >> str;
            dbusArg >> num;
            dbusArg.endStructure();
            v.append(str);
        }
        dbusArg.endArray();

    } else {
        KyWarning() << reply;
    }

    return v;
}


bool Common::isWayland()
{
    QString sessionType = getenv("XDG_SESSION_TYPE");
    if (!sessionType.compare("wayland", Qt::CaseInsensitive)) {
         return true;
    }
    return false;
}


static int x11_getSystemCurrentMouseAndTouchPadDevCount()
{
    unsigned int count = 0 ;
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        XCloseDisplay(display);
        return -1;
    }

    XDeviceInfo *devices;
    int num_devices;
    devices = XListInputDevices(display, &num_devices);
    if (devices == NULL) {
        XCloseDisplay(display);
        return -1;
    }

    Atom mouse_prop = XInternAtom(display,STR_MOUSE,false);
    Atom touchPad_prop = XInternAtom(display,STR_TOUCHPAD,false);

    for (int i = 0; i < num_devices; i++) {
        if (devices[i].type == mouse_prop || devices[i].type == touchPad_prop) {
            QString dev_name = QString(devices[i].name);
            if (dev_name.contains(STR_MOUSE,Qt::CaseInsensitive)
                || dev_name.contains(STR_TOUCHPAD,Qt::CaseInsensitive)
                || dev_name.contains(STR_TRACKPOINT,Qt::CaseInsensitive)
                || dev_name.contains("MS",Qt::CaseInsensitive)
                ) {
                count ++;
            }
        }
    }

    KyDebug() << "mouse devices count:" << count;

    XFreeDeviceList(devices);
    XCloseDisplay(display);

    return count;
}

static int wayland_getSystemCurrentMouseAndTouchPadDevCount()
{
    QStringList devices;
    int count = 0;

    devices = waylany_getdev();
    KyInfo() << devices;

    for(auto iter : devices) {

        QString devname = iter;

        if(devname.endsWith(STR_MOUSE, Qt::CaseInsensitive) ||
           devname.endsWith(STR_TOUCHPAD, Qt::CaseInsensitive) ||
           devname.endsWith(STR_TRACKPOINT, Qt::CaseInsensitive) ||
           devname.endsWith("MS", Qt::CaseInsensitive) ) {
            count++;
        }
    }

    KyInfo() << "mouse devices count:" << count;

    return count;
}

static int x11_getSystemCurrentKeyBoardDevCount()
{
    unsigned int count = 0 ;
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        XCloseDisplay(display);
        return -1;
    }

    XDeviceInfo *devices;
    int num_devices;
    devices = XListInputDevices(display, &num_devices);
    if (devices == NULL) {
        XCloseDisplay(display);
        return -1;
    }

    Atom keyBoard_prop = XInternAtom(display,STR_KEYBOARD,false);

    for (int i = 0; i < num_devices; i++) {

        if(devices[i].type == keyBoard_prop)
        {
            QString dev_name = QString(devices[i].name);
            if (dev_name.contains(STR_KEYBOARD,Qt::CaseInsensitive)) {
                count++;
            }
        }
    }

    KyDebug() << "keyBoard devices count:" << count;

    XFreeDeviceList(devices);
    XCloseDisplay(display);

    return count;
}

static int wayland_getSystemCurrentKeyBoardDevCount()
{
    QStringList devices;
    int count = 0;

    devices = waylany_getdev();
    KyInfo() << devices;

    for(auto iter : devices) {
        QString devname = iter;

        if(devname.endsWith(STR_KEYBOARD, Qt::CaseInsensitive)) {
            count++;
        }
    }

    KyInfo() << "keyBoard devices count:" << count;

    return count;
}



int Common::getSystemCurrentMouseAndTouchPadDevCount()
{
    if (Common::isWayland()) {
        return wayland_getSystemCurrentMouseAndTouchPadDevCount();
    }

    return x11_getSystemCurrentMouseAndTouchPadDevCount();
}

int Common::getSystemCurrentKeyBoardDevCount()
{
    if (Common::isWayland()) {
        return wayland_getSystemCurrentKeyBoardDevCount();
    }

    return x11_getSystemCurrentKeyBoardDevCount();
}
