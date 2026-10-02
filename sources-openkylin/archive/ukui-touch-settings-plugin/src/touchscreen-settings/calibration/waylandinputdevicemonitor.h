#ifndef WAYLANDINPUTDEVICEMONITOR_H
#define WAYLANDINPUTDEVICEMONITOR_H

#include "inputdevicemonitor.h"

class WaylandInputDeviceMonitor : public InputDeviceMonitor
{
    Q_OBJECT
public:
    explicit WaylandInputDeviceMonitor(QObject* parent = nullptr);

private slots:
    void onDeviceAdded(QString deviceEvent);
    void onDeviceRemoved(QString deviceEvent);

private:
    void initDeviceListMonitor();

};

#endif // WAYLANDINPUTDEVICEMONITOR_H
