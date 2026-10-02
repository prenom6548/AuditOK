#ifndef MONITORINFOFINDER_H
#define MONITORINFOFINDER_H

#include "devicefinder.h"
#include "../hardwareinfo/monitorinfo.h"

#include <QThread>

class MonitorInfoFinder : public deviceFinder {
Q_OBJECT
public:
    MonitorInfoFinder();

    ~MonitorInfoFinder();

    QList<MonitorInfo> getMonitorList();

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
    QList<MonitorInfo> m_monitorInfoList;

    void dowork();
};


#endif //MONITORINFOFINDER_H
