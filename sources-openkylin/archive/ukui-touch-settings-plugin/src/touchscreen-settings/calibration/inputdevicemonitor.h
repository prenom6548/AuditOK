#ifndef INPUTDEVICEMONITOR_H
#define INPUTDEVICEMONITOR_H

#include <QObject>

class InputDeviceMonitor : public QObject
{
    Q_OBJECT
public:
    static InputDeviceMonitor* create(QObject* parent = nullptr);
    explicit InputDeviceMonitor(QObject* parent = nullptr);

    InputDeviceMonitor(const InputDeviceMonitor& monitor) = delete;
    InputDeviceMonitor& operator=(const InputDeviceMonitor&) = delete;

signals:
    void deviceAdded(int deviceid);
    void deviceRemoved(int deviceid);
    void deviceEnabled(int deviceid);
    void deviceDisabled(int deviceid);

};
#endif // INPUTDEVICEMONITOR_H
