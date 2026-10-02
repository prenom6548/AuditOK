#ifndef DEVICEMANAGE_H
#define DEVICEMANAGE_H

#include <QMap>
#include <QPointer>

#include "usbdevice.h"
#include "wifidevice.h"

namespace KProjection
{
class DeviceManage : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManage(QObject *parent = nullptr);

    virtual ~DeviceManage();

    bool connectDevice(UsbDevice::DeviceParams params);

    bool connectWifiDevice(WifiDevice::DeviceParams params);

    void disconnectDevice();

protected:
    void setGroupControlSignals(BaseDevice *host, BaseDevice *client, bool install);

    void setGroupControlHost(BaseDevice *host, bool install);

    void initDeviceSignals();

protected Q_SLOTS:
    void onDeviceDisconnect(QString serial);

    void onControlStateChange(BaseDevice *device, BaseDevice::GroupControlState oldState,
                              BaseDevice::GroupControlState newState);

    // neend convert frameSize to its frameSize
    void onMouseEvent(const QMouseEvent *from, const QSize &frameSize, const QSize &showSize);

    void onWheelEvent(const QWheelEvent *from, const QSize &frameSize, const QSize &showSize);

    void onKeyEvent(const QKeyEvent *from, const QSize &frameSize, const QSize &showSize);

private:
    QMap<QString, QPointer<BaseDevice>> m_deviceMap;
    QString m_script;
    BaseDevice::DeviceParams m_params;
    QPointer<BaseDevice> m_device;

Q_SIGNALS:
    void sigSocketDisconnect();
    void sigScreenConnected();
};
} // namespace KProjection
#endif // DEVICEMANAGE_H
