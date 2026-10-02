#ifndef PCINFO_H
#define PCINFO_H

#include <QString>

class PcInfo {
public:
    PcInfo();

    QString hostname{QString()};
    QString sysversion{QString()};
    QString yearversion{QString()};
    QString kernelversion{QString()};
    QString arch{QString()};
    QString sysbits{QString()};
    QString manufacturer{QString()};
    QString version{QString()};
    QString model{QString()};
    QString serial{QString()};
};

#endif //PCINFO_H
