#include "devicemonitor.h"

#include "inputdevicemonitor.h"

#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/config.h>
#include <KF5/KScreen/kscreen/configmonitor.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>

#include <QTimer>

DeviceMonitor* DeviceMonitor::getInstance()
{
    static DeviceMonitor s_monitor;
    return &s_monitor;
}

DeviceMonitor::DeviceMonitor(QObject *parent)
    : QObject(parent)
{
    initInputMonitor();
    initOutputMonitor();
}

void DeviceMonitor::initInputMonitor()
{
    InputDeviceMonitor* monitor = InputDeviceMonitor::create(this);

    connect(monitor, &InputDeviceMonitor::deviceAdded, this, [this](int deviceId){
        emit deviceAdded(DeviceType::InputDevice, deviceId);
    });
    connect(monitor, &InputDeviceMonitor::deviceRemoved, this, [this](int deviceId){
        emit deviceRemoved(DeviceType::InputDevice, deviceId);
    });
    connect(monitor, &InputDeviceMonitor::deviceEnabled, this, [this](int deviceId){
        emit deviceEnabled(DeviceType::InputDevice, deviceId);
    });
    connect(monitor, &InputDeviceMonitor::deviceDisabled, this, [this](int deviceId){
        emit deviceDisabled(DeviceType::InputDevice, deviceId);
    });
}

void DeviceMonitor::initOutputMonitor()
{
    KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation;
    if (!op->exec()) {
        qWarning() << op->errorString();
    }

    m_config = op->config();
    KScreen::ConfigMonitor::instance()->addConfig(m_config);

    connect(m_config.data(), &KScreen::Config::outputAdded,
            this, [this](const KScreen::OutputPtr &output){
        QTimer::singleShot(100, this, [=]() {
            emit deviceAdded(DeviceType::OutputDevice, output.data()->id());
        });
    });
    connect(m_config.data(), &KScreen::Config::outputRemoved,
            this, [this](int id){
        emit deviceRemoved(DeviceType::OutputDevice, id);
    });

    for (const KScreen::OutputPtr &output : m_config->outputs()) {
        connect(output.data(), &KScreen::Output::isConnectedChanged, this, [output, this](){
            onOutputDeviceChanged(output);
        });
    }
}

void DeviceMonitor::onOutputDeviceChanged(const KScreen::OutputPtr &output)
{
    if (output.data()->isConnected()) {
        emit deviceAdded(DeviceType::OutputDevice, output.data()->id());
    } else {
        emit deviceRemoved(DeviceType::OutputDevice, output.data()->id());
    }

}
