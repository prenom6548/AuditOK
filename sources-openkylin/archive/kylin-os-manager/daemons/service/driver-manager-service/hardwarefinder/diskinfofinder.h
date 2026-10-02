#ifndef DISKINFOFINDER_H
#define DISKINFOFINDER_H

#include <QObject>
#include <QThread>
#include <QList>
#include "../hardwareinfo/diskinfo.h"

class DiskInfoFinder : public QObject {
    Q_OBJECT
public:
    DiskInfoFinder();

    ~DiskInfoFinder();

    QList<DiskInfo> getDiskInfo();

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
    QList<DiskInfo> m_diskInfo;

    void dowork();

    QString getUFSInfo(QString name);

    QString getDiskType(QString name);

    bool judge990();

    bool judge9006C();

    bool judgePANGUX();

    bool judgePANGUV();

};


#endif //DISKINFOFINDER_H
