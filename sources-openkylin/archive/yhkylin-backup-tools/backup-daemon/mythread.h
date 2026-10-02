#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>

class MyThread : public QThread
{
    Q_OBJECT
public:
    MyThread(QObject *parent = nullptr);
    virtual ~MyThread();

signals:
    void cancelWork();
};

#endif // MYTHREAD_H
