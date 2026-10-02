#include "inputdevicemonitor.h"
#include "x11inputdevicemonitor.h"
#include "waylandinputdevicemonitor.h"
#include "common.h"

InputDeviceMonitor* InputDeviceMonitor::create(QObject* parent)
{
    if(env::isWayland()) {
        return new WaylandInputDeviceMonitor(parent);
    }
    return new X11InputDeviceMonitor(parent);
}

InputDeviceMonitor::InputDeviceMonitor(QObject* parent)
    :QObject(parent)
{

}
