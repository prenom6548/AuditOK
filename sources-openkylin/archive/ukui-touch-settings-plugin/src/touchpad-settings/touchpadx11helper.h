#ifndef TOUCHPADX11HELPER_H
#define TOUCHPADX11HELPER_H

#include <QDebug>
#include <QX11Info>
extern "C" {
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>
}

class TouchPadX11Helper
{
public:
    TouchPadX11Helper();

    static XDevice* deviceIsTouchpad (XDeviceInfo * deviceinfo);
    static bool deviceHasProperty (XDevice * device, const char * property_name);
};

#endif // TOUCHPADX11HELPER_H
