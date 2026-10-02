#ifndef DISKINFO_H
#define DISKINFO_H

#include <QString>
#include "deviceinfo.h"

class DiskInfo : public DeviceInfo{
public:
    DiskInfo();

    QString model{QString{}};
    QString vendor{QString{}};
    QString name{QString{}};
    QString size{QString{}};
    QString type{QString{}};    //固态？
    QString serial{QString{}};
    QString frimwareversion{QString{}};
    int is_main;
    QString interface{QString{}};
    QString ufs{QString()};
    QString ufsrevision{QString{}};

    QString getValue(QString value);
};


#endif //DISKINFO_H
