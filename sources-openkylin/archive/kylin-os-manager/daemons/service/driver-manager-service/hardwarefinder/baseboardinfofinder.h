#ifndef BASEBOARDINFOFINDER_H
#define BASEBOARDINFOFINDER_H

#include <QObject>
#include <QThread>
#include <QList>
#include "../hardwareinfo/baseboardinfo.h"

class BaseboardInfoFinder : public QObject {
Q_OBJECT
public:
    BaseboardInfoFinder(QObject *parent = nullptr);

    ~BaseboardInfoFinder();

    BaseboardInfo getBaseboardInfo();

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
    BaseboardInfo m_baseboardInfo;

    void dowork();
};


#endif //BASEBOARDINFOFINDER_H
