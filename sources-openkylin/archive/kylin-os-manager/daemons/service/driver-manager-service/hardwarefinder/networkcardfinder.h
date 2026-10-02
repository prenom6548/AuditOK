#ifndef NETWORKCARDFINDER_H
#define NETWORKCARDFINDER_H

#include "devicefinder.h"

#include <QThread>

class NetworkCardFinder: public deviceFinder
{
    Q_OBJECT
public:
    NetworkCardFinder();
    ~NetworkCardFinder();

    QList<NetworkCardInfo> getNetworkDeviceList();

    QThread *m_thread = nullptr;

    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }
    int checkWireless(QString name);
Q_SIGNALS:
    void finished();
    void succeed();
    void failed();

private:
    QList<NetworkCardInfo> m_NetworkCardInfoList;
    void dowork();

};

#endif // NETWORKCARDFINDER_H
