#ifndef PRINTERDRIVERFINDER_H
#define PRINTERDRIVERFINDER_H

#include <QObject>
#include <QThread>
#include <QList>
#include "../hardwareinfo/hardwareinfo.h"

class printerDriverFinder : public QObject
{
    Q_OBJECT
public:
    printerDriverFinder(QObject *parent = nullptr);
    ~printerDriverFinder();

    QList<HardWareInfo> getPrinterInfoList();
    QThread *m_thread = nullptr;

    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }

Q_SIGNALS:
    void finished();
    void succeed();
    void failed();

private:
    QList<HardWareInfo> m_usbInfoList;
    QList<HardWareInfo> m_printerInfoList;

    bool m_isServerAddressConnected;
    bool m_cacheExist = true;

    void dowork();
    void convertStrToDrinfo(QStringList drivers, DriverInfo &driverInfo);
    int tryConnectFromHost(const QString &hostname);
    QStringList getPackagesNameFromHttp(HardWareInfo &hardwareinfo);
    void checkDriverVersion();                   // 用于判断本地是否安装改驱动以及驱动对应版本
    void getDriverContent();
    void upgradeableCheck();
    QString apiServerAddressGet();
};

#endif // PRINTERDRIVERFINDER_H
