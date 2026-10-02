#ifndef PACKAGE_HELPER_H
#define PACKAGE_HELPER_H

#include <QTimer>
#include "device_information.h"
#include <QThread>

class PackageHelper : public QObject
{
    Q_OBJECT
public:
    PackageHelper(DeviceInformation &info, QObject *parent = nullptr);
    ~PackageHelper();
Q_SIGNALS:
    void finished();
    void succeed(bool needInstall);
    void failed();
public:
    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }
private:
    void dowork();
    QThread *m_thread{nullptr};
    DeviceInformation &m_info;
};

#endif//PACKAGE_HELPER_H
