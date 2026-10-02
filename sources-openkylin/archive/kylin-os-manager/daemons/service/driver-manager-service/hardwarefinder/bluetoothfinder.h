#ifndef BLUETOOTHFINDER_H
#define BLUETOOTHFINDER_H

#include "devicefinder.h"

#include <QThread>

class bluetoothFinder : public deviceFinder
{
    Q_OBJECT
public:
    bluetoothFinder();
    ~bluetoothFinder();

    QList<BluetoothInfo> getBluetoothDeviceList();

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
    QList<BluetoothInfo> m_bluetoothInfoList;
    void dowork();

};

#endif // BLUETOOTHFINDER_H
