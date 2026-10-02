#ifndef MOUSEINFO_H
#define MOUSEINFO_H

#include <QString>
#include "deviceinfo.h"

class MouseInfo : public DeviceInfo{
public:
    MouseInfo();

    QString name{QString{}};
    QString devtype{QString{}};
    QString vendor{QString{}};
    QString interface{QString{}};
    QString model{QString{}};
    QString driver{QString{}};

    QString getValue(QString value);

};


#endif //MOUSEINFO_H
