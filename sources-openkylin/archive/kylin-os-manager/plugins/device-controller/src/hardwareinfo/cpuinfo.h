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
    QString L4cache{QString{}};

    QString getValue(QString value);

    void setValue(QString key, QString value);

    QString getFrequency();

    QString getUsageRate();

    QString getTemprature();

private:
    //cpu
    unsigned long long m_prevCpuTotalTime = 0;
    unsigned long long m_prevCpuWorkTime = 0;
};


#endif //CPUINFO_H
