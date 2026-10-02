#include "devicemanage.h"
#include <QDebug>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QApplication>

#include "videoform.h"
#include "usbdevice.h"
#include "wifidevice.h"
#include "config.h"
#include "stream.h"

const int DM_MAX_DEVICES_NUM = 1;

namespace KProjection
{

DeviceManage::DeviceManage(QObject *parent) : QObject(parent)
{
    // set on QApplication before
    // bug: config path is error on mac
    int opengl = Config::getInstance().getDesktopOpenGL();
    if (0 == opengl) {
        QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    } else if (1 == opengl) {
        QApplication::setAttribute(Qt::AA_UseOpenGLES);
    } else if (2 == opengl) {
        QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    }

    Stream::init();
}

DeviceManage::~DeviceManage()
{
    qDebug() << "delete DeviceManage";
    disconnectDevice();
    Stream::deInit();
}

bool DeviceManage::connectDevice(BaseDevice::DeviceParams params)
{
    m_params = params;
    if (m_params.serial.trimmed().isEmpty()) {
        qWarning() << "can not find device";
        return false;
    }
    if (m_deviceMap.contains(m_params.serial)) {
        qWarning() << "devices is connected";
        return false;
    }
    if (DM_MAX_DEVICES_NUM < m_deviceMap.size()) {
        qWarning() << "over the maximum number of connections";
        return false;
    }
    if (!m_deviceMap.isEmpty()) {
        disconnectDevice();
    }
    m_device = new UsbDevice(m_params);
    initDeviceSignals();

    if (!m_script.isEmpty()) {
        m_device->updateScript(m_script);
    }
    return true;
}

bool DeviceManage::connectWifiDevice(BaseDevice::DeviceParams params)
{
    m_params = params;
    qDebug() << "connect Wifi Device";
    if (m_params.serial.trimmed().isEmpty()) {
        qWarning() << "can not find device";
        return false;
    }
    if (m_deviceMap.contains(m_params.serial)) {
        qWarning() << "devices is connected";
        return false;
    }
    if (DM_MAX_DEVICES_NUM < m_deviceMap.size()) {
        qInfo("over the maximum number of connections");
        return false;
    }
    m_device = new WifiDevice(m_params);
    initDeviceSignals();
    return true;
}

void DeviceManage::initDeviceSignals()
{
    if (m_device) {
        qInfo() << "connect device signals";
        connect(m_device, &BaseDevice::deviceDisconnect, this, &DeviceManage::onDeviceDisconnect);
        connect(m_device, &BaseDevice::controlStateChange, this, &DeviceManage::onControlStateChange);
        connect(m_device, &BaseDevice::sigSocketDisconnect, this, &DeviceManage::sigSocketDisconnect);
        connect(m_device, &BaseDevice::sigScreenConnected, this, &DeviceManage::sigScreenConnected);
        connect(m_device, &BaseDevice::sigScreenConnected, this, [this]() {
            m_deviceMap[m_params.serial] = m_device;
            Q_EMIT sigScreenConnected();
        });
        connect(m_device, &BaseDevice::sigVideoClose, this, [this]() {
            disconnectDevice();
        });
    }
}

void DeviceManage::disconnectDevice()
{
    qInfo() << "disconnect screen shot";
    QMapIterator<QString, QPointer<BaseDevice>> i(m_deviceMap);
    while (i.hasNext()) {
        i.next();
        if (i.value()) {
            delete i.value();
        }
    }
}

void DeviceManage::setGroupControlSignals(BaseDevice *host, BaseDevice *client, bool install)
{
    if (!host || !client) {
        return;
    }
    if (install) {
        connect(host, &BaseDevice::postTextInput, client, &BaseDevice::postTextInput);
        connect(host, &BaseDevice::setDeviceClipboard, client, &BaseDevice::setDeviceClipboard);
        connect(host, &BaseDevice::clipboardPaste, client, &BaseDevice::clipboardPaste);
        connect(host, &BaseDevice::pushFileRequest, client, &BaseDevice::pushFileRequest);
        connect(host, &BaseDevice::installApkRequest, client, &BaseDevice::installApkRequest);
        connect(host, &BaseDevice::screenshot, client, &BaseDevice::screenshot);
        connect(host, &BaseDevice::showTouch, client, &BaseDevice::showTouch);
    } else {
        disconnect(host, &BaseDevice::postTextInput, client, &BaseDevice::postTextInput);
        disconnect(host, &BaseDevice::setDeviceClipboard, client, &BaseDevice::setDeviceClipboard);
        disconnect(host, &BaseDevice::clipboardPaste, client, &BaseDevice::clipboardPaste);
        disconnect(host, &BaseDevice::pushFileRequest, client, &BaseDevice::pushFileRequest);
        disconnect(host, &BaseDevice::installApkRequest, client, &BaseDevice::installApkRequest);
        disconnect(host, &BaseDevice::screenshot, client, &BaseDevice::screenshot);
        disconnect(host, &BaseDevice::showTouch, client, &BaseDevice::showTouch);
    }
}

void DeviceManage::setGroupControlHost(BaseDevice *host, bool install)
{
    QMapIterator<QString, QPointer<BaseDevice>> i(m_deviceMap);
    while (i.hasNext()) {
        i.next();
        if (!i.value()) {
            continue;
        }
        if (i.value() == host) {
            continue;
        }
        if (install) {
            if (host) {
                setGroupControlSignals(host, i.value(), true);
            }
            Q_EMIT i.value()->setControlState(i.value(), BaseDevice::GroupControlState::GCS_CLIENT);
        } else {
            if (host) {
                setGroupControlSignals(host, i.value(), false);
            }
            Q_EMIT i.value()->setControlState(i.value(), BaseDevice::GroupControlState::GCS_FREE);
        }
    }
}

void DeviceManage::onDeviceDisconnect(QString serial)
{
    if (!serial.isEmpty() && m_deviceMap.contains(serial)) {
        if (m_deviceMap[serial]->controlState() == BaseDevice::GroupControlState::GCS_HOST) {
            setGroupControlHost(nullptr, false);
        }
        m_deviceMap.remove(serial);
    }
}

void DeviceManage::onControlStateChange(BaseDevice *device, BaseDevice::GroupControlState oldState,
                                        BaseDevice::GroupControlState newState)
{
    if (!device) {
        return;
    }
    // free to host
    if (oldState == BaseDevice::GroupControlState::GCS_FREE && newState == BaseDevice::GroupControlState::GCS_HOST) {
        // install direct control signals
        setGroupControlHost(device, true);
        // install convert control signals(frameSize need convert)
        connect(device, &BaseDevice::mouseEvent, this, &DeviceManage::onMouseEvent, Qt::UniqueConnection);
        connect(device, &BaseDevice::wheelEvent, this, &DeviceManage::onWheelEvent, Qt::UniqueConnection);
        connect(device, &BaseDevice::keyEvent, this, &DeviceManage::onKeyEvent, Qt::UniqueConnection);
        return;
    }
    // host to free
    if (oldState == BaseDevice::GroupControlState::GCS_HOST && newState == BaseDevice::GroupControlState::GCS_FREE) {
        // uninstall direct control signals
        setGroupControlHost(device, false);
        // uninstall convert control signals(frameSize need convert)
        disconnect(device, &BaseDevice::mouseEvent, this, &DeviceManage::onMouseEvent);
        disconnect(device, &BaseDevice::wheelEvent, this, &DeviceManage::onWheelEvent);
        disconnect(device, &BaseDevice::keyEvent, this, &DeviceManage::onKeyEvent);
        return;
    }
}

void DeviceManage::onMouseEvent(const QMouseEvent *from, const QSize &frameSize, const QSize &showSize)
{
    Q_UNUSED(frameSize)
    QMapIterator<QString, QPointer<BaseDevice>> i(m_deviceMap);
    while (i.hasNext()) {
        i.next();
        if (!i.value()) {
            continue;
        }
        if (i.value() == sender()) {
            continue;
        }
        // neend convert frameSize to its frameSize
        Q_EMIT i.value()->mouseEvent(from, i.value()->frameSize(), showSize);
    }
}

void DeviceManage::onWheelEvent(const QWheelEvent *from, const QSize &frameSize, const QSize &showSize)
{
    Q_UNUSED(frameSize)
    QMapIterator<QString, QPointer<BaseDevice>> i(m_deviceMap);
    while (i.hasNext()) {
        i.next();
        if (!i.value()) {
            continue;
        }
        if (i.value() == sender()) {
            continue;
        }
        // neend convert frameSize to its frameSize
        Q_EMIT i.value()->wheelEvent(from, i.value()->frameSize(), showSize);
    }
}

void DeviceManage::onKeyEvent(const QKeyEvent *from, const QSize &frameSize, const QSize &showSize)
{
    Q_UNUSED(frameSize)
    QMapIterator<QString, QPointer<BaseDevice>> i(m_deviceMap);
    while (i.hasNext()) {
        i.next();
        if (!i.value()) {
            continue;
        }
        if (i.value() == sender()) {
            continue;
        }
        // neend convert frameSize to its frameSize
        Q_EMIT i.value()->keyEvent(from, i.value()->frameSize(), showSize);
    }
}

} // namespace KProjection