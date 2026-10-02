#ifndef MEMORYINFOFINDER_H
#define MEMORYINFOFINDER_H

#include <QObject>
#include <QThread>
#include <QList>
#include "../hardwareinfo/memoryinfo.h"

class MemoryInfoFinder : public QObject{
Q_OBJECT
public:
    MemoryInfoFinder(QObject *parent = nullptr);

    ~MemoryInfoFinder();

    QList<MemoryInfo> getMemoryInfo();

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
    QList<MemoryInfo> m_memoryInfo;

    void dowork();
};


#endif //MEMORYINFOFINDER_H
