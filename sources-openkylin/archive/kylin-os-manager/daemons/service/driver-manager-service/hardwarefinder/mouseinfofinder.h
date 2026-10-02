#ifndef MOUSEINFOFINDER_H
#define MOUSEINFOFINDER_H

#include "devicefinder.h"
#include "../hardwareinfo/mouseinfo.h"

#include <QThread>

class MouseInfoFinder : public deviceFinder{
Q_OBJECT
public:
    MouseInfoFinder();
    ~MouseInfoFinder();

    QList<MouseInfo> getMouseList();

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
    QList<MouseInfo> m_mouseInfoList;

    void dowork();

};


#endif //MOUSEINFOFINDER_H
