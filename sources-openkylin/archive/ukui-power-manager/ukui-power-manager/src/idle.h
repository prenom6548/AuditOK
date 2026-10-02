#ifndef IDLE_H
#define IDLE_H

#include "common.h"

class Idle : public QObject
{
    Q_OBJECT
public:
    Idle(QObject *parent = Q_NULLPTR);
    ~Idle();
};

#endif // IDLE_H
