#ifndef CPUINFOFINDER_H
#define CPUINFOFINDER_H

#include <QObject>
#include <QThread>
#include <QList>
#include "../hardwareinfo/cpuinfo.h"

class CpuInfoFinder : public QObject {
Q_OBJECT
public:
    CpuInfoFinder(QObject *parent = nullptr);

    ~CpuInfoFinder();

    CpuInfo getCpuInfo();

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
    CpuInfo m_cpuInfo;

    void dowork();
};


#endif //CPUINFOFINDER_H
