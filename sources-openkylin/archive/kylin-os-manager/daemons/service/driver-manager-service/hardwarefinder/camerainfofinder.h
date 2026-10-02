#ifndef CAMERAINFOFINDER_H
#define CAMERAINFOFINDER_H

#include "devicefinder.h"

#include <QThread>

class CameraInfoFinder : public deviceFinder {
Q_OBJECT
public:
    CameraInfoFinder();
    ~CameraInfoFinder();

    QList<CameraInfo> getCameraList();

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
    QList<CameraInfo> m_camearInfoList;
    void dowork();
};


#endif //CAMERAINFOFINDER_H
