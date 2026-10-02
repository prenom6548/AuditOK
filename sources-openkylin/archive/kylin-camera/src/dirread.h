#ifndef DIRREAD_H
#define DIRREAD_H


#include <QThread>

class DirRead : public QThread
{
    Q_OBJECT
public:
    explicit DirRead();
public Q_SLOTS:
    void run();

Q_SIGNALS:
    void finish();
};

#endif // DIRREAD_H
