#ifndef DEVICE_MONITOR_H
#define DEVICE_MONITOR_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QThread>
#include <QDebug>

#include "device_information.h"




class DeviceMonitorBackend : public QObject
{
    Q_OBJECT

private:
    DeviceMonitorBackend();
public:
    static DeviceMonitorBackend *getInstance();
    ~DeviceMonitorBackend();
    bool doDeviceAdd      (const QString &);
    bool doDeviceRemove   (const QString &);

private:
    QMap<QString, DeviceInformation> m_mpDeviceInformationMap;
    void detectStart ();
    bool usbDeviceIdentify (const QString &);


signals:
    void start();
    bool usbDeviceAdd      (const QString &);
    bool usbDeviceRemove   (const QString &);
    void findUsbConnect    (DeviceInformation);
    void findUsbDisconnect (QString);
};

#endif // DEVICE_MONITOR_H
