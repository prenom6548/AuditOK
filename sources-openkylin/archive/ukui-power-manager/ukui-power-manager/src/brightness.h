#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include "common.h"

class Brightness : public QObject
{
    Q_OBJECT
public:
    explicit Brightness(QObject *parent = Q_NULLPTR);
    ~Brightness();

};

#endif // BRIGHTNESS_H
