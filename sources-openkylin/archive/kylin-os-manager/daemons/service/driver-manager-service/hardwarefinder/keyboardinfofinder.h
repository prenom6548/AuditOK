#ifndef KEYBOARDINFOFINDER_H
#define KEYBOARDINFOFINDER_H

#include "devicefinder.h"
#include "../hardwareinfo/keyboardinfo.h"

#include <QThread>

class KeyboardInfoFinder : public deviceFinder {
Q_OBJECT
public:
    KeyboardInfoFinder();

    ~KeyboardInfoFinder();

    QList<KeyboardInfo> getKeyBoardList();

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
    QList<KeyboardInfo> m_keyboardInfoList;

    void dowork();
};


#endif //KEYBOARDINFOFINDER_H
