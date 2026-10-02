#include "touchpadx11helper.h"

TouchPadX11Helper::TouchPadX11Helper()
{

}

XDevice* TouchPadX11Helper::deviceIsTouchpad(XDeviceInfo *deviceinfo)
{
    XDevice *device;
    if (deviceinfo->type != XInternAtom (QX11Info::display(), XI_TOUCHPAD, true)) {
        return nullptr;
    }

    device = XOpenDevice (QX11Info::display(), deviceinfo->id);
    if(device == nullptr) {
        qDebug()<<"device== null";
        return nullptr;
    }

    if (deviceHasProperty(device, "libinput Tapping Enabled") ||
        deviceHasProperty(device, "Synaptics Off")) {
        return device;
    }
    XCloseDevice (QX11Info::display(), device);
    return nullptr;
}

bool TouchPadX11Helper::deviceHasProperty(XDevice *device, const char *property_name)
{
    Atom realtype, prop;
    int realformat;
    unsigned long nitems, bytes_after;
    unsigned char *data;

    prop = XInternAtom (QX11Info::display(), property_name, True);
    if (!prop) {
        return false;
    }

    if ((XGetDeviceProperty (QX11Info::display(), device, prop, 0, 1, False,
                            XA_INTEGER, &realtype, &realformat, &nitems,
                            &bytes_after, &data) == Success) && (realtype != None))
    {
        XFree (data);
        return true;
    }
    return false;
}
