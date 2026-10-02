#ifndef BATTERYINFO_H
#define BATTERYINFO_H


#include <QString>
#include "deviceinfo.h"

class BatteryInfo : public DeviceInfo{
public:
    BatteryInfo();

    QString vendor{QString()}; // 供应商
    QString model{QString()}; // 型号
    QString serailnum{QString()};
    QString state{QString()};
    QString energy{QString()};
    QString used_times{QString()};   //使用次数
    QString energy_full{QString{}};
    QString time_to_empty{QString()};
    QString percentage{QString{}};

    QString getValue(QString value);

    void setValue(QString key, QString value);

    QString getEnergy();

    QString getPercentage();

    QString getTimeToEmpty();

    QString getState();

};

#endif //BATTERYINFO_H
