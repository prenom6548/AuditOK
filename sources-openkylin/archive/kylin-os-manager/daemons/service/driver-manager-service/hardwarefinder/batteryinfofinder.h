#ifndef BATTERYINFOFINDER_H
#define BATTERYINFOFINDER_H

#include "devicefinder.h"

#include <QThread>

class BatteryInfoFinder : public deviceFinder {
Q_OBJECT
public:
    BatteryInfoFinder();
    ~BatteryInfoFinder();

    QList<BatteryInfo> getBatteryList();

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
    QList<BatteryInfo> m_batteryInfoList;
    void dowork();
};

#endif //BATTERYINFOFINDER_H
