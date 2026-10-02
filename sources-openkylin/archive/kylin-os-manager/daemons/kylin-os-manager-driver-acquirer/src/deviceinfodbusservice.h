#ifndef KYLIN_OS_MANAGER_DEVICEINFODBUSSERVICE_H
#define KYLIN_OS_MANAGER_DEVICEINFODBUSSERVICE_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>

class DeviceInfoDbusService : public QObject {
Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.DeviceInfoService")
public:
    explicit DeviceInfoDbusService(QObject *parent = nullptr);

    bool rigisterService();

public Q_SLOTS:

    QString QueryDevices(const QString &pid,
                         const QString &vid,
                         const QString &vendor,
                         const QString &model,
                         const QString &type);
};


#endif //KYLIN_OS_MANAGER_DEVICEINFODBUSSERVICE_H
