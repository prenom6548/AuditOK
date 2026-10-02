#include "waylandinputdevicemonitor.h"

#include <QDBusConnection>

WaylandInputDeviceMonitor::WaylandInputDeviceMonitor(QObject* parent)
    : InputDeviceMonitor(parent)
{
    initDeviceListMonitor();
}

void WaylandInputDeviceMonitor::initDeviceListMonitor()
{
    QDBusConnection::sessionBus().connect(QString("org.ukui.KWin"),
                                          QStringLiteral("/org/ukui/KWin/InputDevice"),
                                          QStringLiteral("org.ukui.KWin.InputDeviceManager"),
                                          QStringLiteral("deviceAdded"), this, SLOT(onDeviceAdded(QString)));
    QDBusConnection::sessionBus().connect(QString("org.ukui.KWin"),
                                          QStringLiteral("/org/ukui/KWin/InputDevice"),
                                          QStringLiteral("org.ukui.KWin.InputDeviceManager"),
                                          QStringLiteral("deviceRemoved"), this, SLOT(onDeviceRemoved(QString)));
}

void WaylandInputDeviceMonitor::onDeviceAdded(QString deviceEvent)
{
    int deviceId;
    deviceId = deviceEvent.remove("event").toInt();

    emit deviceAdded(deviceId);
}

void WaylandInputDeviceMonitor::onDeviceRemoved(QString deviceEvent)
{
    int deviceId;
    deviceId = deviceEvent.remove("event").toInt();

    emit deviceRemoved(deviceId);
}
