#ifndef BATTERYINFO_H
#define BATTERYINFO_H


#include <QString>

class BatteryInfo {
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
};

#endif //BATTERYINFO_H
