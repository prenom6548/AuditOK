#ifndef FANINFO_H
#define FANINFO_H

#include <QString>
#include "deviceinfo.h"

class FanInfo : public DeviceInfo{
public:
    FanInfo();

    QString name{QString()};
    QString speed{QString()};

    QString getValue(QString value);
};


#endif //FANINFO_H
