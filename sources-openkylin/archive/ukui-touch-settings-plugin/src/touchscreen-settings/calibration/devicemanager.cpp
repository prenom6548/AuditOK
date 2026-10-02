#include "devicemanager.h"
#include "devicehelper.h"
#include "devicemonitor.h"
#include "configmanager.h"

#include <QDir>
#include <QDBusMessage>
#include <QDBusConnection>

static const QString TOUCHSCREEN_CFG_PATH = "/.config/touchcfg.ini";

DeviceManager* DeviceManager::getInstance()
{
    static DeviceManager s_manager;
    return &s_manager;
}

DeviceManager::DeviceManager(QObject* parent)
    : QObject(parent)
    , m_deviceMonitor(DeviceMonitor::getInstance())
    , m_configManager(new ConfigManager(QDir::homePath() + TOUCHSCREEN_CFG_PATH))
{
    initDeviceManager();
}

void DeviceManager::initDeviceManager()
{
    connect(m_deviceMonitor, &DeviceMonitor::deviceAdded, this, &DeviceManager::deviceAdded);
    connect(m_deviceMonitor, &DeviceMonitor::deviceRemoved, this, &DeviceManager::deviceRemoved);
    connect(m_deviceMonitor, &DeviceMonitor::deviceEnabled, this, &DeviceManager::deviceEnabled);
    connect(m_deviceMonitor, &DeviceMonitor::deviceDisabled, this, &DeviceManager::deviceDisabled);
}

QList<Device> DeviceManager::getTouchDevices() const
{
    return DeviceHelper::getTouchDevices();
}

QList<Device> DeviceManager::getOutputDevices() const
{
    return DeviceHelper::getOutputs();
}

QString DeviceManager::getInputName(int deviceId) const
{
    return DeviceHelper::getInputName(deviceId);
}

QString DeviceManager::getInputProductId(int deviceId) const
{
    return DeviceHelper::getInputProductId(deviceId);
}

QString DeviceManager::getInputSerial(int deviceId) const
{
    return DeviceHelper::getInputSerial(deviceId);
}

QString DeviceManager::getDeviceSizeFromDeviceId(int deviceId) const
{
    return DeviceHelper::getDeviceSize(deviceId);
}

bool DeviceManager::isPrimaryOutput(int deviceId) const
{
    return DeviceHelper::isPrimaryOutput(deviceId);
}

void DeviceManager::mapToOutput(int inputDeviceId, const QString &outputName) const
{
    DeviceHelper::mapToOutput(inputDeviceId, outputName);

    TouchDeviceConfigInfo configInfo;
    configInfo.deviceName = getInputName(inputDeviceId);
    configInfo.productId = getInputProductId(inputDeviceId);
    configInfo.serial = getInputSerial(inputDeviceId);

    m_configManager->saveConfigInfo(outputName, configInfo);
}

void DeviceManager::calibrateTouch(int touchDeviceId, QString outputName)
{
    QString id = QString::number(touchDeviceId);
    QDBusMessage msg = QDBusMessage::createSignal("/com/control/center/calibrator", "com.control.center.calibrator.interface", "calibratorEvent");
    msg.setArguments(QList<QVariant>{ id + "," + outputName});
    QDBusConnection::systemBus().send(msg);
}
