#ifndef DEVICEMONITORTHREAD_H
#define DEVICEMONITORTHREAD_H

#include <QThread>

class DeviceMonitorThread : public QThread {
Q_OBJECT
public:
    explicit DeviceMonitorThread(QObject *parent = nullptr);

    void stop();

signals:

    void deviceInsRem(QString action, const QString &deviceType);

protected:
    void run() override;

private:
    bool m_running = true;
    QString m_devPath = "";
    QString m_currentAction = "";
    QStringList devInfos;

    void initDevPaths();
    QString getDeviceID(struct udev_device *dev);
};

#endif // DEVICEMONITORTHREAD_H
