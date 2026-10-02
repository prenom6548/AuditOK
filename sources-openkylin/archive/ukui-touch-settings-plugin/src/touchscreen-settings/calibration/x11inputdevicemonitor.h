#ifndef X11INPUTDEVICEMONITOR_H
#define X11INPUTDEVICEMONITOR_H

#include "inputdevicemonitor.h"

class X11InputDeviceMonitor : public InputDeviceMonitor
{
public:
    explicit X11InputDeviceMonitor(QObject* parent = nullptr);
};

#endif // X11INPUTDEVICEMONITOR_H
