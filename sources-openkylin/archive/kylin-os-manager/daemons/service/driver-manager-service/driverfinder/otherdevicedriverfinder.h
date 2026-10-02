#ifndef OTHERDEVICEDRIVERFINDER_H
#define OTHERDEVICEDRIVERFINDER_H

#include <QObject>
#include <QThread>
#include <QList>
#include "../hardwareinfo/hardwareinfo.h"

class otherDeviceDriverFinder : public QObject {
Q_OBJECT
public:
    explicit otherDeviceDriverFinder(QObject *parent = nullptr);

    ~otherDeviceDriverFinder();

    QList<HardWareInfo> getOtherDevInfoList();

    QThread *m_thread = nullptr;

    void startWorker() {
        if (m_thread != nullptr)
            m_thread->start();
    }

Q_SIGNALS:

    void finished();

    void succeed();

    void failed();

private:
    QList<HardWareInfo> m_usbInfoList;
    QList<HardWareInfo> m_otherDevInfoList;

    bool m_isServerAddressConnected;

    void dowork();

    void convertStrToDrinfo(QStringList drivers, DriverInfo &driverInfo);

    int tryConnectFromHost(const QString &hostname);

    QStringList getPackagesNameFromHttp(HardWareInfo &hardwareinfo);

    void checkDriverVersion();                   // 用于判断本地是否安装改驱动以及驱动对应版本

    void getDriverContent();

    void upgradeableCheck();

    QString apiServerAddressGet();

};


#endif //OTHERDEVICEDRIVERFINDER_H
