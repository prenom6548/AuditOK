#ifndef DEVICEMONITORTHREAD_H
#define DEVICEMONITORTHREAD_H

#include <QThread>
#include <QMap>

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

    QMap<QString, qint64> lastDeviceEvents; // key: devpath, value: timestamp of last event
    const qint64 EVENT_TIMEOUT = 2000; // 2 秒内重复的事件会被忽略
    const qint64 RECORD_LIFETIME = 5000; // 超过 5 秒的记录会被清理


    void initDevPaths();

    QString getDeviceID(struct udev_device *dev);
};

#endif // DEVICEMONITORTHREAD_H
