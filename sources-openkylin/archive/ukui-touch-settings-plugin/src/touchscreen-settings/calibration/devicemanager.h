#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include "common.h"

class DeviceMonitor;
class ConfigManager;
class DeviceManager : public QObject
{
    Q_OBJECT
public:
    static DeviceManager* getInstance();

    QList<Device> getTouchDevices() const;
    QList<Device> getOutputDevices() const;
    QString getInputName(int deviceId) const;
    QString getInputProductId(int deviceId) const;
    QString getInputSerial(int deviceId) const;
    QString getDeviceSizeFromDeviceId(int deviceId) const;
    void mapToOutput(int inputDeviceId, const QString& outputName) const;

    bool isPrimaryOutput(int deviceId) const;
    void calibrateTouch(int touchDeviceId, QString outputName);

signals:
    void deviceAdded(DeviceType type, int deviceid);
    void deviceRemoved(DeviceType type, int deviceid);
    void deviceEnabled(DeviceType type, int deviceid);
    void deviceDisabled(DeviceType type, int deviceid);

private:
    explicit DeviceManager(QObject *parent = nullptr);
    void initDeviceManager();

    DeviceMonitor* m_deviceMonitor;
    ConfigManager* m_configManager;
};

#endif // DEVICEMANAGER_H
