#ifndef CPUINFO_H
#define CPUINFO_H

#include <QString>

class CpuInfo {
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

};


#endif //CPUINFO_H
