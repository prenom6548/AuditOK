#ifndef PCINFOGETTER_H
#define PCINFOGETTER_H

#include <QObject>
#include "../hardwareinfo/pcinfo.h"
#include <QThread>

class PcIfoGetter : public QObject {
Q_OBJECT
public:
    PcIfoGetter();

    ~PcIfoGetter();

    PcInfo getPcInfo();

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
    PcInfo m_pcInfo;

    void dowork();

    QString getHostName();

    QString getSystemVersion();

    QString getSystemYearVersion();

    QString getKernelVersion();

    QString getKernelArch();

    QString getSystemBits();

    QString getManufacturer();

    QString getVersion();

    QString getModel();

    QString getSerial();
};


#endif //PCINFOGETTER_H
