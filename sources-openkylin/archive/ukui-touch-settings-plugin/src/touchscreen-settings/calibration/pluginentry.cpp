#include "pluginentry.h"

#include <QDebug>
#include <ukcc/interface/ukcccommon.h>

#include "touchscreenwidget.h"
#include "devicemanager.h"

PluginEntry::PluginEntry(QObject* parent)
    : QObject(parent)
    , m_widget(nullptr)
    , m_deviceManager(DeviceManager::getInstance())
    , m_widgetEnable(widgetEnable())
{
    initConnection();
}

bool PluginEntry::widgetEnable() const
{
    auto devices = m_deviceManager->getTouchDevices();
    return !devices.isEmpty();
}

void PluginEntry::initConnection()
{
    connect(m_deviceManager, &DeviceManager::deviceAdded, this, &PluginEntry::onDeviceChanged);
    connect(m_deviceManager, &DeviceManager::deviceEnabled, this, &PluginEntry::onDeviceChanged);
    connect(m_deviceManager, &DeviceManager::deviceRemoved, this, &PluginEntry::onDeviceChanged);
    connect(m_deviceManager, &DeviceManager::deviceDisabled, this, &PluginEntry::onDeviceChanged);
}

void PluginEntry::onDeviceChanged(DeviceType deviceType)
{
    qDebug() << (deviceType == DeviceType::InputDevice ? "InputDevice" : "OutputDevice")
             << "list changed";

    if (deviceType == DeviceType::InputDevice)
        updateWidgetEnable();

    resetWidgetCombobox(deviceType);
}

void PluginEntry::updateWidgetEnable()
{
    QList<Device> devices = m_deviceManager->getTouchDevices();
    bool hasTouchDevice = !devices.isEmpty();
    if (m_widgetEnable != hasTouchDevice) {
        m_widgetEnable = hasTouchDevice;
        emit widgetEnableChanged(m_widgetEnable);
    }
}

void PluginEntry::resetWidgetCombobox(DeviceType deviceType)
{
    if (!m_widget)
        return;

    switch (deviceType) {
    case DeviceType::InputDevice:
        m_widget->resetTouchList(m_deviceManager->getTouchDevices());
        break;
    case DeviceType::OutputDevice:
        m_widget->resetOutputList(m_deviceManager->getOutputDevices());
        break;
    default:
        break;
    }
}

QWidget* PluginEntry::widget()
{
    if (!m_widget) {
        m_widget = createWidget();
        initWidgetConnection();
    }

    return m_widget;
}

TouchScreenWidget* PluginEntry::createWidget()
{
    TouchScreenWidget* widget = new TouchScreenWidget;
    widget->resetOutputList(m_deviceManager->getOutputDevices());
    widget->resetTouchList(m_deviceManager->getTouchDevices());

    return widget;
}

void PluginEntry::initWidgetConnection()
{
    connect(m_widget, &TouchScreenWidget::mapButtonClicked, this, [this](){
        Device currentTouch = m_widget->currentTouchDevice();
        Device currenOutPut = m_widget->currentOutputDevice();
        m_deviceManager->mapToOutput(currentTouch.deviceId, currenOutPut.name);
        buriedSettings("TouchCalibratButton", "clicked", QString());
    });
    connect(m_widget, &TouchScreenWidget::calibratButtonClicked, this, [this](){
        Device currentTouch = m_widget->currentTouchDevice();
        Device currenOutPut = m_widget->currentOutputDevice();
        m_deviceManager->calibrateTouch(currentTouch.deviceId, currenOutPut.name);
        buriedSettings("TouchCalibratButton", "clicked", QString());
    });
    connect(m_widget, &TouchScreenWidget::currentOutputChanged, this, [this](Device device) {
        buriedSettings("ChangeOutPutSelect", "select", device.name);
    });
    connect(m_widget, &TouchScreenWidget::currentTouchChanged, this, [this](Device device) {
        buriedSettings("ChangeTouchScreenSelect", "select", QString::number(device.deviceId));
    });
}

void PluginEntry::buriedSettings(const QString& settingsName,
                                 const QString& action,
                                 const QString& value) const
{
    ukcc::UkccCommon::buriedSettings("TouchCalibrate", settingsName, action, value);
}
