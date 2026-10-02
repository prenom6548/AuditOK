#ifndef VIDEODRIVERFINDER_H
#define VIDEODRIVERFINDER_H

#include <QObject>
#include <QThread>
#include <QList>
#include "../hardwareinfo/hardwareinfo.h"

class videoDriverFinder : public QObject {
Q_OBJECT
public:
    explicit videoDriverFinder(QObject *parent = nullptr);

    ~videoDriverFinder();

    QList<HardWareInfo> getVideoInfoList();

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
    QList<HardWareInfo> m_videoInfoList;

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


#endif //VIDEODRIVERFINDER_H
