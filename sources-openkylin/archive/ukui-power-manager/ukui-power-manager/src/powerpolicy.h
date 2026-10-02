#ifndef POWERPOLICY_H
#define POWERPOLICY_H

#include "common.h"

class PowerPolicy : public QObject
{
    Q_OBJECT
public:
    PowerPolicy(QObject *parent = Q_NULLPTR);
    ~PowerPolicy();
};

#endif // POWERPOLICY_H
