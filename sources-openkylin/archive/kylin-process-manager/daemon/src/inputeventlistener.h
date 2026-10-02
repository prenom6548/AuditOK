#ifndef INPUTEVENTLISTENER_H
#define INPUTEVENTLISTENER_H

#include <QObject>
#include <QSocketNotifier>
#include <QTimer>
#include <QList>
#include <QMap>
#include <QFile>
#include <libudev.h>

class InputMonitor : public QObject {
    Q_OBJECT
public:
    explicit InputMonitor(QObject *parent = nullptr);
    ~InputMonitor();
    void stopListening();
Q_SIGNALS:
     void idle();
     void active();

private Q_SLOTS:
    void onDeviceActivity(int fd);
    void onIdle();

private:
    struct DeviceInfo {
        int fd;
        QSocketNotifier* notifier;
        QString path;
    };
    void initUdev();
    void scanExistingDevices();
    void monitorDevice(const QString &path);
    bool checkDeviceSupportsEvents(int fd);
    void readInputEvent(int fd);
    void removeAllDeviceListeners();
    void removeDevice(int fd);

    QMap<int, DeviceInfo> m_deviceMap;
    QTimer idleTimer;
    bool isIdle = false;
    QList<QSocketNotifier*> notifiers;
    QSocketNotifier* udev_notifier = nullptr;
    struct udev* udev = nullptr;
    struct udev_monitor* mon = nullptr;
    int mSecsidle = 65*1000;
};

#endif // INPUTEVENTLISTENER_H
