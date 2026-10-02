#ifndef HOTPLUGDEVDRIVERFINDER_H
#define HOTPLUGDEVDRIVERFINDER_H

#include <QObject>
#include <QThread>
#include <QList>
#include "../hardwareinfo/hardwareinfo.h"

class HotplugDevDriverFinder : public QObject {
Q_OBJECT
public:
    explicit HotplugDevDriverFinder(QObject *parent = nullptr);

    ~HotplugDevDriverFinder();

    void dowork(HardWareInfo &hwInfo, QString vid, QString pid, QString vendor, QString model, QString devtype);

private:
    void checkDriverVersion(HardWareInfo &hwInfo);

    void getDriverContent(HardWareInfo &hwInfo);

    void convertStrToDrinfo(QStringList drivers, DriverInfo &driverInfo);

    void upgradeableCheck(HardWareInfo &hwInfo);

    QString apiServerAddressGet();
};


#endif //HOTPLUGDEVDRIVERFINDER_H
