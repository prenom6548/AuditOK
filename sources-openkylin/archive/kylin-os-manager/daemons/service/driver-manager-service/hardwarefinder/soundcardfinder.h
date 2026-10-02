#ifndef SOUNDCARDFINDER_H
#define SOUNDCARDFINDER_H

#include "devicefinder.h"
#include <QThread>
#include <QObject>

class SoundCardFinder : public deviceFinder
{
    Q_OBJECT
public:
    SoundCardFinder();
    ~SoundCardFinder();

    QList<SoundCardInfo> getSoundCardList();

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
    QList<SoundCardInfo> m_soundCardInfoList;
    void dowork();
};

#endif // SOUNDCARDFINDER_H
