#ifndef DEVICEMONITOR_H
#define DEVICEMONITOR_H

#include <QObject>
#include <KF5/KScreen/kscreen/types.h>

#include "common.h"

class InputDeviceMonitor;
class DeviceMonitor : public QObject
{
    Q_OBJECT
public:
    static DeviceMonitor* getInstance();

signals:
    void deviceAdded(DeviceType type, int deviceid);
    void deviceRemoved(DeviceType type, int deviceid);
    void deviceEnabled(DeviceType type, int deviceid);
    void deviceDisabled(DeviceType type, int deviceid);

private slots:
    void onOutputDeviceChanged(const KScreen::OutputPtr& output);

private:
    explicit DeviceMonitor(QObject *parent = nullptr);
    void initInputMonitor();
    void initOutputMonitor();

    KScreen::ConfigPtr m_config = nullptr;
};

#endif // DEVICEMONITOR_H
