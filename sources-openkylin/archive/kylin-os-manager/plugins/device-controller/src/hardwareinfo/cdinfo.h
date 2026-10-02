#ifndef CDINFO_H
#define CDINFO_H

#include <QString>
#include "deviceinfo.h"

class CDInfo : public DeviceInfo{
public:
    CDInfo();

    QString vendor{QString()}; // 供应商
    QString model{QString()}; // 型号
    QString businfo{QString()};
    QString name{QString()};
    QString version{QString()};
    QString driver{QString()};
    QString devicenum{QString()};
    QString speed{QString()};
    QString serialnum{QString()};

    QString getValue(QString value);

    void setValue(QString key, QString value);
};


#endif //CDINFO_H
