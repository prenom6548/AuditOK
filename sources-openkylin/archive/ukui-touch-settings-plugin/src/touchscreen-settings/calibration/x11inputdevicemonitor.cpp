#include "x11inputdevicemonitor.h"
#include "inputeventfilter.h"

#include <QCoreApplication>


X11InputDeviceMonitor::X11InputDeviceMonitor(QObject* parent)
    : InputDeviceMonitor(parent)
{
    InputEventFilter* filter = new InputEventFilter(this);
    qApp->installNativeEventFilter(filter);

    connect(filter, &InputEventFilter::deviceAdded, this, &X11InputDeviceMonitor::deviceAdded);
    connect(filter, &InputEventFilter::deviceRemoved, this, &X11InputDeviceMonitor::deviceRemoved);
    connect(filter, &InputEventFilter::deviceEnabled, this, &X11InputDeviceMonitor::deviceEnabled);
    connect(filter, &InputEventFilter::deviceDisabled, this, &X11InputDeviceMonitor::deviceDisabled);
}
