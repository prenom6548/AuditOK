#ifndef CDINFOFINDER_H
#define CDINFOFINDER_H
#include "devicefinder.h"

#include <QThread>

class CDInfoFinder : public deviceFinder {
Q_OBJECT
public:
    CDInfoFinder();
    ~CDInfoFinder();

    QList<CDInfo> getCDList();

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
    QList<CDInfo> m_CDInfoList;
    void dowork();
};


#endif //CDINFOFINDER_H
