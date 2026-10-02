#ifndef FANINFOFINDER_H
#define FANINFOFINDER_H

#include "devicefinder.h"

#include <QThread>

class FanInfoFinder : public deviceFinder {
Q_OBJECT
public:
    FanInfoFinder();
    ~FanInfoFinder();

    QList<FanInfo> getFanList();

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
    QList<FanInfo> m_fanInfoList;
    void dowork();
};


#endif //FANINFOFINDER_H
