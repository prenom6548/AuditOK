#ifndef BASEBOARDINFO_H
#define BASEBOARDINFO_H

#include <QString>
#include "deviceinfo.h"

class BaseboardInfo : public DeviceInfo{
public:
    BaseboardInfo();

    QString model{QString{}};
    QString vendor{QString{}};
    QString version{QString{}};
    QString chassishandle{QString{}};  // 主板所属机箱
    QString biosvendor{QString{}};
    QString biosversion{QString{}};
    QString chipset{QString{}};
    QString serial_num{QString{}};
    QString publish_date{QString{}};

    QString getValue(QString value);

    void setValue(QString key, QString value);
};


#endif //BASEBOARDINFO_H
