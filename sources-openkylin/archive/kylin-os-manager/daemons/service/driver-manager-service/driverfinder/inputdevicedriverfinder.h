#ifndef INPUTDEVICEDRIVERFINDER_H
#define INPUTDEVICEDRIVERFINDER_H

#include <QObject>
#include <QThread>
#include <QList>
#include "../hardwareinfo/hardwareinfo.h"

class inputdevicedriverfinder : public QObject {
Q_OBJECT
public:
    explicit inputdevicedriverfinder(QObject *parent = nullptr);

    ~inputdevicedriverfinder();

    QList<HardWareInfo> getInputDevInfoList();

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
    QList<HardWareInfo> m_inputDevInfoList;

    bool m_isServerAddressConnected;

    void dowork();

    void convertStrToDrinfo(QStringList drivers, DriverInfo &driverInfo);

    int tryConnectFromHost(const QString &hostname);

    QStringList getPackagesNameFromHttp(HardWareInfo &hardwareinfo);
    // 用于判断本地是否安装改驱动以及驱动对应版本
    void checkDriverVersion();

    void getDriverContent();

    void upgradeableCheck();

    QString apiServerAddressGet();
};


#endif //INPUTDEVICEDRIVERFINDER_H
