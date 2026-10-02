#ifndef CPUINFO_H
#define CPUINFO_H

#include <QString>
#include "deviceinfo.h"

class CpuInfo : public DeviceInfo {
public:
    CpuInfo();

    QString vendor{QString()}; // 供应商
    QString model{QString()}; // 型号
    QString arch{QString()};
    QString cores{QString{}};
    QString onlinecores{QString()};
    QString threadprecore{QString{}};
    QString maxMHz{QString()};
    QString L1dcache{QString{}};
    QString L1icache{QString{}};
    QString L2cache{QString{}};
    QString L3cache{QString{}};

    QString getValue(QString value);

    QString getFrequency();

    QString getUsageRate();

    QString getTemprature();
};


#endif //CPUINFO_H
