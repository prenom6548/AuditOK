#ifndef UPLOAD_HELPER_H
#define UPLOAD_HELPER_H

#include <QTimer>
#include "device_information.h"
#include <QThread>

class UploadHelper : public QObject
{
    Q_OBJECT
public:
    UploadHelper(DeviceInformation info, int ppdRank, QObject *parent = nullptr);
    ~UploadHelper();
Q_SIGNALS:
    void finished();
public:
    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }
private:
    void dowork();
    QThread *m_thread{nullptr};
    DeviceInformation m_info;
    int m_ppdRank;
};

#endif//UPLOAD_HELPER_H
