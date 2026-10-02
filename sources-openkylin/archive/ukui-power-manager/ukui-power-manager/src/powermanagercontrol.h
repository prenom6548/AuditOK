#ifndef POWERMANAGERCONTROL_H
#define POWERMANAGERCONTROL_H

#include "common.h"

class PowerManagerControl : public QObject
{
    Q_OBJECT
public:
    PowerManagerControl(QObject *parent = Q_NULLPTR);
    ~PowerManagerControl();
};

#endif // POWERMANAGERCONTROL_H
