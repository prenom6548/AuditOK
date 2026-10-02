#ifndef GETINFOTHREAD_H
#define GETINFOTHREAD_H

#include <QThread>

class GetInfoThread : public QThread{
Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
};


#endif //GETINFOTHREAD_H
