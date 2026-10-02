#ifndef USBLISTENERTHREAD_H
#define USBLISTENERTHREAD_H

#include <QThread>
#include <QCoreApplication>
#include <libudev.h>
#include <QString>

struct DeviceInfo {
    QString devPath;
    QString devNode;
    QString VID;
    QString PID;
    QString vendor;
    QString model;
    QString devType;
};

class USBListenerThread : public QThread {

Q_OBJECT

public:
    USBListenerThread(QObject *parent = nullptr);

    ~USBListenerThread();

public slots:

    void initializeConnectedDevices();

signals:

    void deviceChange(QString aciton, QString vid, QString pid, QString vendor, QString model, QString devtype);


protected:

    void run() override;

private:
    QList<DeviceInfo> m_devicesInfo;

    struct udev *m_udev;
    struct udev_monitor *m_monitor;
    int fd;
    QStringList devInfos;

    void processUsbDevice(struct udev_device *dev);

    QString getDeviceTypeFromPath(QString path);

    QString getResultFromCommand(QStringList command);

    void collectDeviceInfo(struct udev_device *dev);

    void initDevPaths();

    QString getDeviceID(struct udev_device *dev);

};


#endif //USBLISTENERTHREAD_H
