#ifndef LOWBATTERY_H
#define LOWBATTERY_H

#include "common.h"

class LowBattery : public QObject
{
    Q_OBJECT
public:
    LowBattery(QObject *parent = Q_NULLPTR);
    ~LowBattery();
};

#endif // LOWBATTERY_H
